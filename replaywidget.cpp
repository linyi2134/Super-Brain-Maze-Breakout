#include "replaywidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QMessageBox>
#include <QDebug>

ReplayWidget::ReplayWidget(const SaveData& data, QWidget *parent)
    : QWidget(parent), game(nullptr), currentStep(0), totalSteps(0), originalData(data)
{
    boardSize = data.boardSize;
    game = new GameController();
    game->fromSaveData(data);
    moveHistory = data.moveHistory;
    totalSteps = moveHistory.size();

    setupUI();

    stepSlider->setRange(0, totalSteps);
    stepSpinBox->setRange(0, totalSteps);
    updateDisplay(0);
    stepSlider->setValue(0);
    stepSpinBox->setValue(0);

    setWindowTitle("对局回放 - 第0步");
    resize(boardSize * 60 + 100, boardSize * 60 + 180);
}

ReplayWidget::~ReplayWidget()
{
    delete game;
}

void ReplayWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* infoLayout = new QHBoxLayout();
    scoreLabel = new QLabel("得分: 0");
    stepLabel = new QLabel("步骤: 0 / 0");
    infoLayout->addWidget(scoreLabel);
    infoLayout->addWidget(stepLabel);
    mainLayout->addLayout(infoLayout);

    QWidget* boardContainer = new QWidget(this);
    QGridLayout* boardLayout = new QGridLayout(boardContainer);
    boardLayout->setSpacing(2);
    cells.resize(boardSize);
    for (int i = 0; i < boardSize; ++i) {
        cells[i].resize(boardSize);
        for (int j = 0; j < boardSize; ++j) {
            QLabel* cell = new QLabel(boardContainer);
            cell->setFixedSize(60, 60);
            cell->setAlignment(Qt::AlignCenter);
            cell->setFrameShape(QFrame::Box);
            cell->setStyleSheet("background-color: white; font-size:16px;");
            boardLayout->addWidget(cell, i, j);
            cells[i][j] = cell;
        }
    }
    mainLayout->addWidget(boardContainer);

    QHBoxLayout* sliderLayout = new QHBoxLayout();
    QLabel* sliderLabel = new QLabel("跳转步数:");
    stepSlider = new QSlider(Qt::Horizontal);
    stepSlider->setRange(0, totalSteps);
    stepSlider->setValue(0);
    stepSpinBox = new QSpinBox();
    stepSpinBox->setRange(0, totalSteps);
    stepSpinBox->setValue(0);
    sliderLayout->addWidget(sliderLabel);
    sliderLayout->addWidget(stepSlider);
    sliderLayout->addWidget(stepSpinBox);
    mainLayout->addLayout(sliderLayout);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    prevBtn = new QPushButton("上一步");
    nextBtn = new QPushButton("下一步");
    QPushButton* closeBtn = new QPushButton("关闭");
    btnLayout->addWidget(prevBtn);
    btnLayout->addWidget(nextBtn);
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);

    connect(prevBtn, &QPushButton::clicked, this, &ReplayWidget::onPrevStep);
    connect(nextBtn, &QPushButton::clicked, this, &ReplayWidget::onNextStep);
    connect(closeBtn, &QPushButton::clicked, this, &ReplayWidget::close);
    connect(stepSlider, &QSlider::valueChanged, this, &ReplayWidget::onStepChanged);
    connect(stepSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ReplayWidget::onStepChanged);

    setLayout(mainLayout);
}

void ReplayWidget::updateDisplay(int stepIndex)
{
    game->fromSaveData(originalData);
    for (int i = 0; i < stepIndex; ++i) {
        QString stepStr = moveHistory[i];
        Direction dir;
        bool isPlayer = stepStr.startsWith("玩家");
        // 如果是跳过步数，不执行移动
        if (stepStr.contains("跳过")) {
            continue;
        }
        if (stepStr.contains("上")) dir = Up;
        else if (stepStr.contains("下")) dir = Down;
        else if (stepStr.contains("左")) dir = Left;
        else if (stepStr.contains("右")) dir = Right;
        else continue;
        if (isPlayer) {
            game->movePlayer(dir);
        } else {
            game->aiMove();
        }
    }
    Board* board = game->getBoard();
    int sz = board->getSize();
    for (int i = 0; i < sz; ++i) {
        for (int j = 0; j < sz; ++j) {
            Chess* ch = board->getChess(i, j);
            if (ch && ch->alive) {
                cells[i][j]->setText(QString::fromStdString(ch->code));
                if (ch->owner == 0)
                    cells[i][j]->setStyleSheet("background-color: lightblue; font-size:16px;");
                else if (ch->owner == 1)
                    cells[i][j]->setStyleSheet("background-color: #ff9999; font-size:16px;");
                else
                    cells[i][j]->setStyleSheet("background-color: lightgray; font-size:16px;");
            } else {
                cells[i][j]->setText("");
                cells[i][j]->setStyleSheet("background-color: white;");
            }
        }
    }
    scoreLabel->setText(QString("得分: %1").arg(game->getPlayer()->score));
    stepLabel->setText(QString("步骤: %1 / %2").arg(stepIndex).arg(totalSteps));
    stepSlider->blockSignals(true);
    stepSpinBox->blockSignals(true);
    stepSlider->setValue(stepIndex);
    stepSpinBox->setValue(stepIndex);
    stepSlider->blockSignals(false);
    stepSpinBox->blockSignals(false);
}

void ReplayWidget::onPrevStep()
{
    if (currentStep > 0) {
        currentStep--;
        updateDisplay(currentStep);
    }
}

void ReplayWidget::onNextStep()
{
    if (currentStep < totalSteps) {
        currentStep++;
        updateDisplay(currentStep);
    }
}

void ReplayWidget::onStepChanged(int value)
{
    if (value >= 0 && value <= totalSteps && value != currentStep) {
        currentStep = value;
        updateDisplay(currentStep);
    }
}