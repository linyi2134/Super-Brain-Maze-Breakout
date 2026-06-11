#include "singleplayermode.h"
#include "ui_singleplayermode.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include "customboardeditor.h"
#include "customboardselector.h"
#include "gamewidget.h"

SinglePlayerMode::SinglePlayerMode(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SinglePlayerMode)
{
    ui->setupUi(this);
    setWindowTitle("单人模式");

    connect(ui->SinglePlayerButton, &QPushButton::clicked, this, &SinglePlayerMode::onSinglePlayerClicked);
    connect(ui->ChallengeModeButton, &QPushButton::clicked, this, &SinglePlayerMode::onChallengeClicked);
    connect(ui->SelfcreatedModeButton, &QPushButton::clicked, this, &SinglePlayerMode::onSelfBuiltClicked);
    connect(ui->BackButton, &QPushButton::clicked, this, &SinglePlayerMode::onBackClicked);
}

SinglePlayerMode::~SinglePlayerMode()
{
    delete ui;
}

void SinglePlayerMode::onSinglePlayerClicked()
{
    emit startRandomGame();
    hide();
}

void SinglePlayerMode::onChallengeClicked()
{
    QStringList items = {"简单", "普通", "困难", "专家", "噩梦"};
    bool ok;
    QString selected = QInputDialog::getItem(this, "选择难度", "难度等级:", items, 0, false, &ok);
    if (ok && !selected.isEmpty()) {
        int level = items.indexOf(selected);
        emit startChallenge(level);
        hide();
    }
}

void SinglePlayerMode::onSelfBuiltClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("自建题库");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QPushButton* createBtn = new QPushButton("创建新棋盘");
    QPushButton* playBtn = new QPushButton("游玩已有棋盘");
    QPushButton* cancelBtn = new QPushButton("取消");
    layout->addWidget(createBtn);
    layout->addWidget(playBtn);
    layout->addWidget(cancelBtn);

    connect(createBtn, &QPushButton::clicked, [&]() {
        dialog.accept();
        CustomBoardEditor* editor = new CustomBoardEditor();
        editor->setAttribute(Qt::WA_DeleteOnClose);
        editor->show();
    });

    connect(playBtn, &QPushButton::clicked, [&]() {
        dialog.accept();
        CustomBoardSelector* selector = new CustomBoardSelector();
        selector->setAttribute(Qt::WA_DeleteOnClose);
        QObject::connect(selector, &CustomBoardSelector::startGame,
                         [this](const std::vector<std::vector<std::string>>& board, int boardSize) {
                             // 拷贝一份数据到堆上，避免局部变量销毁
                             auto* boardCopy = new std::vector<std::vector<std::string>>(board);
                             GameWidget* game = new GameWidget("玩家", false, boardCopy, boardSize);
                             game->setAttribute(Qt::WA_DeleteOnClose);
                             // 游戏窗口关闭时，删除拷贝的数据
                             QObject::connect(game, &QWidget::destroyed, [boardCopy]() { delete boardCopy; });
                             game->show();
                         });
        selector->show();
    });

    // 补上取消按钮的连接
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}


void SinglePlayerMode::onBackClicked()
{
    emit backToModeSelection();
    hide();
}