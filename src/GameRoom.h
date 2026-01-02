#pragma once

#include <memory>
#include <vector>
#include <string>

class Player;
class GameState;

class GameRoom : public std::enable_shared_from_this<GameRoom>
{
private:
    std::string name;                             // kod pokoju
    std::vector<std::shared_ptr<Player>> players; // przechowuje liste graczy w pokoju
    std::shared_ptr<Player> host;                 // wskaznik na hosta
    bool gameActive;                              // czy gra jest aktywna
    std::unique_ptr<GameState> gameState;         // stan gry

    void startGame(std::shared_ptr<Player> player);

    void endGame();

    void assignNewHost();

    void checkResult();

public:
    GameRoom(std::string name, std::shared_ptr<Player> host);

    ~GameRoom();

    bool addPlayer(std::shared_ptr<Player> player);

    void handlePlayerDisconnect(std::shared_ptr<Player> player);

    void broadcastMessage(const std::string &message, std::shared_ptr<Player> excludePlayer = nullptr);

    void handleGameAction(std::shared_ptr<Player> player, const std::string &command);

    std::string getPlayerNicksString() const;

    bool isHost(std::shared_ptr<Player> player) const;

    std::string getName() const { return name; }

    bool isGameActive() const { return gameActive; }

    int getPlayerCount() const { return players.size(); }

    bool isNickTaken(const std::string &nick) const;
};
