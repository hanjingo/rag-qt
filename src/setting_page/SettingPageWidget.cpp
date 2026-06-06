#include <QtMath>
#include <QPainter>
#include <QAction>

#include "SettingPageWidget.h"
#include "ui_SettingPageWidget.h"
#include "SettingPageNetwork.h"
#include "SettingPageHistory.h"

SettingPageWidget *SettingPageWidget::m_stMainSettingPageInst = nullptr;

SettingPageWidget *SettingPageWidget::GetMainSettingPageInst()
{
    if(nullptr == m_stMainSettingPageInst)
    {
        m_stMainSettingPageInst = new SettingPageWidget();
    }

    return m_stMainSettingPageInst;
}

SettingPageWidget::SettingPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageWidget)
{
    ui->setupUi(this);

    ui->tabWidget->addTab(SettingPageHistory::GetSettingPageHistoryInst(),
                          tr("History Settings"));
    ui->tabWidget->addTab(SettingPageNetwork::GetSettingPageNetworkInst(),
                          tr("Network Settings"));

    connect(ui->tabWidget,
            SIGNAL(currentChanged(int)),
            this,
            SLOT(_slotTabCurrentChanged(int)));
}

SettingPageWidget::~SettingPageWidget()
{
    delete ui;
}

void SettingPageWidget::_slotTabCurrentChanged(int iIndex)
{
    switch(iIndex)
    {
        case 0:
            SettingPageHistory::GetSettingPageHistoryInst();
            break;

        case 1:
            SettingPageNetwork::GetSettingPageNetworkInst();
            break;

        default:
            break;
    }
}
