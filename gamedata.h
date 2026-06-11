#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <QDataStream>
#include <QString>
#include <vector>
#include <string>

struct ChessData {
    std::string code;
    int owner;
    bool alive;
    ChessData() : code("A1"), owner(2), alive(true) {}
    ChessData(const std::string& c, int o, bool a) : code(c), owner(o), alive(a) {}
};

struct SaveData {
    int version = 1;
    int mode;
    int boardSize;
    QString playerName;
    int playerScore;
    int playerRemainingChess;
    bool hasAI;
    QString aiName;
    int aiScore;
    int aiRemainingChess;
    bool selectionDone;
    int selectedCount;
    std::vector<std::vector<ChessData>> board;
    int elapsedSeconds;
    bool isTiming;
    int currentTurn;
    QStringList moveHistory;
    int neededCount;
    bool fixed = false;
};

inline QDataStream& operator<<(QDataStream& out, const ChessData& data) {
    out << QString::fromStdString(data.code) << data.owner << data.alive;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, ChessData& data) {
    QString code;
    in >> code >> data.owner >> data.alive;
    data.code = code.toStdString();
    return in;
}

inline QDataStream& operator<<(QDataStream& out, const SaveData& data) {
    out << data.version << data.mode << data.boardSize << data.playerName
        << data.playerScore << data.playerRemainingChess << data.hasAI
        << data.aiName << data.aiScore << data.aiRemainingChess
        << data.selectionDone << data.selectedCount << data.neededCount;

    int size = data.board.size();
    out << size;
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            out << data.board[i][j];
    out << data.elapsedSeconds << data.isTiming << data.currentTurn << data.moveHistory
        << data.fixed;
    return out;
}

inline QDataStream& operator>>(QDataStream& in, SaveData& data) {
    in >> data.version >> data.mode >> data.boardSize >> data.playerName
        >> data.playerScore >> data.playerRemainingChess >> data.hasAI
        >> data.aiName >> data.aiScore >> data.aiRemainingChess
        >> data.selectionDone >> data.selectedCount >> data.neededCount;

    int size;
    in >> size;
    data.board.resize(size, std::vector<ChessData>(size));
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            in >> data.board[i][j];
    in >> data.elapsedSeconds >> data.isTiming >> data.currentTurn >> data.moveHistory
        >> data.fixed;
    return in;
}

#endif // GAMEDATA_H