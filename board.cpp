#include "board.h"
#include <random>


// 方向偏移数组
static const int dr[4] = {-1, 1, 0, 0};
static const int dc[4] = {0, 0, -1, 1};

Board::Board(int size) : size(size) {
    grid.resize(size, std::vector<Chess*>(size, nullptr));
    initRandomBoard();
}

Board::~Board() {
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            delete grid[i][j];
}


int Board::getSize() const {
    return size;
}

Chess* Board::getChess(int row, int col) const {
    if (row < 0 || row >= size || col < 0 || col >= size) return nullptr;
    return grid[row][col];
}

void Board::setChess(int row, int col, Chess* chess) {
    if (row >= 0 && row < size && col >= 0 && col < size)
        grid[row][col] = chess;
}

void Board::initRandomBoard() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> letterDist(0, 4);
    std::uniform_int_distribution<int> numDist(0, 9);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            char letter = 'A' + letterDist(gen);
            int num = numDist(gen);
            std::string code = std::string(1, letter) + std::to_string(num);
            grid[i][j] = new Chess(i, j, code, 2);
        }
    }
}

bool Board::canMerge(Chess* a, Chess* b) const {
    if (!a || !b) return false;
    if (a->code == b->code) return true;
    if (a->code[0] == b->code[0]) return true;
    if (a->code.substr(1) == b->code.substr(1)) return true;
    return false;
}


std::string Board::mergeCodes(const std::string& code1, const std::string& code2) {
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

// board.cpp 中的 moveSingleChess 替换为以下内容
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
    }
    else if (canMerge(chess, target)) {
        std::string newCode = mergeCodes(chess->code, target->code);
        chess->code = newCode;
        delete target;
        setChess(nr, nc, chess);
        setChess(chess->row, chess->col, nullptr);
        chess->row = nr;
        chess->col = nc;
        scoreGain += 1;
        return Merged;
    }
    else {
        return Blocked;
    }
}