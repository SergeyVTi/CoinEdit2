#include "coinedit.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationVersion("1.08");
    app.setWindowIcon(QIcon(":/icons/icon.png"));
    QGuiApplication::setApplicationDisplayName("CoinEdit 2 v" + app.applicationVersion());

    CoinEdit coinEdit;
    coinEdit.show();

    return app.exec();
}
