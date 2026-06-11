#ifndef CHESS_H
#define CHESS_H

#include <string>

class Chess
{
public:
    Chess(int row, int col, const std::string& code, int ownerId);
    ~Chess();

    int row, col;
    std::string code;
    int owner;
    bool alive;
};

#endif