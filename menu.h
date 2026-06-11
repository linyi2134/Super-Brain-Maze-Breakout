#ifndef MENU_H
#define MENU_H

#include <QWidget>

namespace Ui {
class Menu;
}

class Menu : public QWidget
{
    Q_OBJECT
public:
    explicit Menu(QWidget *parent = nullptr);
    ~Menu();

signals:
    void newGameRequested();
    void exitRequested();
    void loadArchive(int slot);
private slots:
    void onNewGameButtonClicked();
    void onRulesButtonClicked();
    void onExitButtonClicked();
    void onArchiveButtonClicked();
    void onHistoryButtonClicked();

private:
    Ui::Menu *ui;
};

#endif