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
