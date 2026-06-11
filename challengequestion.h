#ifndef CHALLENGEQUESTION_H
#define CHALLENGEQUESTION_H

#include <vector>
#include <string>

class ChallengeQuestion
{
public:
    static const std::vector<std::vector<std::string>>& getSimpleBoard();   // 简单 5x5
    static const std::vector<std::vector<std::string>>& getNormalBoard();   // 普通 10x10
    static const std::vector<std::vector<std::string>>& getHardBoard();     // 困难 15x15
    static const std::vector<std::vector<std::string>>& getExpertBoard();   // 专家 20x20
    static const std::vector<std::vector<std::string>>& getNightmareBoard();// 噩梦 25x25

    static const std::vector<std::vector<std::string>>& getBoardByLevel(int level);
};

#endif