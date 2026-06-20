#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QDialog>
#include <QString>

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

    void Clear();
    bool IsValid() const { return m_id > 0 && !m_auth.isEmpty(); }

    void SetId(int32_t id);
    void SetPrivilege(int32_t privilege);
    void SetName(const QString &name);
    void SetAuth(const QString &auth);
    void SetLastLoginTime(const QString &lastLoginTime);

    int32_t Id() const { return m_id; }
    QString Auth() const { return m_auth; }

  private:
    Ui::AccountDialog *ui;

    int32_t m_id;
    int32_t m_privilege;

    QString m_name;
    QString m_auth;
    QString m_lastLoginTime;
};

#endif