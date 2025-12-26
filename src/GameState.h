#pragma once

#include <vector>
#include <deque>
#include <string>
#include <map>
#include <memory>

using namespace std;

class Player;

// kolory kart
enum class CardColor
{
    RED,
    GREEN,
    BLUE,
    YELLOW,
    COUNT
};

// kształty kart
enum class CardShape
{
    CROSS_CIRCLE,
    CROSS_SQUARE,
    CROSS_TRIANGLE,
    EIGHT_FULL,
    EIGHT_HALF,
    EIGHT_S,
    LINE_DIAGONAL_1,
    LINE_DIAGONAL_2,
    LINE_VERTICAL,
    STAR_FULL,
    STAR_HOLE_1,
    STAR_HOLE_2,
    COUNT
};

// struktura opisująca kartę
struct Card
{
    CardColor color;
    CardShape shape;

    // porównanie kształtu kart
    // bool operator==(const Card &other) const
    // {
    //     return shape == other.shape;
    // }

    string toString() const;
};

class GameState
{
private:
    // talia gracza
    struct PlayerDeck
    {
        deque<Card> faceDown; // stos kart zakrytych
        deque<Card> faceUp;   // stos kart odkrytych
    };

    // przypisanie talii do graczy
    map<shared_ptr<Player>, PlayerDeck> playerDecks;

    // lista graczy w kolejności tur
    vector<shared_ptr<Player>> turnOrder;
    // indeks gracza mającego obecnie turę
    size_t currentTurnIndex;

    // pomocnicza zmienna służąca do agregacji kart z stołu do rozdania
    deque<Card> pot;

    // zmienna przechowująca czy jest jakiś aktywny pojedynek
    bool duelActive;
    // wektor z graczami uczestniczącymi w pojedynku (może być ich więcej niż 2)
    vector<shared_ptr<Player>> activeDuelists;

    deque<Card> generateDeck();
    void shuffleDeck(deque<Card> &deck);
    void dealCards(deque<Card> &deck, const vector<shared_ptr<Player>> &targets);
    void collectFaceUpCards(const vector<shared_ptr<Player>> &targets);
    void nextTurn();
    bool checkForDuels();

public:
    GameState();
    ~GameState();

    void initialize(const vector<shared_ptr<Player>> &players);

    string playerFlipCard(shared_ptr<Player> player); // zwraca id wyłożonej karty

    string playerGrabTotem(shared_ptr<Player> player);

    bool isGameOver() const;

    shared_ptr<Player> getWinner() const;

    string getFullState() const;

    void removePlayer(shared_ptr<Player> player);

    int getPlayerDeckSize(shared_ptr<Player> player) const;
};
