#include "boardsizedialog.h"
#include <QPushButton>
#include <QVBoxLayout>

BoardSizeDialog::BoardSizeDialog(QWidget *parent)
    : QDialog(parent), selectedSize(10)
{
    setWindowTitle("选择棋盘大小");
    QVBoxLayout *layout = new QVBoxLayout(this);

    QPushButton *btn5  = new QPushButton("5×5", this);
    QPushButton *btn10 = new QPushButton("10×10", this);
    QPushButton *btn15 = new QPushButton("15×15", this);
    QPushButton *btn20 = new QPushButton("20×20", this);
    QPushButton *btn25 = new QPushButton("25×25", this);

    layout->addWidget(btn5);
    layout->addWidget(btn10);
    layout->addWidget(btn15);
    layout->addWidget(btn20);
    layout->addWidget(btn25);

    connect(btn5,  &QPushButton::clicked, this, [this]() { onSizeSelected(5); });
    connect(btn10, &QPushButton::clicked, this, [this]() { onSizeSelected(10); });
    connect(btn15, &QPushButton::clicked, this, [this]() { onSizeSelected(15); });
    connect(btn20, &QPushButton::clicked, this, [this]() { onSizeSelected(20); });
    connect(btn25, &QPushButton::clicked, this, [this]() { onSizeSelected(25); });
}

void BoardSizeDialog::onSizeSelected(int size)
{
    selectedSize = size;
    accept();
}