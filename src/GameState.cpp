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

void GameState::nextTurn()
{
    currentTurnIndex = (currentTurnIndex + 1) % turnOrder.size();
}

GameState::GameState() {}

GameState::~GameState() {}

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

string GameState::playerFlipCard(shared_ptr<Player> player)
{
    if (player != turnOrder[currentTurnIndex])
    {
        cout << "Błąd kolejki" << endl;
        return "";
    }

    // na potrzeby testów końcowo pojedynek nie może blokować tury
    if (duelActive)
    {
        cout << "Trwa pojedynek" << endl;
        return "";
    }

    PlayerDeck &deck = playerDecks[player];

    if (deck.faceDown.empty())
    {
        nextTurn();          // jeśli gracz nie ma kart na stosie zakrytych to pomijamy jego turę
        return "CARD_ID -1"; // informujemy klienta o tym że gracz wykonał turę a nie ma kart
    }

    Card revealedCard = deck.faceDown.front();
    deck.faceDown.pop_front();
    deck.faceUp.push_back(revealedCard);

    // bool newDuel = checkForDuels();
    // if (newDuel)
    // {
    //     cout << "Gracze w pojedynku" << endl;
    //     for (auto duelist : activeDuelists)
    //     {
    //         cout << duelist->getNick() << endl;
    //     }
    // }
    // else
    // {
    //     nextTurn();
    // }

    return revealedCard.toString();
}

string Card::toString() const
{
    return to_string((int)color) + " " + to_string((int)shape);
}
