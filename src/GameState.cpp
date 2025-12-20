#include <random>
#include <algorithm>

#include "Server.h"
#include "Player.h"
#include "GameRoom.h"
#include "GameState.h"

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
            for (int k = 0; k < 2; k++)
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

void GameState::dealCards(deque<Card> &deck)
{
    size_t playerIndex = 0;

    while (!deck.empty())
    {
        auto currentPlayer = turnOrder[playerIndex];
        playerDecks[currentPlayer].faceDown.push_back(deck.front());
        deck.pop_front();
        playerIndex = (playerIndex + 1) % turnOrder.size(); // liczba aktywnych graczy @todo
    }
}

void GameState::initialize(const vector<shared_ptr<Player>> &players)
{
    this->turnOrder = players;
    this->currentTurnIndex = 0;
    this->playerDecks.clear();
    this->pot.clear();
    this->duelActive = false;

    deque<Card> fullDeck = generateDeck();
    shuffleDeck(fullDeck);
    dealCards(fullDeck);
}