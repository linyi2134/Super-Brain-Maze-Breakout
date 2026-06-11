#include "menu.h"
#include "ui_menu.h"
#include <QMessageBox>
#include <QFile>
#include <QDataStream>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include "gamedata.h"
#include "savedialog.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTextOption>
#include "replaywidget.h"

Menu::Menu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Menu)
{
    ui->setupUi(this);
    setWindowTitle("密阵突围 - 主菜单");
    resize(500, 400);

    connect(ui->NewgameButton, &QPushButton::clicked, this, &Menu::onNewGameButtonClicked);
    connect(ui->RulesButton, &QPushButton::clicked, this, &Menu::onRulesButtonClicked);
    connect(ui->ExitButton, &QPushButton::clicked, this, &Menu::onExitButtonClicked);
    connect(ui->ArchiveButton, &QPushButton::clicked, this, &Menu::onArchiveButtonClicked);
    connect(ui->HistoryButton, &QPushButton::clicked, this, &Menu::onHistoryButtonClicked);
}

Menu::~Menu()
{
    delete ui;
}

void Menu::onNewGameButtonClicked()
{
    emit newGameRequested();
    hide();
}

void Menu::onRulesButtonClicked()
{
    QString rulesText =
        "【密阵突围】游戏规则及功能说明\n\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "一、游戏目标\n"
        "   通过移动己方棋子，与棋盘上的中立或敌方棋子合并，\n"
        "   不断壮大己方棋子并获取分数。当一方棋子被全部消灭\n"
        "   或主动结束时，游戏终止，得分高者获胜。\n\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "二、基本规则\n"
        "   1. 棋盘大小：5×5 / 10×10 / 15×15 / 20×20 / 25×25\n"
        "   2. 每个棋子由「字母(A~E) + 数字(0~9)」组成，例如 A3、D7。\n"
        "   3. 游戏开始时，棋盘上所有棋子均为中立（灰色）。\n"
        "   4. 玩家需点击选中若干中立棋子作为己方初始棋子（数量随模式变化）。\n"
        "   5. 使用键盘方向键（↑ ↓ ← →）移动所有己方棋子，\n"
        "      每回合只能选择一个方向，所有己方棋子同时向该方向移动一格。\n"
        "   6. 移动时如果遇到空格，棋子会继续沿同方向连续移动，直到遇到非空格或边界。\n"
        "   7. 如果与中立或敌方棋子相遇，且满足合并条件，则自动合并并立即结束本回合。\n\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "三、合并规则\n"
        "   • 相同字母 + 不同数字 → 字母不变，数字=(原两数之和) mod 10\n"
        "       例：A3 + A5 → A8\n"
        "   • 相同数字 + 不同字母 → 数字不变，字母=(原两字母值之和) mod 5\n"
        "       字母值：A=1, B=2, C=3, D=4, E=5，余0则变为E(5)\n"
        "       例：B3 + D3 → (2+4=6 mod 5=1 → A) → A3\n"
        "   • 完全相同代码 → 保持不变\n"
        "   • 合并时如果目标棋子属于敌方，则视为「吃子」，额外增加1分。\n"
        "   • 单回合内每吃掉一个敌方棋子，得分+1；若本回合吃子数≥3，奖励+5分；\n"
        "       吃子数≥5，再奖励+10分。\n\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "四、游戏模式\n"
        "   1. 单人随机：随机生成棋盘，玩家独自挑战中立棋子。\n"
        "   2. AI对决：玩家与电脑轮流操作，AI会优先选择能吃子的方向。\n"
        "   3. 挑战模式：内置5种难度（简单～噩梦），棋盘固定。\n"
        "   4. 自建题库：玩家可自由设计棋盘（大小5~25），保存后游玩。\n\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "五、初始棋子数量（重要！）\n"
        "   【单人随机 / 挑战模式 / 自建题库】\n"
        "       5×5   → 选择 1 个初始棋子\n"
        "       10×10 → 选择 2 个初始棋子\n"
        "       15×15 → 选择 4 个初始棋子\n"
        "       20×20 → 选择 6 个初始棋子\n"
        "       25×25 → 选择 8 个初始棋子\n"
        "   【AI对决（人机对战）】\n"
        "       5×5   → 双方各 1 个棋子\n"
        "       10×10 → 双方各 2 个棋子\n"
        "       15×15 → 双方各 3 个棋子\n"
        "       20×20 → 双方各 4 个棋子\n"
        "       25×25 → 双方各 5 个棋子\n"
        "   💡 自建题库保存时要求棋子数量更多（如10x10至少3个），\n"
        "      以确保棋盘上有足够的中立棋子可供合并得分。\n\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "六、功能模块说明\n"
        "   【提示】 开启后，可被当前玩家吃掉的敌方棋子高亮为黄色。\n"
        "   【步数记录】 显示本局每一步操作（移动/合并/跳过）。\n"
        "   【存档】 将游戏进度保存到本地（共3个存档位），支持任意模式。\n"
        "   【读档】 从主菜单「存档」按钮进入，可读取存档继续游戏。\n"
        "   【历史记录】 记录每局结果（模式、得分、用时、棋盘大小），\n"
        "       支持固定置顶、回放对局、删除记录。\n"
        "   【回放】 从历史记录中点击「回放」，逐步回顾整局游戏。\n"
        "   【结束游戏】 手动结束对局，结果自动保存到历史记录。\n"
        "   【返回菜单】 不保存结果直接退出。\n\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "七、小技巧\n"
        "   • 合并后本回合立即结束，不会连续合并。\n"
        "   • 尽量保留多个己方棋子，避免被对方吃光。\n"
        "   • 利用空格长距离移动，寻找更佳合并位置。\n"
        "   • 挑战模式不同难度的棋盘布局经过特殊设计。\n\n"
        "祝您游戏愉快！";

    // 使用 QDialog 加 QTextEdit 实现滚动
    QDialog dialog(this);
    dialog.setWindowTitle("游戏规则及功能说明");
    dialog.resize(600, 500);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QTextEdit *textEdit = new QTextEdit(&dialog);
    textEdit->setPlainText(rulesText);
    textEdit->setReadOnly(true);
    textEdit->setWordWrapMode(QTextOption::WordWrap);
    layout->addWidget(textEdit);

    QPushButton *closeBtn = new QPushButton("关闭", &dialog);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

void Menu::onExitButtonClicked()
{
    emit exitRequested();
}

void Menu::onArchiveButtonClicked()
{
    SaveDialog dlg(this, true);  // true 表示读档模式
    if (dlg.exec() == QDialog::Accepted) {
        int slot = dlg.getSelectedSlot();
        if (slot >= 1 && slot <= 3) {
            emit loadArchive(slot);
            hide();
        }
    }
}



void Menu::onHistoryButtonClicked()
{
    QFile file("history.dat");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "历史记录", "暂无历史记录");
        return;
    }
    QDataStream in(&file);
    QVector<SaveData> historyList;
    while (!in.atEnd()) {
        SaveData data;
        in >> data;
        historyList.push_back(data);
    }
    file.close();

    if (historyList.isEmpty()) {
        QMessageBox::information(this, "历史记录", "暂无历史记录");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("历史记录");
    dialog.resize(750, 450);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QTableWidget* table = new QTableWidget(historyList.size(), 6, &dialog);
    QStringList headers = {"模式", "玩家得分", "AI得分", "用时(秒)", "棋盘", "操作"};
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto saveAll = [&]() {
        QFile outFile("history.dat");
        if (outFile.open(QIODevice::WriteOnly)) {
            QDataStream out(&outFile);
            for (const SaveData& d : historyList) {
                out << d;
            }
            outFile.close();
        }
    };

    auto updateFixedButton = [](QPushButton* btn, bool fixed) {
        btn->setText(fixed ? "取消固定" : "固定");
    };

    for (int i = 0; i < historyList.size(); ++i) {
        SaveData& d = historyList[i];
        QString modeStr;
        if (d.mode == 0) modeStr = "单人随机";
        else if (d.mode == 1) modeStr = "AI对决";
        else modeStr = "挑战模式";

        int playerScore = d.playerScore;
        QString aiScoreStr = (d.hasAI) ? QString::number(d.aiScore) : "—";

        table->setItem(i, 0, new QTableWidgetItem(modeStr));
        table->setItem(i, 1, new QTableWidgetItem(QString::number(playerScore)));
        table->setItem(i, 2, new QTableWidgetItem(aiScoreStr));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(d.elapsedSeconds)));
        table->setItem(i, 4, new QTableWidgetItem(QString("%1x%2").arg(d.boardSize).arg(d.boardSize)));

        QWidget* btnWidget = new QWidget();
        QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
        btnLayout->setContentsMargins(0, 0, 0, 0);

        QPushButton* fixedBtn = new QPushButton(d.fixed ? "取消固定" : "固定");
        fixedBtn->setFixedSize(60, 25);
        QPushButton* replayBtn = new QPushButton("回放");
        replayBtn->setFixedSize(50, 25);
        QPushButton* delBtn = new QPushButton("删除");
        delBtn->setFixedSize(50, 25);

        btnLayout->addWidget(fixedBtn);
        btnLayout->addWidget(replayBtn);
        btnLayout->addWidget(delBtn);
        btnWidget->setLayout(btnLayout);
        table->setCellWidget(i, 5, btnWidget);

        connect(fixedBtn, &QPushButton::clicked, [&d, fixedBtn, &saveAll, &updateFixedButton]() {
            d.fixed = !d.fixed;
            updateFixedButton(fixedBtn, d.fixed);
            saveAll();
        });

        connect(replayBtn, &QPushButton::clicked, [&d, &dialog]() {
            dialog.accept();
            ReplayWidget* replay = new ReplayWidget(d);
            replay->setAttribute(Qt::WA_DeleteOnClose);
            replay->show();
        });

        connect(delBtn, &QPushButton::clicked, [this, i, &historyList, &table, &saveAll, &dialog]() {
            if (QMessageBox::Yes == QMessageBox::question(&dialog, "确认删除", "确定要删除这条记录吗？")) {
                historyList.erase(historyList.begin() + i);
                table->removeRow(i);
                saveAll();
                if (historyList.isEmpty()) {
                    dialog.accept();
                    QMessageBox::information(this, "历史记录", "历史记录已清空");
                }
            }
        });
    }

    layout->addWidget(table);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    QPushButton* clearAllBtn = new QPushButton("清除所有历史记录");
    QPushButton* closeBtn = new QPushButton("关闭");
    bottomLayout->addWidget(clearAllBtn);
    bottomLayout->addWidget(closeBtn);
    layout->addLayout(bottomLayout);

    connect(clearAllBtn, &QPushButton::clicked, [&]() {
        if (QMessageBox::Yes == QMessageBox::question(&dialog, "确认清除", "确定要清除所有历史记录吗？")) {
            QFile::remove("history.dat");
            dialog.accept();
            QMessageBox::information(this, "历史记录", "历史记录已清除");
        }
    });
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}