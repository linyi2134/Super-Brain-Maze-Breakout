#include "gamewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QTimer>
#include <QDialog>
#include <QTextEdit>
#include "savedialog.h"
#include <QDir>
#include <QFile>
#include <QDataStream>

GameWidget::GameWidget(const QString& playerName, bool aiMode,
                       const std::vector<std::vector<std::string>>* presetBoard,
                       int boardSize, bool fromSave, QWidget *parent)
    : QWidget(parent)
    , timer(nullptr)
    , elapsedSeconds(0)
    , isTiming(false)
    , statusLabel(nullptr)
    , timerLabel(nullptr)
    , scoreLabel(nullptr)
    , playerNameLabel(nullptr)
    , backButton(nullptr)
    , endButton(nullptr)
    , hintButton(nullptr)
    , historyButton(nullptr)
    , saveButton(nullptr)
    , hintActive(false)
    , cellSize(60)
    , aiMode(aiMode)
    , playerName(playerName)
    , initialized(false)
    , m_presetBoard(presetBoard)
    , boardSize(boardSize)
    , game(nullptr)
    , turnLabel(nullptr)
    , gameMode(0)
    , m_fromSave(fromSave)
{
    qDebug() << "GameWidget ctor start, boardSize=" << boardSize;
    setupUI();
    setWindowTitle(QString("密阵突围 - %1 %2").arg(playerName).arg(aiMode ? "vs AI" : "单人模式"));
    resize(cellSize * boardSize + 100, cellSize * boardSize + 150);
    setFocusPolicy(Qt::StrongFocus);

    if (aiMode) {
        gameMode = 1;
    } else if (presetBoard != nullptr) {
        gameMode = 2;
    } else {
        gameMode = 0;
    }

    if (!m_fromSave) {
        QTimer::singleShot(100, this, &GameWidget::initializeGame);
    } else {
        qDebug() << "GameWidget in load mode, waiting for loadFromSaveData";
    }
    qDebug() << "GameWidget ctor end";
}

GameWidget::~GameWidget()
{
    if (timer) timer->stop();
    delete game;
}

void GameWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!initialized) {
        initialized = true;
    }
}

void GameWidget::initializeGame()
{
    qDebug() << "initializeGame called";
    startNewGame();
}

void GameWidget::startNewGame()
{
    qDebug() << "startNewGame: enter";
    if (game) { delete game; game = nullptr; }
    qDebug() << "Creating GameController...";
    game = new GameController();
    qDebug() << "Calling game->init...";
    game->init(boardSize, playerName.toStdString(), aiMode, m_presetBoard);
    qDebug() << "game->init finished";
    game->clearMoveHistory();
    elapsedSeconds = 0;
    isTiming = false;
    timer->stop();
    updateBoardDisplay();
    onBoardUpdated();
    qDebug() << "startNewGame: exit";
}

void GameWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* infoLayout = new QHBoxLayout();
    playerNameLabel = new QLabel(playerName);
    scoreLabel = new QLabel("得分: 0");
    timerLabel = new QLabel("时间: 0 秒");
    statusLabel = new QLabel("请点击灰色棋子选择初始棋子");
    backButton = new QPushButton("返回菜单");
    endButton = new QPushButton("结束游戏");
    hintButton = new QPushButton("提示", this);
    historyButton = new QPushButton("步数记录", this);
    saveButton = new QPushButton("存档", this);
    turnLabel = new QLabel(this);
    infoLayout->addWidget(playerNameLabel);
    infoLayout->addWidget(scoreLabel);
    infoLayout->addWidget(timerLabel);
    infoLayout->addWidget(statusLabel);
    infoLayout->addWidget(backButton);
    infoLayout->addWidget(endButton);
    infoLayout->addWidget(hintButton);
    infoLayout->addWidget(historyButton);
    infoLayout->addWidget(saveButton);
    infoLayout->addWidget(turnLabel);
    mainLayout->addLayout(infoLayout);

    QWidget* boardContainer = new QWidget(this);
    QGridLayout* boardLayout = new QGridLayout(boardContainer);
    boardLayout->setSpacing(2);
    cells.resize(boardSize);
    for (int i = 0; i < boardSize; ++i) {
        cells[i].resize(boardSize);
        for (int j = 0; j < boardSize; ++j) {
            QLabel* cell = new QLabel(boardContainer);
            cell->setFixedSize(cellSize, cellSize);
            cell->setAlignment(Qt::AlignCenter);
            cell->setFrameShape(QFrame::Box);
            cell->setStyleSheet("background-color: white; font-size:16px;");
            boardLayout->addWidget(cell, i, j);
            cells[i][j] = cell;
        }
    }
    mainLayout->addWidget(boardContainer);
    setLayout(mainLayout);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWidget::updateTimer);
    connect(backButton, &QPushButton::clicked, this, &GameWidget::onBackToMenuClicked);
    connect(endButton, &QPushButton::clicked, this, &GameWidget::onEndGameClicked);
    connect(hintButton, &QPushButton::clicked, this, &GameWidget::onHintButtonClicked);
    connect(historyButton, &QPushButton::clicked, this, &GameWidget::onHistoryButtonClicked);
    connect(saveButton, &QPushButton::clicked, this, &GameWidget::onSaveButtonClicked);
}

