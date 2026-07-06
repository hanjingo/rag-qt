#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QButtonGroup>

#include "SettingPageDev.h"
#include "ui_SettingPageDev.h"

SettingPageDev *SettingPageDev::m_stSettingPageDevInst = nullptr;
SettingPageDev *SettingPageDev::Instance()
{
    if(nullptr == m_stSettingPageDevInst)
    {
        m_stSettingPageDevInst = new SettingPageDev();
    }

    return m_stSettingPageDevInst;
}

SettingPageDev::SettingPageDev(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageDev)
{
    ui->setupUi(this);
}

SettingPageDev::~SettingPageDev()
{
    delete ui;
}

void SettingPageDev::_initUI()
{
}

void SettingPageDev::_retranslate()
{
}

void SettingPageDev::_initConnections()
{
}