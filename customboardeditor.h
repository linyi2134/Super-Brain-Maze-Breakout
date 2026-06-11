#ifndef CUSTOMBOARDEDITOR_H
#define CUSTOMBOARDEDITOR_H

#include <QWidget>
#include <QComboBox>
#include <QTableWidget>

class CustomBoardEditor : public QWidget
{
    Q_OBJECT
public:
    explicit CustomBoardEditor(QWidget *parent = nullptr);
    ~CustomBoardEditor();

private slots:
    void onSizeSelected(int index);
    void onCellClicked(QTableWidgetItem* item);
    void onClearButtonClicked();
    void onSaveButtonClicked();
    void onCancelButtonClicked();

private:
    void setupUI();
    void createBoardGrid(int size);

    int boardSize;
    QComboBox* sizeCombo;
    QTableWidget* tableWidget;
};

#endif