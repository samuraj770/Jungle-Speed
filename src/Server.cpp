#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <csignal>

#include "Server.h"
#include "Player.h"
#include "Utils.h"
#include "GameRoom.h"

#define MAX_BUF 4096

using namespace std;

volatile sig_atomic_t serverRunning = 1;

void setNonBlocking(int sockfd)
{
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
}

void handleSignal(int)
{
    serverRunning = 0;
}

Server::Server(int port) : port(port)
{
    try
    {
        setUpNetwork();
    }
    catch (const exception &e)
    {
        cerr << "BŁĄD: inicjalizacja serwera: " << e.what() << endl;
        throw;
    }
}

Server::~Server()
{
    clients.clear();
    rooms.clear();

    if (epoll_fd != -1)
    {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, server_fd, nullptr);
        close(epoll_fd);
    }

    if (server_fd != -1)
    {
        close(server_fd);
    }
    cout << "\nSerwer zamknięty poprawnie" << endl;
}

void Server::run()
{
    signal(SIGINT, handleSignal);

    while (serverRunning)
    {
        int ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (ready == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                cerr << "BŁĄD: epoll_wait" << endl;
                break;
            }
        }

        for (int i = 0; i < ready; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                handleNewConnection();
            }
            else
            {
                try
                {
                    handleClientData(events[i].data.fd);
                }
                catch (const exception &e)
                {
                    cerr << "BŁĄD: Przetwarzanie danych" << e.what() << endl;
                    handleClientDisconnect(events[i].data.fd);
                }
            }
        }
    }
}

void Server::createRoom(shared_ptr<Player> host)
{
    string roomName = generateRoomCode();
    while (this->rooms.find(roomName) != this->rooms.end())
    {
        // cout << "Pokoj o podanej nazwie istnieje" << endl;
        roomName = generateRoomCode();
    }

    cout << "Utworzono pokoj: " << roomName << endl;
    auto newRoom = make_shared<GameRoom>(roomName, host);
    this->rooms[roomName] = newRoom;

    newRoom->addPlayer(host);
    host->sendMessage(string("ACCEPT_CR_ROOM") + " " + roomName);
}

void Server::joinRoom(const string &roomName, shared_ptr<Player> player)
{
    auto room_it = this->rooms.find(roomName);
    if (room_it == this->rooms.end())
    {
        player->sendMessage("JOIN_ERR INVALID_CODE");
        handleClientDisconnect(player->getFd());
        return;
    }

    shared_ptr<GameRoom> room = room_it->second;

    if (room->getPlayerCount() >= 4)
    {
        player->sendMessage("JOIN_ERR ROOM_FULL");
        handleClientDisconnect(player->getFd());
        return;
    }

    if (!room->addPlayer(player))
    {
        player->sendMessage("JOIN_ERR NICK_TAKEN");
        handleClientDisconnect(player->getFd());
        return;
    }
}

void Server::removeRoom(const string &roomName)
{
    auto room_it = this->rooms.find(roomName);
    this->rooms.erase(room_it);
}

void Server::setUpNetwork()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        throw runtime_error("BŁĄD: socket");
    }

    int t = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t));

    sockaddr_in serverSocket;
    serverSocket.sin_family = AF_INET;
    serverSocket.sin_addr.s_addr = INADDR_ANY;
    serverSocket.sin_port = htons(port);

    if (bind(server_fd, (sockaddr *)&serverSocket, sizeof(serverSocket)) == -1)
    {
        throw runtime_error("BŁĄD: bind (port zajęty)");
    }

    listen(server_fd, SOMAXCONN);
    setNonBlocking(server_fd);

    epoll_fd = epoll_create1(0);
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

    cout << "Serwer skonfigurowany, nasłuchuje na porcie " << port << endl;
}

void Server::handleNewConnection()
{
    sockaddr_in clientSocket;
    socklen_t lclientSocket = sizeof(clientSocket);

    int client_fd = accept(server_fd, (sockaddr *)&clientSocket, &lclientSocket);

    setNonBlocking(client_fd);

    event.events = EPOLLIN;
    event.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

    auto newPlayer = make_shared<Player>(client_fd);
    this->clients[client_fd] = newPlayer;

    cout << "Nowy klient połączony fd: " << client_fd << endl;
}

void Server::handleClientData(int client_fd)
{
    char buf[MAX_BUF];

    auto player_it = this->clients.find(client_fd);
    if (player_it == this->clients.end())
    {
        return;
    }
    auto player = player_it->second;

    const size_t MAX_PLAYER_BUFFER = 8096;

    int rmsg = read(client_fd, buf, MAX_BUF);

    if (rmsg > 0)
    {
        if (player->buf.size() + rmsg > MAX_PLAYER_BUFFER)
        {
            cerr << "BŁĄD: Przekrocznie limitu bufora przez gracza" << endl;
            handleClientDisconnect(client_fd);
            return;
        }

        player->buf.append(buf, rmsg);

        size_t delimiterPos;
        while ((delimiterPos = player->buf.find('%')) != string::npos)
        {
            string message = player->buf.substr(0, delimiterPos);

            cout << "Klient " << client_fd << " napisal: " << message << endl;
            player->processMessage(message, this);

            player->buf.erase(0, delimiterPos + 1);
        }
    }
    else if (rmsg == 0)
    {
        handleClientDisconnect(client_fd);
    }
    else
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            handleClientDisconnect(client_fd);
        }
    }
}

void Server::handleClientDisconnect(int client_fd)
{
    auto player_it = this->clients.find(client_fd);

    if (player_it == this->clients.end())
    {
        return;
    }

    auto player = player_it->second;

    cout << "Klient rozłączyl sie fd: " << client_fd << endl;

    auto room = player->getRoom();

    player->quitRoom();

    if (room)
    {
        if (room->getPlayerCount() == 0)
        {
            this->removeRoom(room->getName());
        }
    }

    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);

    this->clients.erase(client_fd);
}
