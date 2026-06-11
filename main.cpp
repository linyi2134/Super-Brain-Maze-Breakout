#include <QApplication>
#include <cstdlib>
#include <ctime>
#include "menu.h"
#include "modeselection.h"
#include "singleplayermode.h"
#include "gamewidget.h"
#include "challengequestion.h"
#include "boardsizedialog.h"
#include <QFile>
#include <QDataStream>
#include <QObject>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    srand(static_cast<unsigned>(time(nullptr)));
    QApplication app(argc, argv);

    Menu menu;
    menu.show();

    ModeSelection modeSel;
    SinglePlayerMode singleMode;

    static GameWidget* currentGame = nullptr;

    auto closeCurrentGame = [&]() {
        if (currentGame) {
            currentGame->close();
            currentGame->deleteLater();
            currentGame = nullptr;
        }
    };

    QObject::connect(&menu, &Menu::newGameRequested, [&]() { modeSel.show(); });
    QObject::connect(&menu, &Menu::exitRequested, &app, &QApplication::quit);

    QObject::connect(&modeSel, &ModeSelection::singlePlayerSelected, [&]() { singleMode.show(); });
    QObject::connect(&modeSel, &ModeSelection::aiBattleSelected, [&]() {
        BoardSizeDialog dlg;
        if (dlg.exec() == QDialog::Accepted) {
            int size = dlg.getSelectedSize();
            closeCurrentGame();
            currentGame = new GameWidget("玩家", true, nullptr, size);
            currentGame->setAttribute(Qt::WA_DeleteOnClose);
            QObject::connect(currentGame, &GameWidget::backToMenu, [&]() {
                menu.show();
                currentGame = nullptr;
            });
            currentGame->show();
        }
    });
    QObject::connect(&modeSel, &ModeSelection::backToMenu, [&]() { menu.show(); });

    QObject::connect(&singleMode, &SinglePlayerMode::backToModeSelection, [&]() { modeSel.show(); });
    QObject::connect(&singleMode, &SinglePlayerMode::startRandomGame, [&]() {
        BoardSizeDialog dlg;
        if (dlg.exec() == QDialog::Accepted) {
            int size = dlg.getSelectedSize();
            closeCurrentGame();
            currentGame = new GameWidget("玩家", false, nullptr, size);
            currentGame->setAttribute(Qt::WA_DeleteOnClose);
            QObject::connect(currentGame, &GameWidget::backToMenu, [&]() {
                menu.show();
                currentGame = nullptr;
            });
            currentGame->show();
        }
    });

    QObject::connect(&singleMode, &SinglePlayerMode::startChallenge, [&](int level) {
        const std::vector<std::vector<std::string>>& boardData = ChallengeQuestion::getBoardByLevel(level);
        int size = static_cast<int>(boardData.size());
        closeCurrentGame();
        currentGame = new GameWidget("玩家", false, &boardData, size);
        currentGame->setAttribute(Qt::WA_DeleteOnClose);
        QObject::connect(currentGame, &GameWidget::backToMenu, [&]() {
            menu.show();
            currentGame = nullptr;
        });
        currentGame->show();
    });

    QObject::connect(&menu, &Menu::loadArchive, [&](int slot) {
        closeCurrentGame();
        QString fileName = QString("saves/slot%1.dat").arg(slot);
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(nullptr, "读档失败", "存档文件不存在或无法读取");
            menu.show();
            return;
        }
        QDataStream in(&file);
        SaveData data;
        in >> data;
        file.close();

        int size = data.boardSize;
        bool aiMode = (data.mode == 1);
        GameWidget* game = new GameWidget(data.playerName, aiMode, nullptr, size, true);
        game->setAttribute(Qt::WA_DeleteOnClose);
        game->loadFromSaveData(data);
        QObject::connect(game, &GameWidget::backToMenu, [&]() {
            menu.show();
            currentGame = nullptr;
        });
        currentGame = game;
        game->show();
    });

    return app.exec();
}