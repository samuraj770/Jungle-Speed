#include <iostream>
#include <random>

#include "Utils.h"

#define MAX_LENGTH 6

using namespace std;

string generateRoomCode()
{
    const string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    random_device rd;
    mt19937 generator(rd());

    uniform_int_distribution<> distribution(0, characters.length() - 1);

    string code;

    for (int i = 0; i < MAX_LENGTH; i++)
    {
        code += characters[distribution(generator)];
    }

    return code;
}