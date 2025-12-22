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
            activeDuelists = owners;
            duelActive = true;
        }
    }

    return duelActive;
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
    this->activeDuelists.clear();

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

    nextTurn();
    return revealedCard.toString();
}

void GameState::removePlayer(shared_ptr<Player> player)
{
    // usuwanie gracza z kolejki
    auto it = remove(turnOrder.begin(), turnOrder.end(), player);
    if (it != turnOrder.end())
    {
        turnOrder.erase(it, turnOrder.end());
    }

    // naprawianie indeksu
    if (currentTurnIndex >= turnOrder.size())
    {
        currentTurnIndex = 0;
    }

    if (playerDecks.count(player))
    {
        PlayerDeck &deck = playerDecks[player];
        while (!deck.faceDown.empty())
        {
            pot.push_back(deck.faceDown.front());
            deck.faceDown.pop_front();
        }
        while (!deck.faceUp.empty())
        {
            pot.push_back(deck.faceUp.front());
            deck.faceUp.pop_front();
        }
        playerDecks.erase(player);
        dealCards(pot); // sprawdzić czy działa poprawnie
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

string Card::toString() const
{
    return to_string((int)color) + " " + to_string((int)shape);
}
