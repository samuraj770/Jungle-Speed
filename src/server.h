#pragma once

#include <map>
#include <string>
#include <netinet/in.h>

using namespace std;

class Player;
class GameRoom;

class Server
{
public:
    Server(int port);
    ~Server();

    void run();

    void createRoom();
};
