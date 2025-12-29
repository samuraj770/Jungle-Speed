#include <random>
#include <algorithm>
#include <sstream>

#include "Server.h"
#include "Player.h"
#include "GameRoom.h"
#include "GameState.h"

#define TOLERANCE_TIME 500 // milisekundy

using namespace std;

deque<Card> GameState::generateDeck()
{
    deque<Card> deck;

    int colorCount = static_cast<int>(CardColor::COUNT);
    int shapeCount = static_cast<int>(CardShape::COUNT);

    for (int i = 0; i < colorCount; i++)
    {
        for (int j = 0; j < shapeCount; j++)
        {
            for (int k = 0; k < 1; k++) // ZMIANA!!! k powinno być=2 zmieniono na potrzeby testów @TODO
            {
                deck.push_back({static_cast<CardColor>(i), static_cast<CardShape>(j)});
            }
        }
    }

    return deck;
}

void GameState::shuffleDeck(deque<Card> &deck)
{
    random_device rd;
    mt19937 generator(rd());
    shuffle(deck.begin(), deck.end(), generator);
}

void GameState::dealCards(deque<Card> &deck, const vector<shared_ptr<Player>> &targets)
{
    if (targets.empty())
        return;

    size_t playerIndex = 0;

    while (!deck.empty())
    {
        auto currentPlayer = targets[playerIndex];
        playerDecks[currentPlayer].faceDown.push_back(deck.front());
        deck.pop_front();
        playerIndex = (playerIndex + 1) % targets.size();
    }
}

void GameState::collectFaceUpCards(const vector<shared_ptr<Player>> &targets)
{
    for (const auto &player : targets)
    {
        if (playerDecks.find(player) == playerDecks.end())
            continue;

        deque<Card> &faceUpStack = playerDecks[player].faceUp;

        if (faceUpStack.empty())
            continue;

        pot.insert(pot.end(), faceUpStack.begin(), faceUpStack.end());
        faceUpStack.clear();
    }
}

void GameState::nextTurn()
{
    currentTurnIndex = (currentTurnIndex + 1) % turnOrder.size();
}

bool GameState::checkForDuels()
{
    activeDuelists.clear();
    duelActive = false;

    map<CardShape, vector<shared_ptr<Player>>> shapeOwners;

    for (auto player : turnOrder)
    {
        PlayerDeck &deck = playerDecks[player];
        if (!deck.faceUp.empty())
        {
            Card topCard = deck.faceUp.back();
            shapeOwners[topCard.shape].push_back(player);
        }
    }

    for (auto const &[shape, owners] : shapeOwners)
    {
        if (owners.size() > 1)
        {
            activeDuelists.insert(activeDuelists.end(), owners.begin(), owners.end());
            duelActive = true;
        }
    }

    return duelActive;
}

GameState::GameState()
{
    lastDuelEndTime = chrono::steady_clock::now() - chrono::hours(1);
}

GameState::~GameState() {}

void GameState::initialize(const vector<shared_ptr<Player>> &players)
{
    this->turnOrder = players;
    this->currentTurnIndex = 0;
    this->playerDecks.clear();
    this->pot.clear();
    this->duelActive = false;
    this->activeDuelists.clear();

    deque<Card> fullDeck = generateDeck();
    shuffleDeck(fullDeck);
    dealCards(fullDeck, turnOrder);
}

string GameState::playerFlipCard(shared_ptr<Player> player)
{
    if (player != turnOrder[currentTurnIndex])
    {
        cout << "Błąd kolejki" << endl;
        return string("-1") + " " + turnOrder[currentTurnIndex]->getNick();
    }

    // na potrzeby testów końcowo pojedynek nie może blokować tury
    // if (duelActive)
    // {
    //     cout << "Trwa pojedynek" << endl;
    //     return "-1";
    // }

    PlayerDeck &deck = playerDecks[player];

    if (deck.faceDown.empty())
    {
        nextTurn(); // jeśli gracz nie ma kart na stosie zakrytych to pomijamy jego turę
        return "#"; // informujemy klienta o tym że gracz wykonał turę a nie ma kart
    }

    Card revealedCard = deck.faceDown.front();
    deck.faceDown.pop_front();
    deck.faceUp.push_back(revealedCard);

    bool newDuel = checkForDuels();
    if (newDuel)
    {
        cout << "Gracze w pojedynku" << endl;
        for (auto duelist : activeDuelists)
        {
            cout << duelist->getNick() << endl;
        }
        nextTurn();
    }
    else
    {
        nextTurn();
    }

    // nextTurn();
    return revealedCard.toString();
}

