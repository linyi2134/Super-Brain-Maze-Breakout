#include "gamecontroller.h"
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <QDebug>

static QString dirToString(Direction dir) {
    switch (dir) {
    case Up:    return "上";
    case Down:  return "下";
    case Left:  return "左";
    case Right: return "右";
    default:    return "未知";
    }
}

GameController::GameController()
    : board(nullptr), player(nullptr), aiPlayer(nullptr),
    aiMode(false), gameOver(false), selectionDone(false),
    neededCount(0), selectedCount(0), gameMode(SingleRandom), currentPlayer(0)
{
    qDebug() << "GameController created";
    currentTurn = 0;
}

GameController::~GameController()
{
    delete board; delete player; delete aiPlayer;
}

void GameController::init(int boardSize, const std::string& playerName, bool aiMode,
                          const std::vector<std::vector<std::string>>* presetBoard)
{
    qDebug() << "GameController init, boardSize=" << boardSize;
    delete board; delete player; delete aiPlayer;
    board = nullptr; player = nullptr; aiPlayer = nullptr;
    currentTurn = 0;

    this->aiMode = aiMode;
    gameOver = false;
    selectionDone = false;
    selectedCount = 0;
    moveHistory.clear();

    if (aiMode) gameMode = AIBattle;
    else if (presetBoard != nullptr) gameMode = Challenge;
    else gameMode = SingleRandom;

    auto getBaseCount = [boardSize]() -> int {
        if (boardSize == 5) return 1;
        if (boardSize == 10) return 2;
        if (boardSize == 15) return 3;
        if (boardSize == 20) return 4;
        if (boardSize == 25) return 5;
        return 2;
    };

    int baseCount = getBaseCount();

    // 根据模式设置 neededCount
    if (gameMode == AIBattle) {
        // AI对决：双方棋子数量相同，使用基础数量 1,2,3,4,5
        neededCount = baseCount;
    } else {
        // 单人随机 和 挑战模式（含自建题库）使用统一规则：1,2,4,6,8
        if (boardSize == 5) neededCount = 1;
        else if (boardSize == 10) neededCount = 2;
        else if (boardSize == 15) neededCount = 4;
        else if (boardSize == 20) neededCount = 6;
        else if (boardSize == 25) neededCount = 8;
        else neededCount = 2;
    }

    selectedCount = 0;

    board = new Board(boardSize);
    player = new Player(playerName, 0);

    if (presetBoard != nullptr) {
        qDebug() << "Loading preset board of size:" << presetBoard->size();
        for (int i = 0; i < 5 && i < (int)presetBoard->size(); ++i) {
            for (int j = 0; j < 5 && j < (int)(*presetBoard)[i].size(); ++j) {
                qDebug() << "(" << i << "," << j << "):" << (*presetBoard)[i][j].c_str();
            }
        }
        board->loadFromPreset(*presetBoard);
    }

    if (aiMode) aiPlayer = new Player("AI", 1);
    else aiPlayer = nullptr;

    qDebug() << "GameController init finished";
}

bool GameController::selectInitialChess(int row, int col)
{
    if (selectionDone) return false;
    if (selectedCount >= neededCount) return false;

    Chess* ch = board->getChess(row, col);
    if (!ch || ch->owner != 2) return false;

    ch->owner = 0;
    selectedCount++;
    player->remainingChess++;

    if (selectedCount == neededCount) {
        selectionDone = true;
        if (aiMode && aiPlayer) {
            std::vector<Chess*> candidates;
            int size = board->getSize();
            for (int i=0; i<size; ++i)
                for (int j=0; j<size; ++j)
                    if (board->getChess(i,j)->owner == 2) candidates.push_back(board->getChess(i,j));
            for (int i=candidates.size()-1; i>0; --i) {
                int j = rand() % (i+1);
                std::swap(candidates[i], candidates[j]);
            }
            for (int i=0; i<neededCount && i<(int)candidates.size(); ++i) {
                candidates[i]->owner = 1;
                aiPlayer->remainingChess++;
            }
        }
        captureInitialState();
    }
    return true;
}

int GameController::getRemainingToSelect() const {
    return neededCount - selectedCount;
}

