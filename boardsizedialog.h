#ifndef BOARDSIZEDIALOG_H
#define BOARDSIZEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>

class BoardSizeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BoardSizeDialog(QWidget *parent = nullptr);
    int getSelectedSize() const { return selectedSize; }

private slots:
    void onSizeSelected(int size);

private:
    int selectedSize;
};

#endif