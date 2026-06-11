#include "savedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include "gamedata.h"

SaveDialog::SaveDialog(QWidget *parent, bool forLoad)
    : QDialog(parent), selectedSlot(-1), m_forLoad(forLoad)
{
    setWindowTitle(forLoad ? "读取存档" : "保存游戏");
    setModal(true);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* title = new QLabel(forLoad ? "选择一个存档位读取" : "选择一个存档位保存（可覆盖）", this);
    mainLayout->addWidget(title);

    QHBoxLayout* slotsLayout = new QHBoxLayout();
    for (int i = 1; i <= 3; ++i) {
        QPushButton* btn = new QPushButton(this);
        btn->setFixedSize(240, 180);
        btn->setStyleSheet("text-align: left; padding: 10px;");
        QString info;
        QFile file(QString("saves/slot%1.dat").arg(i));
        if (file.exists()) {
            if (file.open(QIODevice::ReadOnly)) {
                QDataStream in(&file);
                SaveData data;
                in >> data;
                file.close();
                QString modeStr;
                if (data.mode == 0) modeStr = "单人随机";
                else if (data.mode == 1) modeStr = "AI对决";
                else modeStr = "挑战模式";
                info = QString("%1\n得分: %2\n用时: %3秒\n棋盘: %4x%4")
                           .arg(modeStr).arg(data.playerScore).arg(data.elapsedSeconds).arg(data.boardSize);
                if (data.hasAI) {
                    info += QString("\nAI得分: %1").arg(data.aiScore);
                }
            } else {
                info = "存档文件损坏";
            }
        } else {
            info = "空存档位";
        }
        btn->setText(info);
        slotsLayout->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, i](){ onSlotClicked(i); });
    }
    mainLayout->addLayout(slotsLayout);

    QPushButton* cancelBtn = new QPushButton("取消", this);
    connect(cancelBtn, &QPushButton::clicked, this, &SaveDialog::reject);
    mainLayout->addWidget(cancelBtn);

    setLayout(mainLayout);
    resize(800, 280);
}

void SaveDialog::onSlotClicked(int slot)
{
    selectedSlot = slot;
    accept();
}