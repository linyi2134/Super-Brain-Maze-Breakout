#ifndef CHESS_H
#define CHESS_H

#include <string>

class Chess
{
public:
    Chess(int row, int col, const std::string& code, int ownerId);
    ~Chess();

    int row, col;               // 位置
    std::string code;           // 例如 "A3"
    int owner;                  // 0=玩家1, 1=玩家2, 2=中立
    bool alive;

    // 其他方法可以以后添加
};

#endif // CHESS_H
