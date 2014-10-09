#include <QApplication>

#include "menubuilder.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setQuitOnLastWindowClosed(false);
    MenuBuilder b(app.arguments().at(1));

    return app.exec();
}
