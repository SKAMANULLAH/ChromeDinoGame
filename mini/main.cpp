#include <QApplication>
#include "DinoWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    DinoWidget game;
    game.show();

    return app.exec();
}
