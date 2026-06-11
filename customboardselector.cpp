#include "customboardselector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>

CustomBoardSelector::CustomBoardSelector(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("选择自建棋盘");
    setModal(true);
    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("请选择一个已保存的棋盘："));

    QHBoxLayout* slotsLayout = new QHBoxLayout();
    for (int i = 1; i <= 5; ++i) {
        QPushButton* btn = new QPushButton(QString("存档%1").arg(i));
        btn->setFixedSize(100, 80);
        btn->setProperty("slot", i);
        connect(btn, &QPushButton::clicked, this, [this, i]() { onSlotClicked(i); });
        slotsLayout->addWidget(btn);
        slotButtons.append(btn);

        QString fileName = QString("custom/board_%1.txt").arg(i);
        QFile file(fileName);
        bool valid = false;
        if (file.exists() && file.size() > 0) {
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                int size;
                in >> size;
                if (size > 0 && size <= 25) {
                    valid = true;
                    btn->setText(QString("存档%1\n%2x%2").arg(i).arg(size));
                } else {
                    btn->setText(QString("存档%1\n(损坏)").arg(i));
                }
                file.close();
            } else {
                btn->setText(QString("存档%1\n(损坏)").arg(i));
            }
        } else {
            btn->setText(QString("存档%1\n空").arg(i));
        }
        btn->setEnabled(valid);   // 只有有效的存档才可点击
    }

    // 将按钮布局添加到主布局
    layout->addLayout(slotsLayout);

    // 添加“取消”按钮
    QPushButton* cancelBtn = new QPushButton("取消", this);
    layout->addWidget(cancelBtn);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    setLayout(layout);
    resize(600, 200);   // 确保窗口大小合适
}

CustomBoardSelector::~CustomBoardSelector() {}

void CustomBoardSelector::onSlotClicked(int slot)
{
    QString fileName = QString("custom/board_%1.txt").arg(slot);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "存档文件不存在或无法读取");
        return;
    }

    QTextStream in(&file);
    QString firstLine = in.readLine().trimmed();
    bool ok;
    int boardSize = firstLine.toInt(&ok);
    if (!ok || boardSize <= 0 || boardSize > 25) {
        QMessageBox::warning(this, "错误", "存档文件损坏：棋盘大小无效");
        file.close();
        return;
    }

    std::vector<std::vector<std::string>> board(boardSize, std::vector<std::string>(boardSize));
    int totalLines = boardSize * boardSize;
    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            QString line = in.readLine();
            if (line.isNull()) {
                QMessageBox::warning(this, "错误", "存档文件格式错误：数据不足");
                file.close();
                return;
            }
            QString code = line.trimmed();
            board[i][j] = code.toStdString();
        }
    }
    file.close();

    emit startGame(board, boardSize);
    accept();
}