string GameState::playerGrabTotem(shared_ptr<Player> player)
{
    if (duelActive)
    {
        bool isDuelist = false;
        for (auto activeDuelist : activeDuelists)
        {
            if (activeDuelist == player)
            {
                isDuelist = true;
                break;
            }
        }

        if (isDuelist)
        {
            cout << player->getNick() << "Wygrał pojedynek" << endl;
            // przegrani pojedynku
            vector<shared_ptr<Player>> losers;
            for (auto p : activeDuelists)
            {
                if (p != player)
                {
                    losers.push_back(p);
                }
            }

            collectFaceUpCards(activeDuelists);
            dealCards(pot, losers);

            duelActive = false;
            activeDuelists.clear();

            lastDuelEndTime = chrono::steady_clock::now();

            return "TOTEM_WON";
        }
        else
        {
            collectFaceUpCards(turnOrder);
            dealCards(pot, {player});
            duelActive = false;
            activeDuelists.clear();

            lastDuelEndTime = chrono::steady_clock::now();

            return "TOTEM_INVALID";
        }
    }
    else
    {
        auto now = chrono::steady_clock::now();
        auto timeSinceDuel = chrono::duration_cast<chrono::milliseconds>(now - lastDuelEndTime).count();

        if (timeSinceDuel < TOLERANCE_TIME)
        {
            return "TOTEM_LOST";
        }

        collectFaceUpCards(turnOrder);
        dealCards(pot, {player});

        return "TOTEM_INVALID";
    }
}

vector<shared_ptr<Player>> GameState::checkWinners() const
{
    vector<shared_ptr<Player>> winners;
    for (const auto &player : turnOrder)
    {
        if (playerDecks.find(player) == playerDecks.end())
            continue;

        const PlayerDeck &deck = playerDecks.at(player);
        if (deck.faceDown.empty() && deck.faceUp.empty())
        {
            winners.push_back(player);
        }
    }
    return winners;
}

bool GameState::isStalemate() const
{
    if (duelActive)
        return false;

    for (const auto &player : turnOrder)
    {
        if (playerDecks.find(player) != playerDecks.end())
        {
            if (!playerDecks.at(player).faceDown.empty())
                return false;
        }
    }
    return true;
}

void GameState::removePlayer(shared_ptr<Player> player)
{
    // usuwanie gracza z kolejki
    auto it = find(turnOrder.begin(), turnOrder.end(), player);

    // naprawianie tury
    if (it != turnOrder.end())
    {
        auto indexToRemove = static_cast<size_t>(distance(turnOrder.begin(), it));

        if (indexToRemove < currentTurnIndex)
        {
            currentTurnIndex--;
        }
        turnOrder.erase(it);
    }

    if (currentTurnIndex >= turnOrder.size())
    {
        currentTurnIndex = 0;
    }

    if (playerDecks.count(player))
    {
        collectFaceUpCards({player});
        deque<Card> &faceDown = playerDecks[player].faceDown;
        pot.insert(pot.end(), faceDown.begin(), faceDown.end());
        playerDecks.erase(player);
        dealCards(pot, turnOrder);
    }

    if (duelActive)
    {
        auto it = remove(activeDuelists.begin(), activeDuelists.end(), player);
        if (it != activeDuelists.end())
        {
            activeDuelists.erase(it, activeDuelists.end());
        }
        if (activeDuelists.size() < 2)
        {
            duelActive = false;
        }
    }
}

string GameState::getPlayersDeckSizes() const
{
    stringstream ss;
    ss << "DECK_SIZE";

    for (const auto &player : turnOrder)
    {
        if (playerDecks.find(player) != playerDecks.end())
        {
            ss << " " << player->getNick() << " " << playerDecks.at(player).faceDown.size();
        }
    }

    return ss.str();
}

int GameState::getPlayerDeckSize(shared_ptr<Player> player) const
{
    if (playerDecks.find(player) != playerDecks.end())
    {
        return playerDecks.at(player).faceDown.size();
    }
    return 0;
}

string GameState::getPlayersFaceUpCards() const
{
    stringstream ss;
    ss << "FACE_UP_CARDS";

    for (const auto &player : turnOrder)
    {
        if (playerDecks.find(player) != playerDecks.end())
        {
            const auto &deck = playerDecks.at(player);
            string topCard = deck.faceUp.empty() ? "0" : deck.faceUp.back().toString();

            ss << " " << player->getNick() << " " << topCard << " " << deck.faceUp.size();
        }
    }
    return ss.str();
}

string Card::toString() const
{
    return to_string((int)color) + to_string((int)shape);
}
