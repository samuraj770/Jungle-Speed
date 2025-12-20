#pragma once

#include <vector>
#include <deque>
#include <string>
#include <map>
#include <memory>
#include <algorithm>

using namespace std;

class Player;

// kolory kart
enum class CardColor
{
    RED,
    GREEN,
    BLUE,
    YELLOW
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
    STAR_HOLE_2
};

// struktura opisująca kartę
struct Card
{
    CardColor color;
    CardShape shape;

    // porównanie kształtu kart
    bool operator==(const Card &other) const
    {
        return shape == other.shape;
    }

    string toString() const;
};

class GameState
{
private:
    // talia gracza
    struct PlayerDeck
    {
        deque<Card> faceDown; // stos kart zakrytych
        deque<Card> fadeUp;   // stos kart odkrytych
    };

    // przypisanie talii do graczy
    map<shared_ptr<Player>, PlayerDeck> playerDecks;

    // lista graczy w kolejności tur
    vector<shared_ptr<Player>> turnOrder;
    // indeks gracza mającego obecnie turę
    size_t currentTurnIndex;

    // karty pod totemem ?
    deque<Card> pot;

    // zmienna przechowująca czy jest jakiś aktywny pojedynek
    bool duelActive;
    // wektor z graczami uczestniczącymi w pojedynku (może być ich więcej niż 2)
    vector<shared_ptr<Player>> activeDuelists;

    void generateDeck();
    void dealCards();
    void nextTurn();

public:
    GameState();
    ~GameState();

    void initialize(const vector<shared_ptr<Player>> &players);

    string playerFlipCard(shared_ptr<Player> player);

    string playerGrabTotem(shared_ptr<Player> player);

    bool isGameOver() const;

    shared_ptr<Player> getWinner() const;

    string getFullState() const;

    void removePlayer(shared_ptr<Player> player);
};
