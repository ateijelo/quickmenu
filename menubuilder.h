#ifndef MENUBUILDER_H
#define MENUBUILDER_H

#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>
#include <QJsonObject>
#include <QJsonParseError>

class MenuBuilder : public QObject
{
    Q_OBJECT
public:
    explicit MenuBuilder(QString jsonPath, QObject *parent = 0);

signals:

public slots:

private slots:
    void actionTriggered();

private:
    void addMenu(QMenu *menu, const QJsonObject& obj);
    QIcon loadIcon(const QJsonObject& obj);
    std::pair<int, int> position(const QByteArray& data, const QJsonParseError& parseError);

    QSystemTrayIcon icon;
    QMenu rootMenu;

};

#endif // MENUBUILDER_H
