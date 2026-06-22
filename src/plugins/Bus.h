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
    struct ModelInfo
    {
        QString hash;
        QString name;
        QString publisher;
        QString timestamp;
        QString addr;
        QString capabilities;
        int64_t contextSize;
        int32_t cost;
    };

  public:
    static Bus *Instance();
    static void Version(int8_t &major, int8_t &minor, int8_t &patch);

  signals:
    void SignalPong();
    void SignalPing();

    void SignalLanguageSwitch(const QString &lang);

    void SignalModelInfoUpdate(const QVector<Bus::ModelInfo> &modelInfos);

    void SignalNewSession(const QString &title,
                          const QString &content,
                          const QString &model);
    void SignalNewSessionResp(const int64_t  sessionId,
                              const QString &title,
                              const QString &answer);

    void SignalQuery(const int64_t  sessionId,
                     const QString &query,
                     const QString &model);
    void SignalQueryResp(const int64_t sessionId, const QString &resp);

  private:
    explicit Bus(QObject *parent = nullptr)
        : QObject(parent) {};
    ~Bus() {};
};

#endif