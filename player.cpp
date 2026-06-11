#include "player.h"

Player::Player(const std::string& name, int id)
    : name(name), id(id), score(0), remainingChess(0), timeUsed(0.0)
{
}

Player::~Player() = default;

void Player::addScore(int points)
{
    score += points;
}

void Player::setRemainingChess(int count)
{
    remainingChess = count;
}