#include <iostream>
#include <memory>

#include "Server.h"

#define PORT 2137

using namespace std;

int main()
{
    unique_ptr<Server> gameServer;

    try
    {
        gameServer = make_unique<Server>(PORT);

        gameServer->run();
    }
    catch (const exception &e)
    {
        cerr << "BŁĄD: Tworzenie serwera" << e.what() << endl;
        return 1;
    }

    return 0;
}