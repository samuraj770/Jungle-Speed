#include <algorithm>

#include "Server.h"
#include "Player.h"
#include "GameRoom.h"

// #include "GameState.h"

using namespace std;

void GameRoom::startGame()
{
}

void GameRoom::endGame()
{
}

void GameRoom::assignNewHost()
{
    if (players.empty())
        return;

    this->host = players[0];
}

GameRoom::GameRoom(string name, shared_ptr<Player> host)
{
    this->name = name;
    this->host = host;
}

GameRoom::~GameRoom() {}

void GameRoom::addPlayer(shared_ptr<Player> newPlayer)
{
    players.push_back(newPlayer);

    string msg = string("PLAYER_NEW") + " " + newPlayer->getNick();
    broadcastMessage(msg, newPlayer);
}

void GameRoom::handlePlayerDisconnect(shared_ptr<Player> player)
{
    auto it = remove(players.begin(), players.end(), player);
    if (it != players.end())
    {
        players.erase(it, players.end());
    }

    string msg = string("PLAYER_DISC") + " " + player->getNick();
    broadcastMessage(msg);

    if (player == this->host)
    {
        assignNewHost();
    }

    /*if(gameActive && gameState)
    {
        gameState->removePlayer(player);
    }*/
}

void GameRoom::broadcastMessage(const string &message, shared_ptr<Player> excludePlayer)
{
    for (auto &player : players)
    {
        if (player != excludePlayer)
        {
            player->sendMessage(message);
        }
    }
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
        nicks += " ";
        nicks += player->getNick();
    }
    return nicks;
}
