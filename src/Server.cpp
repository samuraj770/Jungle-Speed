#include <iostream>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "Server.h"

// #include "Player.h"
// #include "GameRoom.h"

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
        setupNetwork();
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

    // handleClientData();
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

void Server::createRoom()
{
}

void Server::joinRoom()
{
}

void Server::removeRoom()
{
}

void Server::setupNetwork()
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

    // auto newPlayer = make_shared<Player>(client_fd);
    // this->clients[client_fd] = newPlayer;

    cout << "Nowy klient połączony fd: " << client_fd << endl;

    write(client_fd, "Witaj! Jestem serwer!", 22);
}

void Server::handleClientData(int client_fd)
{
    char buf[MAX_BUF];

    // auto player = this->clients.find(client_fd);

    int rmsg = read(client_fd, buf, MAX_BUF);

    if (rmsg > 0)
    {
        cout << "Klient " << client_fd << " napisal: ";
        write(STDOUT_FILENO, buf, rmsg);
    }
    else
    {
        handleClientDisconnect(client_fd);
    }
}

void Server::handleClientDisconnect(int client_fd)
{
    cout << "Klient rozłączyl sie fd: " << client_fd << endl;

    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);

    this->clients.erase(client_fd);
}
