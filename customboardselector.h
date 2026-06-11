#ifndef CUSTOMBOARDSELECTOR_H
#define CUSTOMBOARDSELECTOR_H

#include <QDialog>
#include <QVector>
#include <QPushButton>

class CustomBoardSelector : public QDialog
{
    Q_OBJECT
public:
    explicit CustomBoardSelector(QWidget *parent = nullptr);
    ~CustomBoardSelector();

signals:
    void startGame(const std::vector<std::vector<std::string>>& board, int boardSize);

private slots:
    void onSlotClicked(int slot);

private:
    QVector<QPushButton*> slotButtons;
};

#endif