#include <unistd.h>

#include "Player.h"

// #include "GameRoom.h"

using namespace std;

Player::Player(int client_fd) : fd(client_fd) {}

Player::~Player()
{
    close(fd);
}
