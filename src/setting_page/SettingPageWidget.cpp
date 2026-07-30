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

void SettingPageWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if(event->type() == QEvent::LanguageChange)
    {
        qDebug() << "SettingPageWidget language change event received.";
        ui->retranslateUi(this);
        _retranslate();
    }
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

        default:
            break;
    }
}

void SettingPageWidget::_slotLoginResp(const int      errorCode,
                                       const int64_t  user_id,
                                       const QString &auth,
                                       const QString &account,
                                       const QString &lastLoginTime,
                                       const bool     isForceUpdate)
{
    qDebug() << "SettingPageWidget::_slotLoginResp enter";
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
    ui->tabWidget->setTabText(0, tr("History Settings"));
    ui->tabWidget->setTabText(1, tr("Model Settings"));
    ui->tabWidget->setTabText(2, tr("Plugin Settings"));
    ui->tabWidget->setTabText(3, tr("Memory Settings"));
    ui->tabWidget->setTabText(4, tr("Network Settings"));
    ui->tabWidget->setTabText(5, tr("Hardware Settings"));
    ui->tabWidget->setTabText(6, tr("Version Info"));
}