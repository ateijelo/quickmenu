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

#ifndef QUICKMENU_H
#define QUICKMENU_H

#include <QSet>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>
#include <QJsonObject>
#include <QJsonParseError>
#include <QFileSystemWatcher>
#include <QLocalSocket>
#include <QLocalServer>

class QuickMenu : public QObject
{
    Q_OBJECT
    public:
        explicit QuickMenu(QString jsonPath, QObject *parent = 0);
        void listenOn(const QString& name);
        void show();

    private slots:
        void actionTriggered();
        void showError(const QString& title, const QString& msg);
        void fileChanged();
        void newConnection();
        void showMenuAtIcon();
        void showMenuAtMouse();

    private:
        QJsonDocument readJsonFile();
        void addMenu(QMenu *menu, const QJsonObject& obj);
        void loadRootMenu();
        QIcon loadIcon(const QJsonObject& obj);
        std::pair<int, int> position(const QByteArray& data, const QJsonParseError& parseError);

        QSystemTrayIcon icon;
        QMenu rootMenu;
        QString jsonPath;
        QFileSystemWatcher watcher;
        QSet<QMenu *> subMenus; // QMenu::clear doesn't delete submenus, I keep track of them here
                                // to delete them myself on reload
        QLocalServer *server;

};

#endif // QUICKMENU_H
