#include "customboardeditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QInputDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>

CustomBoardEditor::CustomBoardEditor(QWidget *parent)
    : QWidget(parent), boardSize(10)
{
    setupUI();
    setWindowTitle("自建题库 - 创建棋盘");
    resize(600, 600);
}

CustomBoardEditor::~CustomBoardEditor() {}

void CustomBoardEditor::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("棋盘大小:"));
    sizeCombo = new QComboBox();
    sizeCombo->addItem("5×5", 5);
    sizeCombo->addItem("10×10", 10);
    sizeCombo->addItem("15×15", 15);
    sizeCombo->addItem("20×20", 20);
    sizeCombo->addItem("25×25", 25);
    connect(sizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CustomBoardEditor::onSizeSelected);
    sizeLayout->addWidget(sizeCombo);
    mainLayout->addLayout(sizeLayout);

    // 使用 QTableWidget 替代 QGridLayout
    tableWidget = new QTableWidget(this);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止直接编辑
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(tableWidget, &QTableWidget::itemClicked, this, &CustomBoardEditor::onCellClicked);
    mainLayout->addWidget(tableWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* clearBtn = new QPushButton("清除选中格子");
    QPushButton* saveBtn = new QPushButton("保存到存档位");
    QPushButton* cancelBtn = new QPushButton("取消");
    btnLayout->addWidget(clearBtn);
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(clearBtn, &QPushButton::clicked, this, &CustomBoardEditor::onClearButtonClicked);
    connect(saveBtn, &QPushButton::clicked, this, &CustomBoardEditor::onSaveButtonClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &CustomBoardEditor::onCancelButtonClicked);

    createBoardGrid(boardSize);
}

void CustomBoardEditor::createBoardGrid(int size)
{
    tableWidget->clear();
    tableWidget->setRowCount(size);
    tableWidget->setColumnCount(size);
    for (int i = 0; i < size; ++i) {
        tableWidget->setRowHeight(i, 50);
        tableWidget->setColumnWidth(i, 50);
        for (int j = 0; j < size; ++j) {
            QTableWidgetItem* item = new QTableWidgetItem("  ");
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(Qt::white);
            tableWidget->setItem(i, j, item);
        }
    }
    // 存储当前棋盘大小
    boardSize = size;
}

void CustomBoardEditor::onSizeSelected(int index)
{
    int newSize = sizeCombo->itemData(index).toInt();
    if (newSize == boardSize) return;

    // 检查是否有非空格子
    int nonEmpty = 0;
    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            QTableWidgetItem* item = tableWidget->item(i, j);
            if (item && item->text().trimmed() != "") {
                nonEmpty++;
            }
        }
    }
    if (nonEmpty > 0) {
        int ret = QMessageBox::question(this, "确认", "切换大小将丢失所有数据，是否继续？",
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) {
            // 恢复下拉框
            int idx = sizeCombo->findData(boardSize);
            if (idx >= 0) {
                sizeCombo->blockSignals(true);
                sizeCombo->setCurrentIndex(idx);
                sizeCombo->blockSignals(false);
            }
            return;
        }
    }
    createBoardGrid(newSize);
}

void CustomBoardEditor::onCellClicked(QTableWidgetItem* item)
{
    if (!item) return;
    int row = item->row();
    int col = item->column();

    QString currentText = item->text().trimmed();
    if (currentText == "") currentText = "  ";

    bool ok;
    QString text = QInputDialog::getText(this, "输入棋子代码",
                                         "请输入代码（如 A3）或留空表示空格：",
                                         QLineEdit::Normal, currentText, &ok);
    if (ok) {
        QString newCode;
        if (text.trimmed().isEmpty()) {
            newCode = "  ";
        } else if (text.length() >= 2) {
            QChar ch = text[0].toUpper();
            if (ch >= 'A' && ch <= 'E' && text[1].isDigit()) {
                newCode = text.left(2);
            } else {
                QMessageBox::warning(this, "错误", "代码格式应为字母A-E加数字0-9，例如 A3");
                return;
            }
        } else {
            QMessageBox::warning(this, "错误", "代码格式应为字母A-E加数字0-9，例如 A3");
            return;
        }
        item->setText(newCode);
        // 设置背景色：有代码的格子浅灰色，空格白色
        if (newCode == "  ") {
            item->setBackground(Qt::white);
        } else {
            item->setBackground(QColor(220, 220, 220));
        }
    }
}

void CustomBoardEditor::onClearButtonClicked()
{
    QList<QTableWidgetItem*> selected = tableWidget->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先点击选中一个格子");
        return;
    }
    for (QTableWidgetItem* item : selected) {
        item->setText("  ");
        item->setBackground(Qt::white);
    }
}

void CustomBoardEditor::onSaveButtonClicked()
{
    // 统计非空格子数量
    int nonEmptyCount = 0;
    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            QTableWidgetItem* item = tableWidget->item(i, j);
            if (item && item->text().trimmed() != "") {
                nonEmptyCount++;
            }
        }
    }

    // 按照新规则设置最低棋子数
    int minRequired = 0;
    switch (boardSize) {
    case 5:  minRequired = 1; break;
    case 10: minRequired = 3; break;
    case 15: minRequired = 5; break;
    case 20: minRequired = 7; break;
    case 25: minRequired = 9; break;
    default: minRequired = 2; break;
    }

    qDebug() << "nonEmptyCount:" << nonEmptyCount << "minRequired:" << minRequired;  // 调试输出

    if (nonEmptyCount < minRequired) {
        QMessageBox::warning(this, "保存失败",
                             QString("棋盘上的棋子数量不足！\n当前有 %1 个棋子，%2x%2 棋盘至少需要 %3 个棋子。")
                                 .arg(nonEmptyCount).arg(boardSize).arg(minRequired));
        return;
    }

    bool ok;
    int slot = QInputDialog::getInt(this, "保存棋盘", "选择存档位 (1-5):", 1, 1, 5, 1, &ok);
    if (!ok) return;

    QDir dir;
    if (!dir.exists("custom")) dir.mkdir("custom");

    QString fileName = QString("custom/board_%1.txt").arg(slot);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "保存失败", "无法写入文件");
        return;
    }

    QTextStream out(&file);
    out << boardSize << "\n";
    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            QTableWidgetItem* item = tableWidget->item(i, j);
            QString code = item ? item->text().trimmed() : "";
            if (code == "") code = "";
            out << code << "\n";
        }
    }
    file.close();

    QMessageBox::information(this, "保存成功", QString("棋盘已保存到存档位 %1").arg(slot));
    close();
}

void CustomBoardEditor::onCancelButtonClicked()
{
    close();
}