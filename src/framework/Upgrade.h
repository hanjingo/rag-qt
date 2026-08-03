#ifndef UPGRADE_H
#define UPGRADE_H

#include <QObject>
#include <QString>
#include <QPointer>

#include "Global.h"
#include "System.h"
#include "Config.h"

class Upgrade : public QObject
{
    Q_OBJECT

  public:
    explicit Upgrade(QObject *parent = nullptr) {};
    ~Upgrade() {};

    static QPointer<Upgrade> instance()
    {
        static QPointer<Upgrade> inst = new Upgrade();
        return inst;
    }

    QString GetUpgradeAddr()
    {
        QVector<QString> urls = Config::instance()->getAppUpgradeUrls();
        QString          addr;
        if(!urls.isEmpty())
            addr = urls.first();

        return addr;
    }
};

#endif // UPGRADE_H