#include <QtMath>
#include <QPainter>
#include <QAction>

#include "SettingPageWidget.h"
#include "ui_SettingPageWidget.h"

#include "StyleMgr.h"
#include "GrpcClient.h"
#include "Account.h"

#include "SettingPageNetwork.h"
#include "SettingPageHistory.h"
#include "SettingPagePlugin.h"
#include "SettingPageMemory.h"
#include "SettingPageVersion.h"
#include "SettingPageModel.h"
#include "SettingPageHardware.h"
#include "SettingPageDev.h"

SettingPageWidget *SettingPageWidget::m_stMainSettingPageInst = nullptr;
SettingPageWidget *SettingPageWidget::Instance()
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

    _initUI();
    _initConnections();
    _retranslate();
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
            SettingPageHistory::Instance();
            break;
        case 1:
            SettingPageModel::Instance();
            break;
        case 2:
            SettingPagePlugin::Instance();
            break;
        case 3:
            SettingPageMemory::Instance();
            break;
        case 4:
            SettingPageNetwork::Instance();
            break;
        case 5:
            SettingPageHardware::Instance();
            break;
        case 6:
            SettingPageVersion::Instance();
            break;
        case 7:
            SettingPageDev::Instance();
            break;

        default:
            break;
    }
}

void SettingPageWidget::_slotLoginResp(const int      errorCode,
                                       const int64_t  user_id,
                                       const QString &auth,
                                       const int32_t  privilege,
                                       const QString &account,
                                       const QString &lastLoginTime)
{
    qDebug() << "SettingPageWidget::_slotLoginResp enter";
    if(privilege < static_cast<int>(Account::PrivilegeType::Developer))
    {
        ui->tabWidget->setTabEnabled(7, false);
    }
}

void SettingPageWidget::_initUI()
{
    ui->tabWidget->setStyleSheet(StyleMgr::ParseFile(":/styles/tab_widget"));

    ui->tabWidget->addTab(SettingPageHistory::Instance(),
                          tr("History Settings"));
    ui->tabWidget->addTab(SettingPageModel::Instance(), tr("Model Settings"));
    ui->tabWidget->addTab(SettingPagePlugin::Instance(), tr("Plugin Settings"));
    ui->tabWidget->addTab(SettingPageMemory::Instance(), tr("Memory Settings"));
    ui->tabWidget->addTab(SettingPageNetwork::Instance(),
                          tr("Network Settings"));
    ui->tabWidget->addTab(SettingPageHardware::Instance(),
                          tr("Hardware Settings"));
    ui->tabWidget->addTab(SettingPageVersion::Instance(), tr("Version Info"));
    ui->tabWidget->addTab(SettingPageDev::Instance(), tr("Developer Settings"));
}

void SettingPageWidget::_initConnections()
{
    connect(ui->tabWidget,
            SIGNAL(currentChanged(int)),
            this,
            SLOT(_slotTabCurrentChanged(int)));

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalLoginResp,
            this,
            &SettingPageWidget::_slotLoginResp);
}

void SettingPageWidget::_retranslate()
{
}