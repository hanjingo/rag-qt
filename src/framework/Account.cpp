#include "Account.h"
#include "ui_Account.h"

Account *Account::m_stAccountInst = nullptr;
Account *Account::Instance()
{
    if(nullptr == m_stAccountInst)
        m_stAccountInst = new Account();

    return m_stAccountInst;
}

Account::Account(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AccountDialog)
    , m_id(-1)
    , m_privilege(PrivilegeType::Normal)
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
    m_privilege = static_cast<PrivilegeType>(privilege);
    switch(m_privilege)
    {
        case PrivilegeType::Normal:
            ui->lblPrivilege->setText(tr("Normal"));
            break;
        case PrivilegeType::Admin:
            ui->lblPrivilege->setText(tr("Admin"));
            break;
        default:
            ui->lblPrivilege->setText(tr("Unknown"));
            break;
    }
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