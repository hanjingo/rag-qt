#include <QTimer>
#include <QDateTime>
#include <QtDebug>
#include <QButtonGroup>

#include "FrameworkWidget.h"
#include "ui_FrameworkWidget.h"

FrameworkWidget *FrameworkWidget::m_stFrameworkWidgetInst = nullptr;

FrameworkWidget *FrameworkWidget::GetFrameworkWidgetInst()
{
    if(nullptr == m_stFrameworkWidgetInst)
        m_stFrameworkWidgetInst = new FrameworkWidget();

    return m_stFrameworkWidgetInst;
}

FrameworkWidget::FrameworkWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , ui(new Ui::FrameworkWidget)
    , m_pAppBarBtnGroup(new QButtonGroup(this))
    , m_pCtlBtnGroup(new QButtonGroup(this))
    , m_pTimer(new QTimer(this))
    , m_pProcManagerInst(ProcManager::GetProcManagerInst())
    , m_pHomePageWgtInst(HomePageWidget::GetMainHomePageInst())
    , m_pSettingPageWgtInst(SettingPageWidget::GetMainSettingPageInst())
    , m_pAudioPageWgtInst(AudioPageWidget::GetAudioPageWidgetInst())
    , m_pImagePageWgtInst(ImagePageWidget::GetImagePageWidgetInst())
    , m_pTextPageWgtInst(TextPageWidget::GetTextPageWidgetInst())
    , m_pVideoPageWgtInst(VideoPageWidget::GetVideoPageWidgetInst())
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    m_pProcManagerInst->init();

    _initAppBar();
    _initControlBar();
    _initStackedWidget();
    _initConnections();
    _initTimer();
}

FrameworkWidget::~FrameworkWidget()
{
    m_pProcManagerInst->destroy();
    delete m_pProcManagerInst;

    delete m_pHomePageWgtInst;
    delete m_pTextPageWgtInst;
    delete m_pImagePageWgtInst;
    delete m_pAudioPageWgtInst;
    delete m_pVideoPageWgtInst;
    delete m_pSettingPageWgtInst;
    delete m_pAppBarBtnGroup;
    delete m_pCtlBtnGroup;
    delete m_pTimer;
    delete ui;
}

void FrameworkWidget::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void FrameworkWidget::_slotBtnExitClicked()
{
    qDebug() << "Exit button clicked.";
    m_pProcManagerInst->destroy();
    this->close();
}

void FrameworkWidget::_slotAppBarBtnGroupClicked(int id)
{
    qDebug() << "App bar button clicked, id:" << id;
    m_pAppBarBtnGroup->buttons().at(id)->setChecked(true);
    switch(id)
    {
        case 0:
            ui->stackedWidget->setCurrentWidget(m_pHomePageWgtInst);
            break;
        case 1:
            ui->stackedWidget->setCurrentWidget(m_pTextPageWgtInst);
            break;
        case 2:
            ui->stackedWidget->setCurrentWidget(m_pImagePageWgtInst);
            break;
        case 3:
            ui->stackedWidget->setCurrentWidget(m_pAudioPageWgtInst);
            break;
        case 4:
            ui->stackedWidget->setCurrentWidget(m_pVideoPageWgtInst);
            break;
        case 5:
            ui->stackedWidget->setCurrentWidget(m_pSettingPageWgtInst);
            break;
        default:
            break;
    }
}

void FrameworkWidget::_slotUpdateRealTime()
{
    // qDebug() << "Updating real time...";
    auto curr = QDateTime::currentDateTime();
    ui->lblTime->setText(curr.toString("hh:mm:ss"));
    ui->lblDate->setText(curr.toString("yyyy-MM-dd"));
}

void FrameworkWidget::_initAppBar()
{
    m_pAppBarBtnGroup->addButton(ui->btnHome, 0);
    m_pAppBarBtnGroup->addButton(ui->btnText, 1);
    m_pAppBarBtnGroup->addButton(ui->btnImage, 2);
    m_pAppBarBtnGroup->addButton(ui->btnAudio, 3);
    m_pAppBarBtnGroup->addButton(ui->btnVideo, 4);
    m_pAppBarBtnGroup->addButton(ui->btnSetting, 5);
}

void FrameworkWidget::_initControlBar()
{
    m_pCtlBtnGroup->addButton(ui->btnExit, 0);
}

void FrameworkWidget::_initStackedWidget()
{
    ui->stackedWidget->addWidget(m_pHomePageWgtInst);
    ui->stackedWidget->addWidget(m_pTextPageWgtInst);
    ui->stackedWidget->addWidget(m_pImagePageWgtInst);
    ui->stackedWidget->addWidget(m_pAudioPageWgtInst);
    ui->stackedWidget->addWidget(m_pVideoPageWgtInst);
    ui->stackedWidget->addWidget(m_pSettingPageWgtInst);

    ui->stackedWidget->setCurrentWidget(m_pHomePageWgtInst);
}

void FrameworkWidget::_initConnections()
{
    connect(ui->btnExit,
            &QPushButton::clicked,
            this,
            &FrameworkWidget::_slotBtnExitClicked);

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slotUpdateRealTime()));

    connect(m_pAppBarBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &FrameworkWidget::_slotAppBarBtnGroupClicked);
}

void FrameworkWidget::_initTimer()
{
    m_pTimer->start(1000);
}