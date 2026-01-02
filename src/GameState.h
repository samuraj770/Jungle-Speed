#pragma once

#include <vector>
#include <deque>
#include <string>
#include <map>
#include <memory>
#include <chrono>

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

    std::string toString() const;
};

class GameState
{
private:
    // talia gracza
    struct PlayerDeck
    {
        std::deque<Card> faceDown; // stos kart zakrytych
        std::deque<Card> faceUp;   // stos kart odkrytych
    };

    // przypisanie talii do graczy
    std::map<std::shared_ptr<Player>, PlayerDeck> playerDecks;

    // lista graczy w kolejności tur
    std::vector<std::shared_ptr<Player>> turnOrder;

    // indeks gracza mającego obecnie turę
    size_t currentTurnIndex;

    // pomocnicza zmienna służąca do agregacji kart z stołu do rozdania
    std::deque<Card> pot;

    // zmienna przechowująca czy jest jakiś aktywny pojedynek
    bool duelActive;

    // wektor z graczami uczestniczącymi w pojedynku (może być ich więcej niż 2)
    std::vector<std::shared_ptr<Player>> activeDuelists;

    // czas zakończenia ostatniego pojedunku
    std::chrono::steady_clock::time_point lastDuelEndTime;

    std::deque<Card> generateDeck();

    void shuffleDeck(std::deque<Card> &deck);

    void dealCards(std::deque<Card> &deck, const std::vector<std::shared_ptr<Player>> &targets);

    void collectFaceUpCards(const std::vector<std::shared_ptr<Player>> &targets);

    void nextTurn();

    bool checkForDuels();

public:
    GameState();

    ~GameState();

    void initialize(const std::vector<std::shared_ptr<Player>> &players);

    std::string playerFlipCard(std::shared_ptr<Player> player);

    std::string playerGrabTotem(std::shared_ptr<Player> player);

    std::vector<std::shared_ptr<Player>> checkWinners() const;

    bool isStalemate() const;

    void removePlayer(std::shared_ptr<Player> player);

    std::string getPlayersDeckSizes() const;

    int getPlayerDeckSize(std::shared_ptr<Player> player) const;

    int getActivePlayersCount() const { return turnOrder.size(); }

    std::string getPlayersFaceUpCards() const;

    std::shared_ptr<Player> getCurrentPlayer() const;
};
