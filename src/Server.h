#pragma once

#include <map>
#include <string>
#include <netinet/in.h>
#include <memory>
#include <sys/epoll.h>

#define MAX_EVENTS 64

class Player;
class GameRoom;

class Server
{
private:
    int port;      // port na ktorym nasłuchuje serwer
    int server_fd; // deskryptor serwera
    int epoll_fd;  // deskryptor mechanizmu epoll
    epoll_event event;
    epoll_event events[MAX_EVENTS];

    std::map<int, std::shared_ptr<Player>> clients; // połączeni klienci

    std::map<std::string, std::shared_ptr<GameRoom>> rooms; // aktywne pokoje

    void setUpNetwork();

    void handleNewConnection();

    void handleClientData(int client_fd);

    void handleClientDisconnect(int client_fd);

public:
    Server(int port); // Konstruktor

    ~Server(); // Destruktor

    void run();

    void createRoom(std::shared_ptr<Player> host);

    void joinRoom(const std::string &roomName, std::shared_ptr<Player> player);

    void removeRoom(const std::string &roomName);
};
