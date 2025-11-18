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
    int fd;
    string nick;

public:
    Player(int fd);
    ~Player();
};
