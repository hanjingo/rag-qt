#ifndef BUS_H
#define BUS_H

#include <QObject>
#include <QString>
#include <QVector>

#define BUS_VERSION_MAJOR 0
#define BUS_VERSION_MINOR 0
#define BUS_VERSION_PATCH 1

class Bus : public QObject
{
    Q_OBJECT
  public:
    struct Session
    {
        qint64  id;
        qint64  userId;
        QString title;
        QString content;
        QString timestamp;
    };
    struct Model
    {
        QString hash;
        QString name;
        QString publisher;
        QString timestamp;
        QString addr;
        QString capabilities;
        qint64  contextSize;
        qint32  cost;
    };

    struct Skill
    {
        QString hash;
        QString name;
        QString desc;
        QString publisher;
        QString version;
        QString timestamp;
        qint32  platform;
    };

  public:
    static Bus *Instance();
    static void Version(int8_t &major, int8_t &minor, int8_t &patch);

  signals:
    void SignalPong();
    void SignalPing();

    void SignalLanguageSwitch(const QString &lang);

    void SignalModelInfoUpdate(const QVector<Bus::Model> &modelInfos);

    void SignalNewSession(const QString &title,
                          const QString &content,
                          const QString &model);
    void SignalNewSessionResp(const int32_t       errorCode,
                              const Bus::Session &session);

    void SignalQuery(const int64_t  sessionId,
                     const QString &query,
                     const QString &model);
    void SignalQueryResp(const int      errorCode,
                         const int64_t  sessionId,
                         const QString &content);

  private:
    explicit Bus(QObject *parent = nullptr)
        : QObject(parent) {};
    ~Bus() {};
};

#endif