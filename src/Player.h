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
    weak_ptr<GameRoom> currentRoom; // wskazanie na pokoj w ktorym gracz sie znajduje
    bool isSpectatorMode = false;   // czy jest w trybie widza

public:
    string buf; // bufor do przechowywania wiadomosci

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

    void setSpectator(bool state) { isSpectatorMode = state; }

    bool isSpectator() const { return isSpectatorMode; }
};
