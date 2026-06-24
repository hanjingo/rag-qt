#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QButtonGroup>

#include "SettingPageNetwork.h"
#include "ui_SettingPageNetwork.h"

SettingPageNetwork *SettingPageNetwork::m_stSettingPageNetworkInst = nullptr;

SettingPageNetwork *SettingPageNetwork::GetSettingPageNetworkInst()
{
    if(nullptr == m_stSettingPageNetworkInst)
    {
        m_stSettingPageNetworkInst = new SettingPageNetwork();
    }

    return m_stSettingPageNetworkInst;
}

SettingPageNetwork::SettingPageNetwork(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageNetwork)
{
    ui->setupUi(this);
}

SettingPageNetwork::~SettingPageNetwork()
{
    delete ui;
}

void SettingPageNetwork::_initUI()
{
}

void SettingPageNetwork::_retranslate()
{
}

void SettingPageNetwork::_initConnections()
{
}