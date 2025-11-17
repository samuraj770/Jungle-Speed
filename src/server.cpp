#include <iostream>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

// #include "Player.h"
// #include "GameRoom.h"

#define PORT 2137
#define MAX_EVENTS 64

using namespace std;