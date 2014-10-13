/*

The MIT License (MIT)

Copyright (c) 2014 Andy Teijelo <ateijelo@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QLocalServer>
#include <QLocalSocket>

#include "quickmenu.h"

void activate(const QString& name)
{
    QLocalSocket s;
    s.connectToServer(name);
    s.waitForConnected();
    s.readAll();
    s.close();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationDomain("ateijelo.com");
    app.setOrganizationName("ateijelo");
    app.setApplicationName("QuickMenu");
    app.setApplicationVersion("1.0");

    QCommandLineParser p;
    p.setApplicationDescription("Easily create menus for your notification area");
    p.addHelpOption();
    p.addVersionOption();

    p.addPositionalArgument("jsonfile","JSON file describing the menu");
    QCommandLineOption nameOption(QStringList() << "n" << "name",
                                  "Give this menu a name to be remotely activated","name");
    QCommandLineOption showOption(QStringList() << "s" << "show",
                                  "Remotely activate a named menu","name");

    p.addOption(nameOption);
    p.addOption(showOption);

    p.process(app);

    if (p.isSet("show"))
    {
        activate(p.value("show"));
        return 0;
    }

    app.setQuitOnLastWindowClosed(false);

    if (p.positionalArguments().length() < 1)
    {
        p.showHelp();
        return 0;
    }
    QuickMenu b(p.positionalArguments().at(0));
    b.show();

    if (p.isSet(nameOption))
    {
        b.listenOn(p.value("name"));
    }

    return app.exec();
}
