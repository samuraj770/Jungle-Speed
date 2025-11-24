#include "Server.h"
#include "Player.h"
#include "GameRoom.h"
// #include "GameState.h"

using namespace std;

GameRoom::GameRoom(string name, shared_ptr<Player> host)
{
}

GameRoom::~GameRoom()
{
}

void GameRoom::addPlayer(shared_ptr<Player> player)
{
}

void GameRoom::handlePlayerDisconnect(shared_ptr<Player> player)
{
}

void GameRoom::broadcastMessage(const string &message, shared_ptr<Player> excludePlayer)
{
}

void GameRoom::handleGameAction(shared_ptr<Player> player, const string &command)
{
}

bool GameRoom::isHost(shared_ptr<Player> player) const
{
    return this->host == player;
}

string GameRoom::getPlayerNicks() const
{
    string nicks;
    for (const auto &player : this->players)
    {
        nicks += "#";
        nicks += player->getNick();
    }
    return nicks;
}
