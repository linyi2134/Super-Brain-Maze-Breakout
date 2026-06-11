#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include "gamecontroller.h"

class GameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameWidget(const QString& playerName, bool aiMode = false,
                        const std::vector<std::vector<std::string>>* presetBoard = nullptr,
                        int boardSize = 10, bool fromSave = false, QWidget *parent = nullptr);
    ~GameWidget();

    void loadFromSaveData(const SaveData& data);
    void setGameMode(GameMode mode);

signals:
    void backToMenu();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void updateTimer();
    void onBoardUpdated();
    void onBackToMenuClicked();
    void onEndGameClicked();
    void initializeGame();
    void onHintButtonClicked();
    void onHistoryButtonClicked();
    void onSaveButtonClicked();

private:
    void setupUI();
    void updateBoardDisplay();
    void startNewGame();
    void resizeBoard(int newCellSize);
    void saveToHistory();
    void saveGame();
    void updateTurnDisplay();

    QVector<QVector<QLabel*>> cells;
    GameController* game;
    QTimer* timer;
    int elapsedSeconds;
    bool isTiming;
    QLabel* statusLabel;
    QLabel* timerLabel;
    QLabel* scoreLabel;
    QLabel* playerNameLabel;
    QLabel* turnLabel;
    QPushButton* backButton;
    QPushButton* endButton;
    QPushButton* hintButton;
    QPushButton* historyButton;
    QPushButton* saveButton;
    bool hintActive;
    int cellSize;
    bool aiMode;
    QString playerName;
    bool initialized;
    const std::vector<std::vector<std::string>>* m_presetBoard;
    int boardSize;
    int gameMode;     // 0单人随机 1AI 2挑战（去掉多人）
    bool m_fromSave;
};

#endif