#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Player
{
public:
    Player(const std::string& name, int id);
    ~Player();

    std::string name;
    int id;
    int score;
    int remainingChess;
    double timeUsed;

    void addScore(int points);
    void setRemainingChess(int count);
};

#endif