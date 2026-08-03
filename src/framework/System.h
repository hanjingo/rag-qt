#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QString>
#include <QPointer>

#include "Global.h"

class System : public QObject
{
    Q_OBJECT

  public:
    explicit System(QObject *parent = nullptr);
    ~System();

    static QPointer<System> instance()
    {
        static QPointer<System> inst = new System();
        return inst;
    }

    QString LocalLang();

    QString Version() { return QString(RAG_QT_VERSION); }
    QString Platform() { return QSysInfo::productType(); }
    QString Arch();
};

#endif