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

SettingPageWidget::SettingPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageWidget)
{
    ui->setupUi(this);

    _initConnections();
    _initUI();
    _retranslate();
}

SettingPageWidget::~SettingPageWidget()
{
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
            SettingPageHistory::instance();
            break;
        case 1:
            SettingPageModel::instance();
            break;
        case 2:
            SettingPagePlugin::instance();
            break;
        case 3:
            SettingPageMemory::instance();
            break;
        case 4:
            SettingPageNetwork::instance();
            break;
        case 5:
            SettingPageHardware::instance();
            break;
        case 6:
            SettingPageVersion::instance();
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

    ui->tabWidget->addTab(SettingPageHistory::instance(),
                          tr("History Settings"));
    ui->tabWidget->addTab(SettingPageModel::instance(), tr("Model Settings"));
    ui->tabWidget->addTab(SettingPagePlugin::instance(), tr("Plugin Settings"));
    ui->tabWidget->addTab(SettingPageMemory::instance(), tr("Memory Settings"));
    ui->tabWidget->addTab(SettingPageNetwork::instance(),
                          tr("Network Settings"));
    ui->tabWidget->addTab(SettingPageHardware::instance(),
                          tr("Hardware Settings"));
    ui->tabWidget->addTab(SettingPageVersion::instance(), tr("Version Info"));
}

void SettingPageWidget::_initConnections()
{
    connect(ui->tabWidget,
            SIGNAL(currentChanged(int)),
            this,
            SLOT(_slotTabCurrentChanged(int)));

    connect(GrpcClient::instance(),
            &GrpcClient::signalLoginResp,
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