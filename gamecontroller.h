#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include "board.h"
#include "player.h"

class GameController
{
public:
    GameController(int boardSize, const std::string& playerName);
    ~GameController();

    // 选择初始棋子
    bool selectInitialChess(int row, int col);
    // 是否已完成初始棋子选择
    bool isInitialSelectionDone() const { return selectionDone; }
    // 还需要选多少个初始棋子
    int getRemainingToSelect() const;

    // 移动
    void movePlayer(Direction dir);

    Player* getPlayer() { return player; }
    Board* getBoard() { return board; }
    bool isGameOver() const { return gameOver; }

private:
    Board* board;
    Player* player;
    bool gameOver;
    bool selectionDone;
    int neededCount;
    int selectedCount;
};

#endif