void GameController::movePlayer(Direction dir) {
    if (!board || !player) return;
    if (!selectionDone || gameOver) return;

    if (gameMode == SingleRandom || gameMode == Challenge) {
        // 单人/挑战模式
        int totalGain = 0;
        std::vector<Chess*> myChess;
        int size = board->getSize();
        for (int i=0; i<size; ++i)
            for (int j=0; j<size; ++j)
                if (board->getChess(i,j) && board->getChess(i,j)->alive && board->getChess(i,j)->owner == player->id)
                    myChess.push_back(board->getChess(i,j));
        bool merged = false;
        for (Chess* c : myChess) {
            int gain = 0;
            if (board->moveSingleChess(c, dir, player->id, gain) == Merged) {
                totalGain += gain;
                merged = true;
                break;
            }
        }
        if (merged) player->addScore(totalGain);
        updateRemainingChess();
        checkGameOver();
        QString step = QString("玩家 %1").arg(dirToString(dir));
        if (merged) step += QString(" 合并，得分 +%1").arg(totalGain);
        else step += " 移动";
        moveHistory.append(step);
        currentTurn = 0;
    }
    else if (gameMode == AIBattle) {
        if (currentPlayer != 0) return;
        int totalGain = 0;
        std::vector<Chess*> myChess;
        int size = board->getSize();
        for (int i=0; i<size; ++i)
            for (int j=0; j<size; ++j)
                if (board->getChess(i,j) && board->getChess(i,j)->alive && board->getChess(i,j)->owner == player->id)
                    myChess.push_back(board->getChess(i,j));
        bool merged = false;
        for (Chess* c : myChess) {
            int gain = 0;
            if (board->moveSingleChess(c, dir, player->id, gain) == Merged) {
                totalGain += gain;
                merged = true;
                break;
            }
        }
        if (merged) player->addScore(totalGain);
        updateRemainingChess();
        checkGameOver();
        QString step = QString("玩家 %1").arg(dirToString(dir));
        if (merged) {
            step += QString(" 合并，得分 +%1").arg(totalGain);
            currentPlayer = 1;
        } else {
            step += " 移动";
            if (!hasAnyLegalMove()) {
                currentPlayer = 1;
                step += " (无合法移动，跳过)";
            }
        }
        moveHistory.append(step);
        currentTurn = currentPlayer;
    }
}

void GameController::aiMove() {
    if (!aiMode || gameMode != AIBattle) return;
    if (!selectionDone || gameOver) return;
    if (currentPlayer != 1) return;

    static int recursionDepth = 0;
    if (recursionDepth > 100) { recursionDepth = 0; return; }
    recursionDepth++;

    int size = board->getSize();
    Direction bestDir = Up;
    bool found = false;
    for (int i=0; i<size && !found; ++i)
        for (int j=0; j<size && !found; ++j) {
            Chess* c = board->getChess(i,j);
            if (c && c->alive && c->owner == 1) {
                for (int d=0; d<4; ++d) {
                    int nr = i + dr[d], nc = j + dc[d];
                    if (board->canEat(c, nr, nc)) {
                        bestDir = static_cast<Direction>(d);
                        found = true;
                        break;
                    }
                }
            }
        }

    Direction dir = found ? bestDir : static_cast<Direction>(rand() % 4);

    int totalGain = 0;
    std::vector<Chess*> aiChess;
    for (int i=0; i<size; ++i)
        for (int j=0; j<size; ++j)
            if (board->getChess(i,j) && board->getChess(i,j)->alive && board->getChess(i,j)->owner == 1)
                aiChess.push_back(board->getChess(i,j));

    bool merged = false;
    for (Chess* c : aiChess) {
        int gain = 0;
        if (board->moveSingleChess(c, dir, 1, gain) == Merged) {
            totalGain += gain;
            merged = true;
            break;
        }
    }
    if (merged && aiPlayer) aiPlayer->addScore(totalGain);
    updateRemainingChess();
    checkGameOver();

    QString step = QString("AI %1").arg(dirToString(dir));
    if (merged) {
        step += QString(" 合并，得分 +%1").arg(totalGain);
        currentPlayer = 0;
        recursionDepth = 0;
    } else {
        step += " 移动";
        if (!hasAnyLegalMoveForAI()) {
            currentPlayer = 0;
            step += " (无合法移动，跳过)";
        } else {
            aiMove();
            return;
        }
    }
    moveHistory.append(step);
    currentTurn = currentPlayer;
}