void GameWidget::setGameMode(GameMode mode)
{
    gameMode = mode;
    if (game) game->setGameMode(mode);
    updateTurnDisplay();
}

void GameWidget::updateTurnDisplay()
{
    if (!game) return;
    if (gameMode == AIBattle) {
        turnLabel->setText(game->getCurrentPlayerName() + "回合");
    } else {
        turnLabel->setText("");
    }
}

void GameWidget::onBoardUpdated()
{
    updateBoardDisplay();
    if (!game->isInitialSelectionDone()) {
        int remain = game->getRemainingToSelect();
        statusLabel->setText(QString("还需选择 %1 个初始棋子").arg(remain));
        return;
    }

    if (!isTiming) {
        isTiming = true;
        timer->start(1000);
    }
    statusLabel->setText("使用方向键移动棋子");

    // 单人模式检查无合法移动
    if (gameMode == SingleRandom && !game->isGameOver() && !game->hasAnyLegalMove()) {
        timer->stop();
        saveToHistory();
        QMessageBox::information(this, "游戏结束", QString("无棋可走！\n最终得分: %1\n用时: %2秒")
                                                       .arg(game->getPlayer()->score).arg(elapsedSeconds));
        emit backToMenu();
        close();
        return;
    }

    if (game->getPlayer()) {
        scoreLabel->setText(QString("得分: %1").arg(game->getPlayer()->score));
    }
    updateTurnDisplay();
}

void GameWidget::updateBoardDisplay()
{
    if (!game) return;
    Board* board = game->getBoard();
    if (!board) return;
    int sz = board->getSize();
    if (sz != boardSize) return;
    for (int i = 0; i < sz; ++i) {
        for (int j = 0; j < sz; ++j) {
            Chess* ch = board->getChess(i, j);
            if (ch && ch->alive) {
                cells[i][j]->setText(QString::fromStdString(ch->code));
                bool isEatable = false;
                if (hintActive && game->isInitialSelectionDone() && !game->isGameOver()) {
                    isEatable = game->isEatableTarget(i, j);
                }
                if (isEatable) {
                    cells[i][j]->setStyleSheet("background-color: #ffff99; font-size:16px;");
                } else {
                    if (ch->owner == 0)
                        cells[i][j]->setStyleSheet("background-color: lightblue; font-size:16px;");
                    else if (ch->owner == 1)
                        cells[i][j]->setStyleSheet("background-color: #ff9999; font-size:16px;");
                    else
                        cells[i][j]->setStyleSheet("background-color: lightgray; font-size:16px;");
                }
            } else {
                cells[i][j]->setText("");
                cells[i][j]->setStyleSheet("background-color: white;");
            }
        }
    }
}

void GameWidget::mousePressEvent(QMouseEvent *e)
{
    if (!game || game->isInitialSelectionDone()) return;
    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            if (cells[i][j]->geometry().contains(e->pos())) {
                if (game->selectInitialChess(i, j)) {
                    updateBoardDisplay();
                    onBoardUpdated();
                } else {
                    statusLabel->setText("只能选择灰色中立棋子！");
                    QTimer::singleShot(1000, this, [this](){ onBoardUpdated(); });
                }
                return;
            }
        }
    }
}

