#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QString>

#include "Global.h"

class System : public QObject
{
    Q_OBJECT

  public:
    explicit System(QObject *parent = nullptr);
    ~System();

    static System *Instance()
    {
        static System inst;
        return &inst;
    }

    QString LocalLang();

    QString Version() { return QString(RAG_QT_VERSION); }
    QString Platform() { return QSysInfo::productType(); }
    QString Arch();
};

#endif