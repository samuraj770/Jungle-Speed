#include <unistd.h>
#include <sstream>

#include "Player.h"

// #include "GameRoom.h"

using namespace std;

Player::Player(int client_fd) : fd(client_fd) {}

Player::~Player()
{
    close(fd);
}

void Player::processMessage(const string &message, Server *server)
{
    istringstream issMessage(message);
    string command;
    issMessage >> command;
}

void Player::sendMessage(const string &message)
{
}

void Player::setRoom(shared_ptr<GameRoom> room)
{
}

void Player::leaveRoom()
{
}