void GameWidget::keyPressEvent(QKeyEvent *e)
{
    if (!game || !game->isInitialSelectionDone() || game->isGameOver()) return;

    Direction dir;
    if (gameMode == SingleRandom || gameMode == Challenge) {
        switch (e->key()) {
        case Qt::Key_Up: dir = Up; break;
        case Qt::Key_Down: dir = Down; break;
        case Qt::Key_Left: dir = Left; break;
        case Qt::Key_Right: dir = Right; break;
        default: return;
        }
        game->movePlayer(dir);
        updateBoardDisplay();
        onBoardUpdated();
        updateTurnDisplay();
        if (game->isGameOver()) {
            timer->stop();
            if (gameMode == AIBattle) {
                int playerScore = game->getPlayer()->score;
                int aiScore = game->getAIPlayer()->score;
                QString winner = (playerScore > aiScore) ? "玩家获胜" : (aiScore > playerScore) ? "AI获胜" : "平局";
                QMessageBox::information(this, "游戏结束", QString("玩家得分: %1\nAI得分: %2\n%3\n用时: %4秒")
                                                                   .arg(playerScore).arg(aiScore).arg(winner).arg(elapsedSeconds));
            } else {
                QMessageBox::information(this, "游戏结束", QString("最终得分: %1\n用时: %2秒")
                                                                   .arg(game->getPlayer()->score).arg(elapsedSeconds));
            }
            emit backToMenu();
            close();
            return;
        }
    }
    else if (gameMode == AIBattle) {
        if (game->getCurrentPlayer() != 0) return;
        switch (e->key()) {
        case Qt::Key_Up: dir = Up; break;
        case Qt::Key_Down: dir = Down; break;
        case Qt::Key_Left: dir = Left; break;
        case Qt::Key_Right: dir = Right; break;
        default: return;
        }
        game->movePlayer(dir);
        updateBoardDisplay();
        onBoardUpdated();
        updateTurnDisplay();
        if (game->isGameOver()) {
            timer->stop();
            int playerScore = game->getPlayer()->score;
            int aiScore = game->getAIPlayer()->score;
            QString winner = (playerScore > aiScore) ? "玩家获胜" : (aiScore > playerScore) ? "AI获胜" : "平局";
            QMessageBox::information(this, "游戏结束", QString("玩家得分: %1\nAI得分: %2\n%3\n用时: %4秒")
                                                               .arg(playerScore).arg(aiScore).arg(winner).arg(elapsedSeconds));
            emit backToMenu();
            close();
            return;
        }
        // 延迟执行AI移动
        if (!game->isGameOver() && game->getCurrentPlayer() == 1) {
            QTimer::singleShot(500, this, [this]() {
                if (game && game->getCurrentPlayer() == 1 && !game->isGameOver()) {
                    game->aiMove();
                    updateBoardDisplay();
                    onBoardUpdated();
                    updateTurnDisplay();
                    if (game->isGameOver()) {
                        timer->stop();
                        int playerScore = game->getPlayer()->score;
                        int aiScore = game->getAIPlayer()->score;
                        QString winner = (playerScore > aiScore) ? "玩家获胜" : (aiScore > playerScore) ? "AI获胜" : "平局";
                        QMessageBox::information(this, "游戏结束", QString("玩家得分: %1\nAI得分: %2\n%3\n用时: %4秒")
                                                                           .arg(playerScore).arg(aiScore).arg(winner).arg(elapsedSeconds));
                        emit backToMenu();
                        close();
                    }
                }
            });
        }
    }
}

void GameWidget::onBackToMenuClicked()
{
    timer->stop();
    emit backToMenu();
    close();
}

void GameWidget::onEndGameClicked()
{
    if (!game || !game->isInitialSelectionDone() || game->isGameOver()) return;
    timer->stop();
    saveToHistory();
    if (gameMode == AIBattle) {
        int playerScore = game->getPlayer()->score;
        int aiScore = game->getAIPlayer()->score;
        QString winner = (playerScore > aiScore) ? "玩家获胜" : (aiScore > playerScore) ? "AI获胜" : "平局";
        QMessageBox::information(this, "游戏结束", QString("你主动结束了游戏\n玩家得分: %1\nAI得分: %2\n%3\n用时: %4秒")
                                                       .arg(playerScore).arg(aiScore).arg(winner).arg(elapsedSeconds));
    } else {
        QMessageBox::information(this, "游戏结束", QString("你主动结束了游戏\n最终得分: %1\n用时: %2秒")
                                                       .arg(game->getPlayer()->score).arg(elapsedSeconds));
    }
    emit backToMenu();
    close();
}

void GameWidget::updateTimer()
{
    if (isTiming) {
        elapsedSeconds++;
        timerLabel->setText(QString("时间: %1 秒").arg(elapsedSeconds));
    }
}

void GameWidget::closeEvent(QCloseEvent *event)
{
    emit backToMenu();
    event->accept();
}

void GameWidget::onHintButtonClicked()
{
    if (!game || !game->isInitialSelectionDone() || game->isGameOver()) return;
    hintActive = !hintActive;
    updateBoardDisplay();
    hintButton->setText(hintActive ? "隐藏提示" : "提示");
}

