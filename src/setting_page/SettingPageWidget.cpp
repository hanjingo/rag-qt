#include <QtMath>
#include <QPainter>
#include <QAction>

#include "SettingPageWidget.h"
#include "ui_SettingPageWidget.h"

#include "StyleMgr.h"

#include "SettingPageNetwork.h"
#include "SettingPageHistory.h"
#include "SettingPageSkill.h"
#include "SettingPageSync.h"
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

    ui->tabWidget->setStyleSheet(StyleMgr::ParseFile(":/styles/tab_widget"));

    ui->tabWidget->addTab(SettingPageHistory::Instance(),
                          tr("History Settings"));
    ui->tabWidget->addTab(SettingPageModel::Instance(), tr("Model Settings"));
    ui->tabWidget->addTab(SettingPageSkill::Instance(), tr("Skill Settings"));
    ui->tabWidget->addTab(SettingPageSync::Instance(), tr("Sync Settings"));
    ui->tabWidget->addTab(SettingPageNetwork::Instance(),
                          tr("Network Settings"));
    ui->tabWidget->addTab(SettingPageHardware::Instance(),
                          tr("Hardware Settings"));
    ui->tabWidget->addTab(SettingPageVersion::Instance(), tr("Version Info"));

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
            SettingPageHistory::Instance();
            break;
        case 1:
            SettingPageModel::Instance();
            break;
        case 2:
            SettingPageSkill::Instance();
            break;
        case 3:
            SettingPageSync::Instance();
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