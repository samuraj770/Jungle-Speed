#pragma once

#include <memory>
#include <vector>
#include <string>

using namespace std;

class Player;
class GameState;

class GameRoom : public enable_shared_from_this<GameRoom>
{
private:
    string name;                        // kod pokoju
    vector<shared_ptr<Player>> players; // przechowuje liste graczy w pokoju
    shared_ptr<Player> host;            // wskaznik na hosta
    bool gameActive;                    // czy gra jest aktywna
    unique_ptr<GameState> gameState;    // stan gry

    void startGame(shared_ptr<Player> player);

    void endGame();

    void assignNewHost();

public:
    GameRoom(string name, shared_ptr<Player> host);

    ~GameRoom();

    void addPlayer(shared_ptr<Player> player);

    void handlePlayerDisconnect(shared_ptr<Player> player);

    void broadcastMessage(const string &message, shared_ptr<Player> excludePlayer = nullptr);

    void handleGameAction(shared_ptr<Player> player, const string &command);

    string getPlayerNicks() const;

    bool isHost(shared_ptr<Player> player) const;

    string getName() const { return name; }

    bool isGameActive() const { return gameActive; }

    int getPlayerCount() const { return players.size(); }
};
