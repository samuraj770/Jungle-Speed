#pragma once

#include <memory>
#include <iostream>
#include <string>

class Server;
class GameRoom;

class Player : public std::enable_shared_from_this<Player>
{
private:
    int fd;                              // deskryptor pliku gracza (klienta)
    std::string nick;                    // nazwa gracza
    std::weak_ptr<GameRoom> currentRoom; // wskazanie na pokoj w ktorym gracz sie znajduje
    bool isSpectatorMode = false;        // czy jest w trybie widza

public:
    std::string buf; // bufor do przechowywania wiadomosci

    Player(int fd);

    ~Player();

    void processMessage(const std::string &message, Server *server);

    void sendMessage(const std::string &message);

    void setRoom(std::shared_ptr<GameRoom> room);

    void quitRoom();

    std::shared_ptr<GameRoom> getRoom();

    int getFd() const { return fd; }

    void setNick(const std::string &nick) { this->nick = nick; }

    std::string getNick() const { return nick; }

    void setSpectator(bool state) { isSpectatorMode = state; }

    bool isSpectator() const { return isSpectatorMode; }
};
