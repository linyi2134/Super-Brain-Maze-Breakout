#ifndef MODESELECTION_H
#define MODESELECTION_H

#include <QWidget>

namespace Ui {
class ModeSelection;
}

class ModeSelection : public QWidget
{
    Q_OBJECT

public:
    explicit ModeSelection(QWidget *parent = nullptr);
    ~ModeSelection();

signals:
    void singlePlayerSelected();
    void aiBattleSelected();
    void backToMenu();

private slots:
    void onSinglePlayerClicked();
    void onAIClicked();
    void onMultiplayerClicked();
    void onBackClicked();

private:
    Ui::ModeSelection *ui;
};

#endif