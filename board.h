#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>
#include <utility>
#include "chess.h"

enum Direction { Up = 0, Down = 1, Left = 2, Right = 3 };
enum MoveResult { Blocked = 0, Moved = 1, Merged = 2 };

extern const int dr[4];
extern const int dc[4];

class Board
{
public:
    Board(int size);
    ~Board();

    int getSize() const;
    Chess* getChess(int row, int col) const;
    void setChess(int row, int col, Chess* chess);

    MoveResult moveSingleChess(Chess* chess, Direction dir, int ownerId, int& scoreGain);
    bool canMerge(Chess* a, Chess* b) const;
    std::string mergeCodes(const std::string& code1, const std::string& code2);
    void initRandomBoard();
    bool canEat(Chess* attacker, int targetRow, int targetCol) const;
    std::vector<std::pair<int,int>> getReachableEmptyCells(int row, int col) const;
    void loadFromPreset(const std::vector<std::vector<std::string>>& codes);

private:
    int size;
    std::vector<std::vector<Chess*>> grid;
};

#endif