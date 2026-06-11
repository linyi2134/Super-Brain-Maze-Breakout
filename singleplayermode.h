#ifndef SINGLEPLAYERMODE_H
#define SINGLEPLAYERMODE_H

#include <QWidget>

namespace Ui {
class SinglePlayerMode;
}

class SinglePlayerMode : public QWidget
{
    Q_OBJECT
public:
    explicit SinglePlayerMode(QWidget *parent = nullptr);
    ~SinglePlayerMode();

signals:
    void backToModeSelection();
    void startRandomGame();
    void startChallenge(int level);

private slots:
    void onSinglePlayerClicked();
    void onChallengeClicked();
    void onSelfBuiltClicked();
    void onBackClicked();

private:
    Ui::SinglePlayerMode *ui;
};

#endif