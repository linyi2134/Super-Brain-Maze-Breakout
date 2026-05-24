#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QTimer>
#include "gamecontroller.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

private slots:
    void onNewGame();
    void onExit();
    void updateTimer();

private:
    void setupUI();
    void updateBoardDisplay();
    void startNewGame();
    void updateStatusMessage();

    QVector<QVector<QLabel*>> cells;
    GameController* game;
    int boardSize;
    QTimer* timer;
    int elapsedSeconds;
    bool isTiming;
    QLabel* statusLabel;
    QLabel* timerLabel;
    QLabel* scoreLabel;
};

#endif