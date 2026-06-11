#include "board.h"
#include <cstdlib>
#include <ctime>
#include <queue>
#include <set>
#include <utility>
#include <QRandomGenerator>
#include <QDebug>

const int dr[4] = {-1, 1, 0, 0};
const int dc[4] = {0, 0, -1, 1};

Board::Board(int size) : size(size)
{
    grid.resize(size, std::vector<Chess*>(size, nullptr));
}

Board::~Board()
{
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            delete grid[i][j];
}

int Board::getSize() const { return size; }

Chess* Board::getChess(int row, int col) const
{
    if (row < 0 || row >= size || col < 0 || col >= size) return nullptr;
    return grid[row][col];
}

void Board::setChess(int row, int col, Chess* chess)
{
    if (row >= 0 && row < size && col >= 0 && col < size)
        grid[row][col] = chess;
}

void Board::initRandomBoard()
{
    QRandomGenerator *gen = QRandomGenerator::global();
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            char letter = 'A' + (gen->bounded(5));
            int num = gen->bounded(10);
            std::string code = std::string(1, letter) + std::to_string(num);
            grid[i][j] = new Chess(i, j, code, 2);
        }
    }
    qDebug() << "initRandomBoard finished, size=" << size;
}

bool Board::canMerge(Chess* a, Chess* b) const
{
    if (!a || !b) return false;
    if (a->code == b->code) return true;
    if (a->code[0] == b->code[0]) return true;
    if (a->code.substr(1) == b->code.substr(1)) return true;
    return false;
}

std::string Board::mergeCodes(const std::string& code1, const std::string& code2)
{
    char c1 = code1[0], c2 = code2[0];
    int n1 = std::stoi(code1.substr(1));
    int n2 = std::stoi(code2.substr(1));

    if (c1 == c2) {
        int newNum = (n1 + n2) % 10;
        return std::string(1, c1) + std::to_string(newNum);
    } else if (n1 == n2) {
        int v1 = c1 - 'A' + 1;
        int v2 = c2 - 'A' + 1;
        int newV = (v1 + v2) % 5;
        if (newV == 0) newV = 5;
        char newC = 'A' + newV - 1;
        return std::string(1, newC) + std::to_string(n1);
    }
    return code1;
}

MoveResult Board::moveSingleChess(Chess* chess, Direction dir, int ownerId, int& scoreGain)
{
    if (!chess || chess->owner != ownerId) return Blocked;
    int nr = chess->row + dr[dir];
    int nc = chess->col + dc[dir];
    if (nr < 0 || nr >= size || nc < 0 || nc >= size) return Blocked;

    Chess* target = grid[nr][nc];
    if (target == nullptr) {
        setChess(nr, nc, chess);
        setChess(chess->row, chess->col, nullptr);
        chess->row = nr;
        chess->col = nc;
        return Moved;
    } else if (canMerge(chess, target)) {
        std::string newCode = mergeCodes(chess->code, target->code);
        chess->code = newCode;
        delete target;
        setChess(nr, nc, chess);
        setChess(chess->row, chess->col, nullptr);
        chess->row = nr;
        chess->col = nc;
        scoreGain += 1;
        return Merged;
    } else {
        return Blocked;
    }
}

bool Board::canEat(Chess* attacker, int targetRow, int targetCol) const
{
    if (!attacker || !attacker->alive) return false;
    Chess* target = getChess(targetRow, targetCol);
    if (!target || target->owner == attacker->owner) return false;
    return canMerge(attacker, target);
}

std::vector<std::pair<int,int>> Board::getReachableEmptyCells(int attackerRow, int attackerCol) const
{
    std::vector<std::pair<int,int>> result;
    std::queue<std::pair<int,int>> q;
    std::set<std::pair<int,int>> visited;

    Chess* attacker = getChess(attackerRow, attackerCol);
    if (!attacker) return result;

    // 从攻击者的四方向开始，将空格或队友格子加入队列
    for (int d = 0; d < 4; ++d) {
        int nr = attackerRow + dr[d];
        int nc = attackerCol + dc[d];
        if (nr < 0 || nr >= size || nc < 0 || nc >= size) continue;
        Chess* neighbor = grid[nr][nc];
        if (neighbor == nullptr || (neighbor->alive && neighbor->owner == attacker->owner)) {
            if (visited.find({nr, nc}) == visited.end()) {
                visited.insert({nr, nc});
                q.push({nr, nc});
            }
        }
    }

    while (!q.empty()) {
        std::pair<int,int> pos = q.front(); q.pop();
        int r = pos.first, c = pos.second;
        // 只将真正的空格加入结果（队友位置不算空格，但作为路径）
        if (grid[r][c] == nullptr) {
            result.push_back(pos);
        }
        for (int d = 0; d < 4; ++d) {
            int nr = r + dr[d];
            int nc = c + dc[d];
            if (nr < 0 || nr >= size || nc < 0 || nc >= size) continue;
            Chess* neighbor = grid[nr][nc];
            if (neighbor == nullptr || (neighbor->alive && neighbor->owner == attacker->owner)) {
                if (visited.find({nr, nc}) == visited.end()) {
                    visited.insert({nr, nc});
                    q.push({nr, nc});
                }
            }
        }
    }
    return result;
}

void Board::loadFromPreset(const std::vector<std::vector<std::string>>& codes)
{

    int newSize = static_cast<int>(codes.size());
    if (newSize <= 0 || newSize > 30) {
        qDebug() << "loadFromPreset: invalid newSize =" << newSize;
        return;
    }

    qDebug() << "loadFromPreset: clearing old board";
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            delete grid[i][j];

    qDebug() << "newSize:" << newSize << "old size:" << size;
    if (newSize != size) {
        size = newSize;
        grid.resize(size, std::vector<Chess*>(size, nullptr));
    }

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            const std::string& code = codes[i][j];
            if (code.empty()) {
                grid[i][j] = nullptr;
                qDebug() << "set empty at" << i << j;
            } else {
                grid[i][j] = new Chess(i, j, code, 2);
                qDebug() << "created Chess" << code.c_str() << "at" << i << j;
            }
        }
    }
    qDebug() << "loadFromPreset finished";
}