#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include <QDialog>
#include <QVector>

class SaveDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SaveDialog(QWidget *parent = nullptr, bool forLoad = false);
    int getSelectedSlot() const { return selectedSlot; } // 返回1-3，或-1表示取消

private slots:
    void onSlotClicked(int slot);

private:
    void loadSlotInfo(int slot);
    void updateButton(int slot, const QString& info);
    int selectedSlot;
    bool m_forLoad;
};

#endif