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
private:
    int port;      // port na ktorym nasluchuje serwer
    int server_fd; // deskryptor serwera
    int epoll_fd;  // deskryptor mechanizmu epoll
    epoll_event event;
    epoll_event events[MAX_EVENTS];

    map<int, shared_ptr<Player>> clients; // polaczeni klienci

    map<string, shared_ptr<GameRoom>> rooms; // aktywne pokoje

    void setUpNetwork();

    void handleNewConnection();

    void handleClientData(int client_fd);

    void handleClientDisconnect(int client_fd);

public:
    Server(int port); // Konstruktor
    ~Server();        // Destruktor

    void run();

    void createRoom(shared_ptr<Player> host);

    void joinRoom(const string &roomName, shared_ptr<Player> player);

    void removeRoom(const string &roomName);
};
