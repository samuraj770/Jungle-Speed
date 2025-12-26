#include <algorithm>

#include "Server.h"
#include "Player.h"
#include "GameRoom.h"
#include "GameState.h"

using namespace std;

void GameRoom::startGame(shared_ptr<Player> player)
{
    if (!isHost(player))
    {
        cout << "Gracz nie będący hostem nie może zacząć gry" << endl;
        return;
    }

    this->gameState = make_unique<GameState>();
    this->gameActive = true;
    gameState->initialize(players);
    // broadcastMessage(string("ACCEPT_GAME_START") + " " + to_string(gameState->getPlayerDeckSize(player)));
    broadcastMessage(string("ACCEPT_GAME_START"));
    broadcastMessage(gameState->getPlayersDeckSizes());
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
    this->gameActive = false;
}

GameRoom::~GameRoom() {}

void GameRoom::addPlayer(shared_ptr<Player> newPlayer)
{
    players.push_back(newPlayer);
    newPlayer->setRoom(shared_from_this());

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

    if (players.empty())
    {
        gameActive = false;
        return;
    }

    if (player == this->host)
    {
        assignNewHost();
    }

    if (gameActive && gameState)
    {
        gameState->removePlayer(player);
        broadcastMessage(gameState->getPlayersDeckSizes());
    }
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
    if (command == "GAME_START")
    {
        startGame(player);
    }
    else if (command == "CARD_REVEAL")
    {
        string msg = gameState->playerFlipCard(player);
        broadcastMessage(string("CARD_ID") + " " + msg + " " + player->getNick());
    }
    else if (command == "TOTEM_GRAB")
    {
        string msg = gameState->playerGrabTotem(player);
        broadcastMessage(gameState->getPlayersDeckSizes());
    }
    else
    {
        cout << "BŁĄD: Niepoprawny komunikat" << endl;
    }
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
