#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QString>

class Account : public QObject
{
    Q_OBJECT
  public:
    explicit Account(QObject *parent = nullptr);
    ~Account();

    void Clear();
    bool IsValid() const { return m_id > 0 && !m_auth.isEmpty(); }

    void SetId(int32_t id) { m_id = id; }
    void SetAuth(const QString &auth) { m_auth = auth; }

    int32_t Id() const { return m_id; }
    QString Auth() const { return m_auth; }

  private:
    int32_t m_id;
    QString m_auth;
};

#endif