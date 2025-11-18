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

#define MAX_EVENTS 64
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
    close(server_fd);
    close(epoll_fd);
}

void Server::run()
{
    handleNewConnection();
    // handleClientData();
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

    cout << "Serwer skonfigurowany, nasłuchuje na porcie " << port << endl;
}

void Server::handleNewConnection()
{
    while (true)
    {
        sockaddr_in clientSocket;
        socklen_t lclientSocket = sizeof(clientSocket);

        int client_fd = accept(server_fd, (sockaddr *)&clientSocket, &lclientSocket);

        setNonBlocking(client_fd);

        write(client_fd, "HELLO!", 7);

        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }
}

void Server::handleClientData(int client_fd)
{
    char buf[MAX_BUF];

    int rmsg = read(client_fd, buf, MAX_BUF);
}

void Server::handleClientDisconnect(int client_fd)
{

    //~Player();
}