bool GameController::hasAnyLegalMove() const {
    if (!board || !player) return false;
    if (!selectionDone || gameOver) return false;
    int size = board->getSize();
    for (int i=0; i<size; ++i)
        for (int j=0; j<size; ++j) {
            Chess* attacker = board->getChess(i,j);
            if (attacker && attacker->alive && attacker->owner == player->id) {
                for (int d=0; d<4; ++d) {
                    int nr = i + dr[d], nc = j + dc[d];
                    if (board->canEat(attacker, nr, nc)) return true;
                }
                auto reachable = board->getReachableEmptyCells(i,j);
                for (auto pos : reachable) {
                    for (int d=0; d<4; ++d) {
                        int nr = pos.first + dr[d], nc = pos.second + dc[d];
                        if (board->canEat(attacker, nr, nc)) return true;
                    }
                }
            }
        }
    return false;
}

bool GameController::hasAnyLegalMoveForAI() const {
    if (!board || !aiPlayer) return false;
    if (!selectionDone || gameOver) return false;
    int size = board->getSize();
    for (int i=0; i<size; ++i)
        for (int j=0; j<size; ++j) {
            Chess* attacker = board->getChess(i,j);
            if (attacker && attacker->alive && attacker->owner == 1) {
                for (int d=0; d<4; ++d) {
                    int nr = i + dr[d], nc = j + dc[d];
                    if (board->canEat(attacker, nr, nc)) return true;
                }
                auto reachable = board->getReachableEmptyCells(i,j);
                for (auto pos : reachable) {
                    for (int d=0; d<4; ++d) {
                        int nr = pos.first + dr[d], nc = pos.second + dc[d];
                        if (board->canEat(attacker, nr, nc)) return true;
                    }
                }
            }
        }
    return false;
}

void GameController::updateRemainingChess() {
    if (player) player->remainingChess = 0;
    if (aiPlayer) aiPlayer->remainingChess = 0;
    if (!board) return;
    int size = board->getSize();
    for (int i=0; i<size; ++i)
        for (int j=0; j<size; ++j) {
            Chess* c = board->getChess(i,j);
            if (c && c->alive) {
                if (c->owner == 0) player->remainingChess++;
                else if (c->owner == 1 && aiPlayer) aiPlayer->remainingChess++;
            }
        }
}

void GameController::checkGameOver() {
    if (gameMode == AIBattle) {
        if (player->remainingChess == 0 || aiPlayer->remainingChess == 0) gameOver = true;
    } else {
        if (player->remainingChess == 0) gameOver = true;
    }
}

