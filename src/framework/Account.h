#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QDialog>
#include <QString>
#include <QPointer>

namespace Ui
{
class AccountDialog;
}

class Account : public QDialog
{
    Q_OBJECT

  public:
    explicit Account(QWidget *parent = nullptr);
    ~Account();

    static QPointer<Account> instance()
    {
        static QPointer<Account> inst = new Account();
        return inst;
    }

    void clear();
    bool isValid() const { return m_id > 0 && !m_auth.isEmpty(); }

    void setId(int32_t id);
    void setName(const QString &name);
    void setAuth(const QString &auth);
    void setLastLoginTime(const QString &lastLoginTime);

    int32_t id() const { return m_id; }
    QString name() const { return m_name; }
    QString auth() const { return m_auth; }
    QString lastLoginTime() const { return m_lastLoginTime; }

  private:
    Ui::AccountDialog *ui;

    int32_t m_id;
    QString m_name;
    QString m_auth;
    QString m_lastLoginTime;
};

#endif