void GameWidget::resizeBoard(int newCellSize)
{
    if (newCellSize < 30) newCellSize = 30;
    if (newCellSize > 120) newCellSize = 120;
    cellSize = newCellSize;
    int sz = boardSize;
    for (int i = 0; i < sz; ++i) {
        for (int j = 0; j < sz; ++j) {
            cells[i][j]->setFixedSize(cellSize, cellSize);
            QFont font = cells[i][j]->font();
            font.setPointSize(cellSize / 4);
            cells[i][j]->setFont(font);
        }
    }
    resize(cellSize * boardSize + 100, cellSize * boardSize + 150);
}

void GameWidget::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        resizeBoard(cellSize + 5);
    } else {
        resizeBoard(cellSize - 5);
    }
    event->accept();
}

void GameWidget::onHistoryButtonClicked()
{
    if (!game) return;
    QStringList history = game->getMoveHistory();
    if (history.isEmpty()) {
        QMessageBox::information(this, "步数记录", "暂无任何步数记录");
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle("游戏步数记录");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QTextEdit *textEdit = new QTextEdit(&dialog);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(history.join("\n"));
    layout->addWidget(textEdit);
    QPushButton *closeBtn = new QPushButton("关闭");
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.resize(400, 300);
    dialog.exec();
}

void GameWidget::onSaveButtonClicked()
{
    if (!game || !game->isInitialSelectionDone() || game->isGameOver()) {
        QMessageBox::warning(this, "存档失败", "游戏未开始或已结束，无法存档");
        return;
    }
    saveGame();
}

void GameWidget::saveGame()
{
    if (!game) return;
    SaveDialog dlg(this, false);
    if (dlg.exec() == QDialog::Accepted) {
        int slot = dlg.getSelectedSlot();
        if (slot < 1 || slot > 3) return;
        QDir dir;
        if (!dir.exists("saves")) dir.mkdir("saves");
        QString fileName = QString("saves/slot%1.dat").arg(slot);
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, "存档失败", "无法写入文件");
            return;
        }
        SaveData data = game->toSaveData();
        data.mode = gameMode;
        data.elapsedSeconds = elapsedSeconds;
        data.isTiming = isTiming;
        data.currentTurn = game->getCurrentTurn();
        QDataStream out(&file);
        out << data;
        file.close();
        QMessageBox::information(this, "存档成功", QString("已保存到存档位%1").arg(slot));
    }
}

void GameWidget::loadFromSaveData(const SaveData& data)
{
    if (data.boardSize != boardSize) {
        QMessageBox::warning(this, "读档失败",
                             QString("存档棋盘大小为 %1x%1，当前窗口为 %2x%2，无法读取。")
                                 .arg(data.boardSize).arg(boardSize));
        return;
    }
    if (timer && timer->isActive()) timer->stop();
    if (game) delete game;
    game = new GameController();
    game->fromSaveData(data);
    elapsedSeconds = data.elapsedSeconds;
    isTiming = data.isTiming;
    if (isTiming) timer->start(1000);
    else timer->stop();
    updateBoardDisplay();
    onBoardUpdated();
    setWindowTitle(QString("密阵突围 - %1 %2")
                       .arg(playerName)
                       .arg(game->getAIPlayer() ? "vs AI" : "单人模式"));
}

void GameWidget::saveToHistory()
{
    if (!game) return;
    SaveData data = game->getInitialSaveData();
    data.mode = gameMode;
    data.playerScore = game->getPlayer()->score;
    data.elapsedSeconds = elapsedSeconds;
    data.isTiming = false;
    data.fixed = false;
    data.moveHistory = game->getMoveHistory();

    qDebug() << "Saving history - score:" << data.playerScore;

    QVector<SaveData> allRecords;
    QFile file("history.dat");
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        while (!in.atEnd()) {
            SaveData d;
            in >> d;
            allRecords.push_back(d);
        }
        file.close();
    }

    QVector<SaveData> fixedRecords;
    QVector<SaveData> normalRecords;
    for (const SaveData& d : allRecords) {
        if (d.fixed) fixedRecords.push_back(d);
        else normalRecords.push_back(d);
    }

    normalRecords.push_back(data);
    if (normalRecords.size() > 10) {
        normalRecords.erase(normalRecords.begin(), normalRecords.begin() + (normalRecords.size() - 10));
    }

    QVector<SaveData> newAll = fixedRecords;
    newAll.append(normalRecords);

    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        for (const SaveData& d : newAll) {
            out << d;
        }
        file.close();
    }
}