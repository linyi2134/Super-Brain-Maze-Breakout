#include "gamecontroller.h"


GameController::GameController(int boardSize, const std::string& playerName)
    : board(new Board(boardSize)), player(new Player(playerName, 0)),
    gameOver(false), selectionDone(false), selectedCount(0)
{

    if (boardSize == 5) neededCount = 1;
    else if (boardSize == 10) neededCount = 2;
    else if (boardSize == 15) neededCount = 3;
    else if (boardSize == 20) neededCount = 4;
    else if (boardSize == 25) neededCount = 5;
    else neededCount = 2;
}

GameController::~GameController()
{
    delete board;
    delete player;
}

bool GameController::selectInitialChess(int row, int col)
{
    if (selectionDone) return false;
    if (selectedCount >= neededCount) return false;

    Chess* ch = board->getChess(row, col);
    // 只能选择中立棋子（owner == 2）
    if (!ch || ch->owner != 2) return false;

    // 改为玩家棋子
    ch->owner = 0;
    selectedCount++;
    player->remainingChess++;

    if (selectedCount == neededCount) {
        selectionDone = true;
    }
    return true;
}

int GameController::getRemainingToSelect() const
{
    if (selectionDone) return 0;
    return neededCount - selectedCount;
}

void GameController::movePlayer(Direction dir)
{
    if (!selectionDone || gameOver) return;

    int totalScoreGain = 0;
    // 收集当前玩家所有存活的棋子
    std::vector<Chess*> myChess;
    int size = board->getSize();
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j) {
            Chess* ch = board->getChess(i, j);
            if (ch && ch->alive && ch->owner == player->id)
                myChess.push_back(ch);
        }

    bool merged = false;
    for (Chess* ch : myChess) {
        int gain = 0;
        MoveResult res = board->moveSingleChess(ch, dir, player->id, gain);
        if (res == Merged) {
            totalScoreGain += gain;
            merged = true;
            break;
        }
    }

    if (merged) {
        player->addScore(totalScoreGain);
    }
}