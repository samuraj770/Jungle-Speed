#pragma once

#include <memory>
#include <iostream>
#include <string>

using namespace std;

class Server;
class GameRoom;

class Player : public enable_shared_from_this<Player>
{
private:
    int fd;                         // deskryptor pliku gracza (klienta)
    string nick;                    // nazwa gracza
    string buf;                     // bufor do przechowywania wiadomosci
    weak_ptr<GameRoom> currentRoom; // wskazanie na pokoj w ktorym gracz sie znajduje

public:
    Player(int fd);
    ~Player();

    void processMessage(const string &message, Server *server);

    void sendMessage(const string &message);

    void setRoom(shared_ptr<GameRoom> room);

    void quitRoom();

    shared_ptr<GameRoom> getRoom();

    int getFd() const { return fd; }

    void setNick(const string &nick) { this->nick = nick; }
    string getNick() const { return nick; }
};
