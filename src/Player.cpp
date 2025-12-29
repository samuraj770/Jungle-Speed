#include <unistd.h>
#include <sstream>

#include "Player.h"
#include "Server.h"
#include "GameRoom.h"

using namespace std;

Player::Player(int client_fd) : fd(client_fd) {}

Player::~Player()
{
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

void Player::processMessage(const string &message, Server *server)
{
    istringstream issMessage(message);
    string command;
    issMessage >> command;
    if (command == "CREATE_ROOM")
    {
        server->createRoom(shared_from_this());
        issMessage >> this->nick;
    }
    else if (command == "JOIN_ROOM")
    {
        string roomName;
        issMessage >> roomName;
        issMessage >> this->nick;
        cout << "Nick gracza: " << this->nick << endl;
        server->joinRoom(roomName, shared_from_this());
    }
    else
    {
        auto room = getRoom();
        if (room)
        {
            room->handleGameAction(shared_from_this(), command);
        }
    }
}

void Player::sendMessage(const string &message)
{
    write(fd, (message + "%").c_str(), message.length() + 1);
}

void Player::setRoom(shared_ptr<GameRoom> room)
{
    this->currentRoom = room;
}

void Player::quitRoom()
{
    auto lockedRoom = currentRoom.lock();
    if (lockedRoom)
    {
        lockedRoom->handlePlayerDisconnect(shared_from_this());
        currentRoom.reset();
    }
}

shared_ptr<GameRoom> Player::getRoom()
{
    return currentRoom.lock();
}
