#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <string>
#include <vector>
#include <QString>
#include <QStringList>
#include "board.h"
#include "player.h"
#include "gamedata.h"

enum GameMode { SingleRandom, AIBattle, Challenge };

class GameController
{
public:
    GameController();
    ~GameController();

    void init(int boardSize, const std::string& playerName, bool aiMode,
              const std::vector<std::vector<std::string>>* presetBoard = nullptr);

    void setGameMode(GameMode mode) { gameMode = mode; }
    GameMode getGameMode() const { return gameMode; }
    int getCurrentPlayer() const { return currentPlayer; }
    QString getCurrentPlayerName() const;

    bool selectInitialChess(int row, int col);
    bool isInitialSelectionDone() const { return selectionDone; }
    int getRemainingToSelect() const;

    void movePlayer(Direction dir);
    void aiMove();

    Player* getPlayer() { return player; }
    Player* getAIPlayer() { return aiPlayer; }
    Board* getBoard() { return board; }
    bool isGameOver() const { return gameOver; }

    bool hasAnyLegalMove() const;
    bool hasAnyLegalMoveForAI() const;
    bool isEatableTarget(int row, int col) const;
    QString getHintText() const;

    QStringList getMoveHistory() const { return moveHistory; }
    void clearMoveHistory() { moveHistory.clear(); }

    SaveData toSaveData() const;
    void fromSaveData(const SaveData& data);
    int getCurrentTurn() const { return currentTurn; }

    SaveData getInitialSaveData() const { return initialSaveData; }
    void captureInitialState();

private:
    void updateRemainingChess();
    void checkGameOver();

    SaveData initialSaveData;
    int currentTurn;
    Board* board;
    Player* player;
    Player* aiPlayer;
    bool aiMode;
    bool gameOver;
    bool selectionDone;
    int neededCount;
    int selectedCount;
    QStringList moveHistory;

    GameMode gameMode;
    int currentPlayer;
};

#endif