#include <QtDebug>
#include <QFile>
#include <QAction>
#include <QJsonDocument>
#include <QJsonArray>
#include <QApplication>
#include <QMessageBox>
#include <QProcess>

#include "menubuilder.h"

MenuBuilder::MenuBuilder(QString jsonPath, QObject *parent) :
    QObject(parent)
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
    auto iconPath = obj.value("icon").toString();
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

