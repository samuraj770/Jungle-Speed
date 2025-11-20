#pragma once

#include <memory>
#include <iostream>
#include <string>

using namespace std;

class Server;
class GameRoom;

class Player
{
private:
    int fd; // deskryptor pliku gracza (klienta)
    string nick;
    string buf;                     // bufor do przechowywania wiadomosci
    weak_ptr<GameRoom> currentRoom; // wskazanie na pokoj w ktorym gracz sie znajduje

public:
    Player(int fd);
    ~Player();

    void processMessage(const string &message, Server *server);

    void sendMessage(const string &message);

    void setRoom(shared_ptr<GameRoom> room);

    void leaveRoom();

    int getFd() const { return fd; }

    void setNick(const string &nick) { this->nick = nick; }
    string getNick() const { return nick; }
};
