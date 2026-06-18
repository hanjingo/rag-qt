#ifndef BUS_H
#define BUS_H

#include <QObject>

class Bus : public QObject
{
    Q_OBJECT
  public:
    static Bus *Instance();

  signals:
    void SignalPing();
    void SignalPong();

  private:
    explicit Bus(QObject *parent = nullptr)
        : QObject(parent) {};
    ~Bus() {};
};

#endif