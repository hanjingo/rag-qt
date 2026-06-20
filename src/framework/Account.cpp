#include "Account.h"
#include "ui_Account.h"

Account::Account(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AccountDialog)
    , m_id(-1)
    , m_auth("")
{
    ui->setupUi(this);
}

Account::~Account()
{
    delete ui;
}

void Account::Clear()
{
    m_id   = -1;
    m_auth = "";

    ui->lblAccount->setText("");
    ui->lblPrivilege->setText("");
}

void Account::SetId(int32_t id)
{
    m_id = id;
}

void Account::SetPrivilege(int32_t privilege)
{
    m_privilege = privilege;
    ui->lblPrivilege->setText(QString::number(privilege));
}

void Account::SetName(const QString &name)
{
    m_name = name;
    ui->lblAccount->setText(name);
}

void Account::SetAuth(const QString &auth)
{
    m_auth = auth;
}

void Account::SetLastLoginTime(const QString &lastLoginTime)
{
    m_lastLoginTime = lastLoginTime;
    ui->lblLastLoginTime->setText(lastLoginTime);
}