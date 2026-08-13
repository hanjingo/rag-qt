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

void Account::clear()
{
    m_id   = -1;
    m_auth = "";

    ui->lblAccount->setText("");
}

void Account::setId(int64_t id)
{
    m_id = id;
}

void Account::setName(const QString &name)
{
    m_name = name;
    ui->lblAccount->setText(name);
}

void Account::setAuth(const QString &auth)
{
    m_auth = auth;
}

void Account::setLastLoginTime(const QString &lastLoginTime)
{
    m_lastLoginTime = lastLoginTime;
    ui->lblLastLoginTime->setText(lastLoginTime);
}