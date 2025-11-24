#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

#include "Server.h"
#include "Player.h"
#include "Utils.h"
#include "GameRoom.h"

#define MAX_BUF 256

using namespace std;

void setNonBlocking(int sockfd)
{
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
}

Server::Server(int port) : port(port)
{
    try
    {
        setUpNetwork();
    }
    catch (exception &e)
    {
        cerr << "blad inicjalizacji serwera: " << e.what() << endl;
        throw;
    }
}

Server::~Server()
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, server_fd, nullptr);
    close(server_fd);
    close(epoll_fd);
}

void Server::run()
{
    while (true)
    {
        int ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < ready; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                handleNewConnection();
            }
            else
            {
                handleClientData(events[i].data.fd);
            }
        }
    }
}

void Server::createRoom(shared_ptr<Player> host)
{
    string roomName = generateRoomCode();
    while (this->rooms.find(roomName) != this->rooms.end())
    {
        cout << "Pokoj o podanej nazwie istnieje" << endl;
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
        player->sendMessage("JOIN_ERR 0");
        return;
    }

    shared_ptr<GameRoom> room = room_it->second;
    // tryb widza i pelny pokoj zrobic

    room->addPlayer(player);

    string nicks = room->getPlayerNicks();
    string msg = string("ACCEPT_JOIN") + " " +
                 to_string(room->isGameActive()) + " " +   // kod czy aktywny gra
                 to_string(room->getPlayerCount()) + " " + // liczba aktywnych graczy
                 to_string(room->getPlayerCount()) +       // liczba graczy w pokoju
                 nicks;
    player->sendMessage(msg);
}

void Server::removeRoom(const string &roomName)
{
    auto room_it = this->rooms.find(roomName);
    this->rooms.erase(room_it);
}

void Server::setUpNetwork()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int t = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t));

    sockaddr_in serverSocket;
    serverSocket.sin_family = AF_INET;
    serverSocket.sin_addr.s_addr = INADDR_ANY;
    serverSocket.sin_port = htons(port);

    if (bind(server_fd, (sockaddr *)&serverSocket, sizeof(serverSocket)) == -1)
    {
        perror("blad bind");
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

    auto player_it = this->clients.find(client_fd); // find zwraca iterator
    auto player = player_it->second;

    int rmsg = read(client_fd, buf, MAX_BUF);

    if (rmsg > 0)
    {
        if (rmsg < MAX_BUF)
        {
            buf[rmsg - 1] = '\0';
            string message(buf);
            cout << "Klient " << client_fd << " napisal: " << buf << endl;
            player->processMessage(message, this);
        }
    }
    else
    {
        handleClientDisconnect(client_fd);
    }
}

void Server::handleClientDisconnect(int client_fd)
{
    auto player_it = this->clients.find(client_fd); // find zwraca iterator
    auto player = player_it->second;

    cout << "Klient rozłączyl sie fd: " << client_fd << endl;

    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);

    this->clients.erase(client_fd);
}
