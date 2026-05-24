#include "widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QMouseEvent>
#include <string>

Widget::Widget(QWidget *parent)
    : QWidget(parent), game(nullptr), boardSize(10),
    timer(nullptr), elapsedSeconds(0), isTiming(false)
{
    setupUI();
    startNewGame();
    setWindowTitle("密阵突围 - 单人模式");
    resize(boardSize * 60 + 20, boardSize * 60 + 120);
}

Widget::~Widget()
{
    delete game;
    if (timer) timer->stop();
}

void Widget::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ----- 菜单栏 -----
    QMenuBar* menuBar = new QMenuBar(this);
    QMenu* gameMenu = menuBar->addMenu("游戏");
    QAction* newGameAction = new QAction("新游戏", this);
    QAction* exitAction = new QAction("退出", this);
    gameMenu->addAction(newGameAction);
    gameMenu->addSeparator();
    gameMenu->addAction(exitAction);
    connect(newGameAction, &QAction::triggered, this, &Widget::onNewGame);
    connect(exitAction, &QAction::triggered, this, &Widget::onExit);
    mainLayout->setMenuBar(menuBar);

    // ----- 信息栏（水平布局）-----
    QHBoxLayout* infoLayout = new QHBoxLayout();
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    infoLayout->addWidget(statusLabel);

    scoreLabel = new QLabel("得分: 0", this);
    scoreLabel->setStyleSheet("font-size: 14px;");
    infoLayout->addWidget(scoreLabel);

    timerLabel = new QLabel("时间: 0 秒", this);
    timerLabel->setStyleSheet("font-size: 14px;");
    infoLayout->addWidget(timerLabel);

    infoLayout->addStretch();
    mainLayout->addLayout(infoLayout);

    // ----- 棋盘网格 -----
    QGridLayout* boardLayout = new QGridLayout();
    boardLayout->setSpacing(2);
    cells.resize(boardSize);
    for (int i = 0; i < boardSize; ++i) {
        cells[i].resize(boardSize);
        for (int j = 0; j < boardSize; ++j) {
            QLabel* cell = new QLabel(this);
            cell->setFixedSize(60, 60);
            cell->setAlignment(Qt::AlignCenter);
            cell->setFrameShape(QFrame::Box);
            cell->setStyleSheet("background-color: white; font-size: 16px; font-weight: bold;");
            boardLayout->addWidget(cell, i, j);
            cells[i][j] = cell;
        }
    }
    mainLayout->addLayout(boardLayout);

    setLayout(mainLayout);

    // 计时器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::updateTimer);
}

void Widget::startNewGame()
{
    if (game) delete game;
    game = new GameController(boardSize, "玩家");
    elapsedSeconds = 0;
    isTiming = false;
    if (timer) timer->stop();
    updateTimer();
    updateBoardDisplay();
    updateStatusMessage();

}

void Widget::updateStatusMessage()
{
    if (!game) return;
    if (!game->isInitialSelectionDone()) {
        int remain = game->getRemainingToSelect();
        statusLabel->setText(QString("请点击棋盘上的灰色棋子选择初始棋子 (还需选择 %1 个)").arg(remain));
        statusLabel->setStyleSheet("color: blue; font-weight: bold;");
    } else {
        statusLabel->setText("使用方向键移动棋子");
        statusLabel->setStyleSheet("color: green; font-weight: bold;");
    }
}

void Widget::updateBoardDisplay()
{
    if (!game) return;
    Board* board = game->getBoard();
    int size = board->getSize();
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            Chess* ch = board->getChess(i, j);
            if (ch && ch->alive) {
                cells[i][j]->setText(QString::fromStdString(ch->code));
                if (ch->owner == 0) {
                    cells[i][j]->setStyleSheet("background-color: lightblue; font-size: 16px; font-weight: bold;");
                } else {
                    cells[i][j]->setStyleSheet("background-color: lightgray; font-size: 16px; font-weight: bold;");
                }
            } else {
                cells[i][j]->setText("");
                cells[i][j]->setStyleSheet("background-color: white;");
            }
        }
    }
    // 更新得分显示
    if (game->getPlayer()) {
        scoreLabel->setText(QString("得分: %1").arg(game->getPlayer()->score));
    }
}

void Widget::mousePressEvent(QMouseEvent *e)
{
    if (!game) return;

    if (!game->isInitialSelectionDone()) {

        QPoint pos = e->pos();

        for (int i = 0; i < boardSize; ++i) {
            for (int j = 0; j < boardSize; ++j) {
                QLabel* cell = cells[i][j];
                QRect cellRect = cell->geometry();
                if (cellRect.contains(pos)) {
                    if (game->selectInitialChess(i, j)) {
                        updateBoardDisplay();
                        updateStatusMessage();

                        if (game->isInitialSelectionDone()) {
                            elapsedSeconds = 0;
                            isTiming = true;
                            timer->start(1000);
                            updateTimer();
                        }
                        return;
                    } else {

                        statusLabel->setText("只能选择灰色的中立棋子！");
                        QTimer::singleShot(1000, this, [this](){ updateStatusMessage(); });
                    }
                }
            }
        }
    }
}

void Widget::keyPressEvent(QKeyEvent *e)
{
    if (!game) return;
    if (!game->isInitialSelectionDone()) {

        return;
    }

    Direction dir;
    switch (e->key()) {
    case Qt::Key_Up: dir = Up; break;
    case Qt::Key_Down: dir = Down; break;
    case Qt::Key_Left: dir = Left; break;
    case Qt::Key_Right: dir = Right; break;
    default: return;
    }

    game->movePlayer(dir);
    updateBoardDisplay();


    if (game->getPlayer()->remainingChess == 0) {
        timer->stop();
        QMessageBox::information(this, "游戏结束",
                                 QString("游戏结束！\n最终得分: %1\n用时: %2 秒")
                                     .arg(game->getPlayer()->score)
                                     .arg(elapsedSeconds));
        startNewGame();
    }
}

void Widget::updateTimer()
{
    if (isTiming) {
        elapsedSeconds++;
        timerLabel->setText(QString("时间: %1 秒").arg(elapsedSeconds));
    } else {
        timerLabel->setText("时间: 0 秒");
    }
}

void Widget::onNewGame()
{
    if (timer) timer->stop();
    startNewGame();
}

void Widget::onExit()
{
    close();
}