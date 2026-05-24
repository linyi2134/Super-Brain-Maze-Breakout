#include "chess.h"


Chess::Chess(int row, int col, const std::string& code, int ownerId)
    : row(row), col(col), code(code), owner(ownerId), alive(true)
{
}

Chess::~Chess() = default;