bool GameController::isEatableTarget(int row, int col) const
{
    if (!board || !player) return false;
    Chess* target = board->getChess(row, col);
    if (!target || target->owner == player->id) return false;
    int size = board->getSize();
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            Chess* attacker = board->getChess(i, j);
            if (attacker && attacker->alive && attacker->owner == player->id) {
                for (int d = 0; d < 4; ++d) {
                    int nr = i + dr[d];
                    int nc = j + dc[d];
                    if (nr == row && nc == col && board->canEat(attacker, row, col))
                        return true;
                }
                std::vector<std::pair<int,int>> reachable = board->getReachableEmptyCells(i, j);
                for (auto pos : reachable) {
                    for (int d = 0; d < 4; ++d) {
                        int nr = pos.first + dr[d];
                        int nc = pos.second + dc[d];
                        if (nr == row && nc == col && board->canEat(attacker, row, col))
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

QString GameController::getHintText() const
{
    if (!board || !player) return "";
    if (!selectionDone || gameOver) return "";
    QStringList hints;
    int size = board->getSize();
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            Chess* attacker = board->getChess(i, j);
            if (attacker && attacker->alive && attacker->owner == player->id) {
                for (int d = 0; d < 4; ++d) {
                    int nr = i + dr[d];
                    int nc = j + dc[d];
                    if (board->canEat(attacker, nr, nc)) {
                        Chess* target = board->getChess(nr, nc);
                        hints << QString("(%1,%2)[%3] → (%4,%5)[%6]")
                                     .arg(i).arg(j).arg(attacker->code.c_str())
                                     .arg(nr).arg(nc).arg(target->code.c_str());
                    }
                }
                std::vector<std::pair<int,int>> reachable = board->getReachableEmptyCells(i, j);
                for (auto pos : reachable) {
                    for (int d = 0; d < 4; ++d) {
                        int nr = pos.first + dr[d];
                        int nc = pos.second + dc[d];
                        if (board->canEat(attacker, nr, nc)) {
                            Chess* target = board->getChess(nr, nc);
                            hints << QString("(%1,%2)[%3] → 空格(%4,%5) → (%6,%7)[%8]")
                                         .arg(i).arg(j).arg(attacker->code.c_str())
                                         .arg(pos.first).arg(pos.second)
                                         .arg(nr).arg(nc).arg(target->code.c_str());
                        }
                    }
                }
            }
        }
    }
    if (hints.isEmpty()) return "当前无可吃的棋子";
    return "可吃路线：\n" + hints.join("\n");
}

SaveData GameController::toSaveData() const {
    SaveData data;
    data.version = 1;
    data.mode = (gameMode == SingleRandom ? 0 : (gameMode == AIBattle ? 1 : 2));
    data.boardSize = board->getSize();
    data.playerName = QString::fromStdString(player->name);
    data.playerScore = player->score;
    data.playerRemainingChess = player->remainingChess;
    data.hasAI = (aiPlayer != nullptr);
    if (data.hasAI) {
        data.aiName = QString::fromStdString(aiPlayer->name);
        data.aiScore = aiPlayer->score;
        data.aiRemainingChess = aiPlayer->remainingChess;
    }
    data.selectionDone = selectionDone;
    data.selectedCount = selectedCount;
    data.neededCount = neededCount;
    data.elapsedSeconds = 0;
    data.isTiming = false;
    data.currentTurn = currentTurn;
    data.moveHistory = moveHistory;
    data.fixed = false;

    int size = board->getSize();
    data.board.resize(size, std::vector<ChessData>(size));
    for (int i=0; i<size; ++i)
        for (int j=0; j<size; ++j) {
            Chess* c = board->getChess(i,j);
            if (c) data.board[i][j] = ChessData(c->code, c->owner, c->alive);
            else data.board[i][j] = ChessData("", 2, false);
        }
    return data;
}

void GameController::fromSaveData(const SaveData& data) {
    delete board; delete player; delete aiPlayer;
    board = nullptr; player = nullptr; aiPlayer = nullptr;

    gameMode = static_cast<GameMode>(data.mode);
    aiMode = (gameMode == AIBattle);
    selectionDone = data.selectionDone;
    selectedCount = data.selectedCount;
    neededCount = data.neededCount;
    moveHistory = data.moveHistory;
    currentTurn = data.currentTurn;
    gameOver = false;

    board = new Board(data.boardSize);
    int size = data.boardSize;
    for (int i=0; i<size; ++i)
        for (int j=0; j<size; ++j) {
            const ChessData& cd = data.board[i][j];
            if (cd.alive) board->setChess(i,j, new Chess(i,j,cd.code,cd.owner));
            else board->setChess(i,j, nullptr);
        }

    player = new Player(data.playerName.toStdString(), 0);
    player->score = data.playerScore;
    player->remainingChess = data.playerRemainingChess;

    if (data.hasAI) {
        aiPlayer = new Player(data.aiName.toStdString(), 1);
        aiPlayer->score = data.aiScore;
        aiPlayer->remainingChess = data.aiRemainingChess;
    } else {
        aiPlayer = nullptr;
    }
}

void GameController::captureInitialState() {
    initialSaveData = toSaveData();
    initialSaveData.moveHistory.clear();
}

QString GameController::getCurrentPlayerName() const {
    if (gameMode == AIBattle) {
        return (currentPlayer == 0) ? QString::fromStdString(player->name) : "AI";
    } else {
        return QString::fromStdString(player->name);
    }
}