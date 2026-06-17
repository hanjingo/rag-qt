#include "Account.h"

Account::Account(QObject *parent)
    : QObject(parent)
    , m_id(-1)
    , m_auth("")
{
}

Account::~Account()
{
}