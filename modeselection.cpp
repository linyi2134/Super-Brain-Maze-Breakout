#include "modeselection.h"
#include "ui_modeselection.h"
#include <QMessageBox>

ModeSelection::ModeSelection(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModeSelection)
{
    ui->setupUi(this);
    setWindowTitle("选择游戏模式");

    connect(ui->OneplayerModeButton, &QPushButton::clicked, this, &ModeSelection::onSinglePlayerClicked);
    connect(ui->AIModeButton, &QPushButton::clicked, this, &ModeSelection::onAIClicked);
    connect(ui->MultiplayerButton, &QPushButton::clicked, this, &ModeSelection::onMultiplayerClicked);
    connect(ui->BackButton, &QPushButton::clicked, this, &ModeSelection::onBackClicked);
}

ModeSelection::~ModeSelection()
{
    delete ui;
}

void ModeSelection::onSinglePlayerClicked()
{
    emit singlePlayerSelected();
    hide();
}

void ModeSelection::onAIClicked()
{
    emit aiBattleSelected();
    hide();
}

void ModeSelection::onMultiplayerClicked()
{
    QMessageBox::information(this, "提示", "多人模式开发中，敬请期待！");
}

void ModeSelection::onBackClicked()
{
    emit backToMenu();
    hide();
}