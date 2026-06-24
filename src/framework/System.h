#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QString>

class System : public QObject
{
    Q_OBJECT

  public:
    static System *Instance();
    explicit System(QObject *parent = nullptr);
    ~System();

    QString LocalLang();

  private:
    static System *m_stSystemInst;
};

#endif