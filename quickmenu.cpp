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

#include <QtDebug>
#include <QFile>
#include <QAction>
#include <QJsonDocument>
#include <QJsonArray>
#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QScreen>

#include "quickmenu.h"

void QuickMenu::show()
{
    icon.show();
}

QuickMenu::QuickMenu(QString jsonPath, QObject *parent) :
    QObject(parent), jsonPath(jsonPath)
{
    loadRootMenu();
    icon.setContextMenu(&rootMenu);

    watcher.addPath(jsonPath);
    connect(&watcher,SIGNAL(fileChanged(QString)),this,SLOT(fileChanged()));
    connect(&icon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(showMenuAtMouse()));
}

QJsonDocument QuickMenu::readJsonFile()
{
    QJsonDocument rootDoc;

    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly))
    {
        rootMenu.addAction(QIcon::fromTheme("dialog-warning"), QString("Error opening %1: %2")
                             .arg(jsonPath)
                             .arg(f.errorString()));
        return rootDoc;
    }

    QByteArray data;
    QJsonParseError parseError;

    data = f.readAll();
    rootDoc = QJsonDocument::fromJson(data,&parseError);
    if (rootDoc.isNull())
    {
        std::pair<int,int> p = position(data, parseError);
        rootMenu.addAction(QIcon::fromTheme("dialog-warning"),QString("Parse Error: %1: %2:%3: %4")
                                 .arg(jsonPath)
                                 .arg(p.first)
                                 .arg(p.second)
                                 .arg(parseError.errorString()));
    }
    return rootDoc;
}

void QuickMenu::loadRootMenu()
{
    rootMenu.clear();
    foreach (QMenu *m, subMenus)
    {
        m->deleteLater();
    }
    subMenus.clear();

    auto rootDoc = readJsonFile();

    if (!(rootDoc.isNull()))
    {
        auto rootObj = rootDoc.object();
        addMenu(&rootMenu, rootObj);

        if (rootMenu.icon().isNull())
        {
            icon.setIcon(QIcon(":/default.png"));
        }
        else
        {
            icon.setIcon(rootMenu.icon());
        }
    }
    else
    {
        icon.setIcon(QIcon(":/default-warning.png"));
    }

    rootMenu.addAction("Quit",qApp,SLOT(quit()));
}

void QuickMenu::actionTriggered()
{
    QAction* a = (QAction *)sender();
    QJsonObject entry = a->data().toJsonObject();
    QString pwd = entry.value("pwd").toString();
    QStringList args;
    args << "-c";
    args << entry.value("action").toString();
    QProcess::startDetached("sh",args,pwd);
}

void QuickMenu::showError(const QString &title, const QString &msg)
{
    icon.showMessage(title, msg);
}

void QuickMenu::fileChanged()
{
    loadRootMenu();
}

void QuickMenu::newConnection()
{
    showMenuAtIcon();
}

void QuickMenu::showMenuAtIcon()
{
    qDebug() << icon.geometry().topLeft();
    //icon.contextMenu()->popup(icon.geometry().topLeft());
    int top = icon.geometry().top();
    if (top > (qApp->primaryScreen()->size().height() / 2))
    {
        top -= rootMenu.sizeHint().height();
    }
    int left = icon.geometry().left();
    qDebug() << qApp->primaryScreen()->size();
    icon.contextMenu()->popup(QPoint(left,top));
}

void QuickMenu::showMenuAtMouse()
{
    qDebug() << QCursor::pos();
    icon.contextMenu()->popup(QCursor::pos());
}

void QuickMenu::addMenu(QMenu *menu, const QJsonObject &obj)
{
    if (!obj.contains("menu")) return;

    menu->setIcon(loadIcon(obj));

    if (obj.contains("shortcut"))
    {
        menu->menuAction()->setShortcut(QKeySequence(obj.value("shortcut").toString()));
    }

    QJsonArray entries = obj.value("menu").toArray();
    foreach (QJsonValue e, entries)
    {
        if (!e.isObject())
            continue;
        auto entry = e.toObject();
        auto label = entry.value("label").toString();
        if (entry.contains("menu"))
        {
            QMenu *subMenu = menu->addMenu(label);
            subMenus.insert(subMenu);
            addMenu(subMenu, entry);
        }
        else if (entry.contains("action"))
        {
            QAction *action = menu->addAction(loadIcon(entry),label,this,SLOT(actionTriggered()));
            action->setData(QVariant(entry));
        }
    }
}

QIcon QuickMenu::loadIcon(const QJsonObject &obj)
{
    QFileInfo fi(jsonPath);
    auto iconPath = fi.dir().absoluteFilePath(obj.value("icon").toString());
    if (QFile::exists(iconPath))
        return QIcon(iconPath);
    return QIcon();
}

std::pair<int, int> QuickMenu::position(const QByteArray &data, const QJsonParseError &parseError)
{
    int line = 1;
    int column = 1;
    for (int i=0; i<parseError.offset; i++)
    {
        if (data.at(i) == '\n')
        {
            line += 1;
            column = 0;
        }
        column++;
    }
    return {line,column};
}

void QuickMenu::listenOn(const QString &name)
{
    server = new QLocalServer(this);
    int attempts = 2;
    while (true)
    {
        attempts--;
        server->listen(name);
        if (server->isListening())
        {
            break;
        }
        else
        {
            QLocalServer::removeServer(name);
        }
        if (attempts <= 0)
        {
            qDebug() << QString("Failed to create local server '%1'").arg(name);
            break;
        }
    }
    connect(server,SIGNAL(newConnection()),this,SLOT(newConnection()));
}

