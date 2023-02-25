#include "coinedit.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName("Coinedit 2");

    CoinEdit coinEdit;
    coinEdit.show();

    return app.exec();
}
