#ifndef REPLAYWIDGET_H
#define REPLAYWIDGET_H

#include <QWidget>
#include <QVector>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include "gamecontroller.h"
#include "gamedata.h"

class ReplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReplayWidget(const SaveData& data, QWidget *parent = nullptr);
    ~ReplayWidget();

protected:
    void keyPressEvent(QKeyEvent * /*e*/) override {}

private slots:
    void onPrevStep();
    void onNextStep();
    void onStepChanged(int value);

private:
    void setupUI();
    void updateDisplay(int stepIndex);

    QVector<QVector<QLabel*>> cells;
    GameController* game;
    QStringList moveHistory;
    int currentStep;
    int totalSteps;
    QLabel* stepLabel;
    QPushButton* prevBtn;
    QPushButton* nextBtn;
    QLabel* scoreLabel;
    QSlider* stepSlider;
    QSpinBox* stepSpinBox;
    int boardSize;
    SaveData originalData;
};

#endif