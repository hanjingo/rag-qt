#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QButtonGroup>

#include "SettingPageSync.h"
#include "ui_SettingPageSync.h"

SettingPageSync *SettingPageSync::m_stSettingPageSyncInst = nullptr;
SettingPageSync *SettingPageSync::Instance()
{
    if(nullptr == m_stSettingPageSyncInst)
    {
        m_stSettingPageSyncInst = new SettingPageSync();
    }

    return m_stSettingPageSyncInst;
}

SettingPageSync::SettingPageSync(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageSync)
{
    ui->setupUi(this);
}

SettingPageSync::~SettingPageSync()
{
    delete ui;
}

void SettingPageSync::_initUI()
{
}

void SettingPageSync::_retranslate()
{
}

void SettingPageSync::_initConnections()
{
}