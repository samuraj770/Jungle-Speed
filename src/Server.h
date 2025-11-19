#pragma once

#include <map>
#include <string>
#include <netinet/in.h>
#include <memory>
#include <sys/epoll.h>

#define MAX_EVENTS 64

using namespace std;

class Player;
class GameRoom;

class Server
{
public:
    Server(int port); // Konstruktor
    ~Server();        // Destruktor

    void run();

    void createRoom();

    void joinRoom();

    void removeRoom();

private:
    int port;
    int server_fd;
    int epoll_fd;
    epoll_event event;
    epoll_event events[MAX_EVENTS];

    map<int, shared_ptr<Player>> clients; // polaczeni klienci

    map<string, shared_ptr<GameRoom>> rooms; // aktywne pokoje

    void setupNetwork();

    void handleNewConnection();

    void handleClientData(int client_fd);

    void handleClientDisconnect(int client_fd);
};
