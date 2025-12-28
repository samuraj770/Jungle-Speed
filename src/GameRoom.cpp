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

    if (players.size() < 2)
    {
        return;
    }

    for (auto &player : players)
    {
        player->setSpectator(false);
    }

    this->gameState = make_unique<GameState>();
    this->gameActive = true;
    gameState->initialize(players);
    broadcastMessage(string("ACCEPT_GAME_START") + " " + to_string(gameState->getPlayerDeckSize(player)));
}

void GameRoom::endGame()
{
    gameActive = false;
    gameState.reset();
}

void GameRoom::assignNewHost()
{
    if (players.empty())
        return;

    this->host = players[0];
}

void GameRoom::checkResult()
{
    auto winners = gameState->checkWinners();

    if (!winners.empty())
    {
        for (auto winner : winners)
        {
            broadcastMessage(string("GAME_FINISHED") + " " + winner->getNick());
            gameState->removePlayer(winner);
        }
    }

    if (gameState->getActivePlayersCount() < 2 || gameState->isStalemate())
    {
        broadcastMessage(string("GAME_OVER"));
        endGame();
    }
}

GameRoom::GameRoom(string name, shared_ptr<Player> host)
{
    this->name = name;
    this->host = host;
    this->gameActive = false;
}

GameRoom::~GameRoom()
{
    cout << "Zniszczono pokój: " << name << endl;
}

void GameRoom::addPlayer(shared_ptr<Player> newPlayer)
{
    players.push_back(newPlayer);
    newPlayer->setRoom(shared_from_this());

    if (this->gameActive) // @TODO: sprawdzić co się stanie gdy gracze wyjdą ale są osoby w trybie widza
    {
        newPlayer->setSpectator(true);

        if (gameState)
        {
            newPlayer->sendMessage(gameState->getPlayersDeckSizes());
            newPlayer->sendMessage(gameState->getPlayersFaceUpCards());
        }
    }
    else
    {
        newPlayer->setSpectator(false);
        string msg = string("PLAYER_NEW") + " " + newPlayer->getNick();
        broadcastMessage(msg, newPlayer);
    }

    string msg = string("ACCEPT_JOIN") + " " +
                 to_string(this->isGameActive()) + " " +               // kod czy aktywny gra
                 to_string(gameState->getActivePlayersCount()) + " " + // liczba aktywnych graczy @TODO
                 to_string(this->getPlayerCount()) +                   // liczba graczy w pokoju
                 this->getPlayerNicksString();
    newPlayer->sendMessage(msg);
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

    if (players.size() < 2) //@TODO: przy jednym graczu zakończyć grę
    {
        endGame();
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
    if (player->isSpectator())
    {
        return;
    }

    if (command == "GAME_START")
    {
        startGame(player);
    }
    else if (command == "CARD_REVEAL")
    {
        if (!gameActive) // @TODO: sprawdzić czy nie ma błędu pamięci
            return;
        string msg = gameState->playerFlipCard(player);
        broadcastMessage(string("CARD_ID") + " " + msg + " " + player->getNick());
        checkResult();
    }
    else if (command == "TOTEM")
    {
        string msg = gameState->playerGrabTotem(player);
        if (msg == "TOTEM_WON")
        {
            broadcastMessage(msg + " " + player->getNick());
        }
        else
        {
            broadcastMessage(msg + " " + player->getNick());
        }
        broadcastMessage(gameState->getPlayersDeckSizes());

        checkResult();
    }
    else
    {
        cerr << "BŁĄD: Niepoprawny komunikat" << endl;
    }
}

bool GameRoom::isHost(shared_ptr<Player> player) const
{
    return this->host == player;
}

string GameRoom::getPlayerNicksString() const
{
    string nicks;
    for (const auto &player : this->players)
    {
        nicks += " ";
        nicks += player->getNick();
    }
    return nicks;
}
