#include <unistd.h>
#include <sstream>

#include "Player.h"
#include "Server.h"

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
    if (command == "CREATE_ROOM")
    {
        server->createRoom(shared_from_this());
    }
    else if (command == "JOIN_ROOM")
    {
        string roomName;
        issMessage >> roomName;
        server->joinRoom(roomName, shared_from_this());
        issMessage >> this->nick;
        cout << "Nick gracza: " << this->nick << endl;
    }
    else if (command == "QUIT_ROOM")
    {
        quitRoom();
    }
    else
    {
        auto room = getRoom();
        if (room)
        {
            // room->handleGameAction();
        }
    }
}

void Player::sendMessage(const string &message)
{
    write(fd, (message + "%").c_str(), message.length() + 1); // sprawidzic \0
}

void Player::setRoom(shared_ptr<GameRoom> room)
{
    this->currentRoom = room;
}

void Player::quitRoom()
{
}

shared_ptr<GameRoom> Player::getRoom()
{
    return shared_ptr<GameRoom>();
}
