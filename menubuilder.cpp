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

#include "menubuilder.h"

MenuBuilder::MenuBuilder(QString jsonPath, QObject *parent) :
    QObject(parent), jsonPath(jsonPath)
{
    QJsonParseError parseError;
    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr,"Error",QString("Error opening %1: %2")
                             .arg(jsonPath)
                             .arg(f.errorString()),QMessageBox::Ok);
        qDebug() << "Error opening" << jsonPath;
        exit(1);
    }
    QByteArray data = f.readAll();
    QJsonDocument rootDoc = QJsonDocument::fromJson(data,&parseError);
    if (rootDoc.isNull())
    {
        std::pair<int,int> p = position(data, parseError);
        QMessageBox::warning(nullptr,"Parse Error",QString("%1: %2:%3: %4")
                             .arg(jsonPath)
                             .arg(p.first)
                             .arg(p.second)
                             .arg(parseError.errorString()),QMessageBox::Ok);
        exit(1);
    }

    auto rootObj = rootDoc.object();
    addMenu(&rootMenu, rootObj);

    rootMenu.addAction("Quit",qApp,SLOT(quit()));

    if (rootMenu.icon().isNull())
    {
        icon.setIcon(QIcon(":/default.png"));
    }
    else
    {
        icon.setIcon(rootMenu.icon());
    }
    icon.setContextMenu(&rootMenu);
    icon.show();
}

void MenuBuilder::actionTriggered()
{
    QAction* a = (QAction *)sender();
    QJsonObject entry = a->data().toJsonObject();
    QString pwd = entry.value("pwd").toString();
    QStringList args;
    args << "-c";
    args << entry.value("action").toString();
    QProcess::startDetached("sh",args,pwd);
}

void MenuBuilder::addMenu(QMenu *menu, const QJsonObject &obj)
{
    if (!obj.contains("menu")) return;

    menu->setIcon(loadIcon(obj));

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
            addMenu(subMenu, entry);
        }
        else if (entry.contains("action"))
        {
            QAction *action = menu->addAction(loadIcon(entry),label,this,SLOT(actionTriggered()));
            action->setData(QVariant(entry));
        }
    }
}

QIcon MenuBuilder::loadIcon(const QJsonObject &obj)
{
    QFileInfo fi(jsonPath);
    auto iconPath = fi.dir().absoluteFilePath(obj.value("icon").toString());
    if (QFile::exists(iconPath))
        return QIcon(iconPath);
    return QIcon();
}

std::pair<int, int> MenuBuilder::position(const QByteArray &data, const QJsonParseError &parseError)
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

