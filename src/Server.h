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

    void createRoom(const string &roomName, shared_ptr<Player> host);

    void joinRoom(const string &roomName, shared_ptr<Player> player);

    void removeRoom(const string &roomName);

private:
    int port;
    int server_fd;
    int epoll_fd;
    epoll_event event;
    epoll_event events[MAX_EVENTS];

    // map<klucz, wartosc>
    // polaczeni klienci
    map<int, shared_ptr<Player>> clients;

    // aktywne pokoje
    map<string, shared_ptr<GameRoom>> rooms;

    void setupNetwork();

    void handleNewConnection();

    void handleClientData(int client_fd);

    void handleClientDisconnect(int client_fd);
};
