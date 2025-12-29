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
        cerr << "BŁĄD: Gracz nie będący hostem nie może zacząć gry." << endl;
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

GameRoom::~GameRoom() {}

bool GameRoom::addPlayer(shared_ptr<Player> newPlayer)
{
    if (isNickTaken(newPlayer->getNick()))
    {
        return false;
    }
    players.push_back(newPlayer);
    newPlayer->setRoom(shared_from_this());

    if (this->gameActive) // @TODO: sprawdzić co się stanie gdy gracze wyjdą ale są osoby w trybie widza
    {
        newPlayer->setSpectator(true);
        string msg = string("PLAYER_NEW") + " " + newPlayer->getNick();
        broadcastMessage(msg, newPlayer);

        if (gameState)
        {
            string msg = string("ACCEPT_JOIN") + " " +
                         to_string(this->isGameActive()) + " " +               // kod czy aktywny gra
                         to_string(gameState->getActivePlayersCount()) + " " + // liczba aktywnych graczy @TODO
                         to_string(this->getPlayerCount()) +                   // liczba graczy w pokoju
                         this->getPlayerNicksString();
            newPlayer->sendMessage(msg);
            newPlayer->sendMessage(gameState->getPlayersDeckSizes());
            newPlayer->sendMessage(gameState->getPlayersFaceUpCards());
        }
    }
    else
    {
        newPlayer->setSpectator(false);
        string msg = string("PLAYER_NEW") + " " + newPlayer->getNick();
        broadcastMessage(msg, newPlayer);

        if (newPlayer != host)
        {
            msg = string("ACCEPT_JOIN") + " " +
                  to_string(this->isGameActive()) + " " + // kod czy aktywny gra
                  "0" + " " +                             // liczba aktywnych graczy @TODO
                  to_string(this->getPlayerCount()) +     // liczba graczy w pokoju
                  this->getPlayerNicksString();
            newPlayer->sendMessage(msg);
        }
    }

    return true;
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

    if (players.size() < 2)
    {
        if (gameActive)
        {
            broadcastMessage(string("GAME_OVER"));
            endGame();
            return;
        }
    }

    if (gameActive && gameState)
    {
        gameState->removePlayer(player);
        if (gameState->getActivePlayersCount() < 2)
        {
            broadcastMessage("GAME_OVER");
            endGame();
        }
        else
        {
            broadcastMessage(gameState->getPlayersDeckSizes());
        }
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
        if (!gameState)
            return;
        string msg = gameState->playerFlipCard(player);
        broadcastMessage(string("CARD_ID") + " " + player->getNick() + " " + msg);
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
        cerr << "BŁĄD: Niepoprawny komunikat." << endl;
    }
}

bool GameRoom::isHost(shared_ptr<Player> player) const
{
    return this->host == player;
}

bool GameRoom::isNickTaken(const string &nick) const
{
    if (this->players.empty())
        return false;

    for (const auto &player : players)
    {
        if (player->getNick() == nick)
        {
            return true;
        }
    }
    return false;
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
