/*

QuickMenu Copyright (C) 2014 Andy Teijelo <ateijelo@gmail.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

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
