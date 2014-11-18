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

//#define QT_NO_DEBUG_OUTPUT

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
    QObject(parent), jsonPath(jsonPath), creationCheckInterval(100)
{
    // We've just started
    buildMenu();
    if (!QFile::exists(jsonPath))
    {
        fileExisted = false;
        waitForCreation();
    }
    else
    {
        fileExisted = true;
        startWatching();
    }

    icon.setContextMenu(&rootMenu);

    connect(&watcher,SIGNAL(fileChanged(QString)),this,SLOT(fileChanged()));
    connect(&icon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(showMenuAtIcon()));
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

void QuickMenu::buildMenu()
{
    qDebug() << "building menu";
    rootMenu.clear();
    foreach (QMenu *m, subMenus)
    {
        m->deleteLater();
    }
    subMenus.clear();

    auto rootDoc = readJsonFile();
    qDebug() << "rootDoc is null?" << rootDoc.isNull();

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

        if (!rootObj.value("hide-quit").toBool())
        {
            rootMenu.addAction("Quit",qApp,SLOT(quit()));
        }
    }
    else
    {
        icon.setIcon(QIcon(":/default-warning.png"));
    }

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

void QuickMenu::startWatching()
{
    watcher.addPath(jsonPath);
}

void QuickMenu::waitForCreation()
{
    if (!QFile::exists(jsonPath))
    {
        qDebug() << "path" << jsonPath << "doesn't exist";
        QTimer::singleShot(creationCheckInterval, this, SLOT(waitForCreation()));
        creationCheckInterval = qMin(creationCheckInterval * 2, 1000);
        if (fileExisted)
        {
            buildMenu(); // The file disappeared, reflect it in the menu
            fileExisted = false;
        }
    }
    else
    {
        buildMenu();
        fileExisted = true;
        startWatching();
    }
}

void QuickMenu::fileChanged()
{
    qDebug() << "fileChanged";
    if (!watcher.files().contains(jsonPath))
    {
        qDebug() << "watch list didn't contain" << jsonPath;
        creationCheckInterval = 100;
        // We'll give the file 100ms to reappear before changing the
        // icon to a warning, in case it was just overwritten, or moved.
        // After that, the menu will reflect the error, and QuickMenu
        // will start checking regularly for the file to reappear.
        QTimer::singleShot(creationCheckInterval, this, SLOT(waitForCreation()));
    }
    else
    {
        buildMenu();
    }
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

