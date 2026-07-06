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
    enum class PrivilegeType
    {
        Normal    = 0,
        Developer = 1,
        Admin     = 2
    };

  public:
    explicit Account(QWidget *parent = nullptr);
    ~Account();

    static Account *Instance();

    void Clear();
    bool IsValid() const { return m_id > 0 && !m_auth.isEmpty(); }

    void SetId(int32_t id);
    void SetPrivilege(int32_t privilege);
    void SetName(const QString &name);
    void SetAuth(const QString &auth);
    void SetLastLoginTime(const QString &lastLoginTime);

    int32_t       Id() const { return m_id; }
    PrivilegeType Privilege() const { return m_privilege; }
    QString       Name() const { return m_name; }
    QString       Auth() const { return m_auth; }
    QString       LastLoginTime() const { return m_lastLoginTime; }

  private:
    Ui::AccountDialog *ui;
    static Account    *m_stAccountInst;

    int32_t       m_id;
    PrivilegeType m_privilege;

    QString m_name;
    QString m_auth;
    QString m_lastLoginTime;
};

#endif