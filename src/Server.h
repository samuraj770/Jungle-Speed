#pragma once

#include <map>
#include <string>
#include <netinet/in.h>
#include <memory>

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

    map<int, shared_ptr<Player>> clients; // polaczeni klienci

    map<string, shared_ptr<GameRoom>> rooms; // aktywne pokoje

    void setupNetwork();

    void handleNewConnection();

    void handleClientData();

    void handleClientDisconnect();
};
