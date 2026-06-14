#include <QTimer>
#include <QDateTime>
#include <QtDebug>
#include <QButtonGroup>
#include <QMouseEvent>
#include <QCursor>
#include <QDir>
#include <QMessageBox>
#include <QMenu>
#include <QScreen>
#include <QGuiApplication>

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
    , m_pTranslator(new QTranslator(this))
    , m_pGrpcClient(GrpcClient::GetGrpcClientInst())
    , m_pHomePageWgtInst(HomePageWidget::GetMainHomePageInst())
    , m_pSettingPageWgtInst(SettingPageWidget::GetMainSettingPageInst())
    , m_pAudioPageWgtInst(AudioPageWidget::GetAudioPageWidgetInst())
    , m_pImagePageWgtInst(ImagePageWidget::GetImagePageWidgetInst())
    , m_pTextPageWgtInst(TextPageWidget::GetTextPageWidgetInst())
    , m_pVideoPageWgtInst(VideoPageWidget::GetVideoPageWidgetInst())
    , m_pToolPageWgtInst(ToolPageWidget::GetToolPageWidgetInst())
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    _initProcMgr();

    _initConnections();

    _initAppBar();
    _initControlBar();
    _initStackedWidget();
    _initTimer();
    _initServer();
    _initLanguage();
}

FrameworkWidget::~FrameworkWidget()
{
    m_pProcManagerInst->destroy();
    delete m_pProcManagerInst;

    delete m_pGrpcClient;

    delete m_pHomePageWgtInst;
    delete m_pTextPageWgtInst;
    delete m_pImagePageWgtInst;
    delete m_pAudioPageWgtInst;
    delete m_pVideoPageWgtInst;
    delete m_pSettingPageWgtInst;
    delete m_pToolPageWgtInst;
    delete m_pAppBarBtnGroup;
    delete m_pCtlBtnGroup;
    delete m_pTimer;
    delete ui;
}

void FrameworkWidget::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void FrameworkWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        const int region =
            _hitTestResizeRegion(event->globalPosition().toPoint());
        if(region != ResizeNone)
        {
            m_isResizing     = true;
            m_resizeRegion   = region;
            m_pressGlobalPos = event->globalPosition().toPoint();
            m_pressGeometry  = geometry();
            event->accept();
            return;
        }
    }

    QWidget::mousePressEvent(event);
}

void FrameworkWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isResizing)
    {
        QRect        newGeom = m_pressGeometry;
        const QPoint delta =
            event->globalPosition().toPoint() - m_pressGlobalPos;

        if(m_resizeRegion & ResizeLeft)
            newGeom.setLeft(newGeom.left() + delta.x());
        if(m_resizeRegion & ResizeRight)
            newGeom.setRight(newGeom.right() + delta.x());
        if(m_resizeRegion & ResizeTop)
            newGeom.setTop(newGeom.top() + delta.y());
        if(m_resizeRegion & ResizeBottom)
            newGeom.setBottom(newGeom.bottom() + delta.y());

        const QSize min = minimumSize();
        if(newGeom.width() < min.width())
        {
            if(m_resizeRegion & ResizeLeft)
                newGeom.setLeft(newGeom.right() - min.width() + 1);
            else
                newGeom.setRight(newGeom.left() + min.width() - 1);
        }
        if(newGeom.height() < min.height())
        {
            if(m_resizeRegion & ResizeTop)
                newGeom.setTop(newGeom.bottom() - min.height() + 1);
            else
                newGeom.setBottom(newGeom.top() + min.height() - 1);
        }

        setGeometry(newGeom);
        event->accept();
        return;
    }

    _updateResizeCursor(
        _hitTestResizeRegion(event->globalPosition().toPoint()));
    QWidget::mouseMoveEvent(event);
}

void FrameworkWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_isResizing)
    {
        m_isResizing   = false;
        m_resizeRegion = ResizeNone;
        _updateResizeCursor(
            _hitTestResizeRegion(event->globalPosition().toPoint()));
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void FrameworkWidget::leaveEvent(QEvent *event)
{
    if(!m_isResizing)
        unsetCursor();

    QWidget::leaveEvent(event);
}

void FrameworkWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void FrameworkWidget::_slotCtlBtnGroupClicked(int id)
{
    qDebug() << "Control bar button clicked, id:" << id;
    m_pCtlBtnGroup->buttons().at(id)->setChecked(true);
    switch(id)
    {
        case 0: // Minimize
            qDebug() << "Minimize button clicked.";
            _minimizeWindow();
            break;
        case 1: // Select Screen
            qDebug() << "Select Screen button clicked.";
            _selectScreen();
            break;
        case 2: // Alarm
            qDebug() << "Alarm button clicked.";
            _showAlarmDialog();
            break;
        case 3: // Exit
            qDebug() << "Exit button clicked.";
            _exit();
            break;
        default:
            break;
    }
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
        case 6:
            ui->stackedWidget->setCurrentWidget(m_pToolPageWgtInst);
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

void FrameworkWidget::_slotGrpcConnected(const QString &address)
{
    qDebug() << "FrameworkWidget connected to gRPC server at " << address;
    ui->lblNetStatus->setText(tr("Connected"));
}

void FrameworkWidget::_slotGrpcConnectFailed(const QString &address)
{
    qDebug() << "Fail to Connect gRPC server at " << address;
    ui->lblNetStatus->setText(tr("Disconnected"));
}

void FrameworkWidget::_slotQueryResp(const QString &resp)
{
    qDebug() << "Query Response with " << resp;
}

void FrameworkWidget::_slotComboLangCurrentChanged(int iIndex)
{
    qDebug() << "Language combo box current index changed: " << iIndex;
    QApplication::removeTranslator(m_pTranslator);
    switch(iIndex)
    {
        case 0: // Chinese
            qDebug() << "Switching to Chinese.";
            m_pTranslator->load(":/languages/zh_CN");
            break;
        case 1: // English
            qDebug() << "Switching to English.";
            m_pTranslator->load(":/languages/en_UK");
            break;
        case 2: // German
            qDebug() << "Switching to German.";
            m_pTranslator->load(":/languages/de_DE");
            break;
        default:
            qDebug() << "Unknown language index: " << iIndex;
            return;
    }
    QApplication::installTranslator(m_pTranslator);
}

void FrameworkWidget::_initProcMgr()
{
    m_pProcManagerInst->init();
}

int FrameworkWidget::_hitTestResizeRegion(const QPoint &globalPos) const
{
    constexpr int kBorder = 8;
    const QRect   rect    = frameGeometry();

    int region = ResizeNone;
    if(globalPos.x() >= rect.left() && globalPos.x() <= rect.left() + kBorder)
        region |= ResizeLeft;
    else if(globalPos.x() <= rect.right()
            && globalPos.x() >= rect.right() - kBorder)
        region |= ResizeRight;

    if(globalPos.y() >= rect.top() && globalPos.y() <= rect.top() + kBorder)
        region |= ResizeTop;
    else if(globalPos.y() <= rect.bottom()
            && globalPos.y() >= rect.bottom() - kBorder)
        region |= ResizeBottom;

    return region;
}

void FrameworkWidget::_updateResizeCursor(int region)
{
    switch(region)
    {
        case ResizeLeft:
        case ResizeRight:
            setCursor(Qt::SizeHorCursor);
            break;
        case ResizeTop:
        case ResizeBottom:
            setCursor(Qt::SizeVerCursor);
            break;
        case ResizeTop | ResizeLeft:
        case ResizeBottom | ResizeRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case ResizeTop | ResizeRight:
        case ResizeBottom | ResizeLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
        default:
            unsetCursor();
            break;
    }
}

void FrameworkWidget::_initAppBar()
{
    ui->btnHome->setText(tr("Home"));
    ui->btnText->setText(tr("Text"));
    ui->btnImage->setText(tr("Image"));
    ui->btnAudio->setText(tr("Audio"));
    ui->btnVideo->setText(tr("Video"));
    ui->btnSetting->setText(tr("Setting"));
    ui->btnTool->setText(tr("Tool"));

    m_pAppBarBtnGroup->addButton(ui->btnHome, 0);
    m_pAppBarBtnGroup->addButton(ui->btnText, 1);
    m_pAppBarBtnGroup->addButton(ui->btnImage, 2);
    m_pAppBarBtnGroup->addButton(ui->btnAudio, 3);
    m_pAppBarBtnGroup->addButton(ui->btnVideo, 4);
    m_pAppBarBtnGroup->addButton(ui->btnSetting, 5);
    m_pAppBarBtnGroup->addButton(ui->btnTool, 6);
}

void FrameworkWidget::_initControlBar()
{
    m_pCtlBtnGroup->addButton(ui->btnMinimize, 0);
    m_pCtlBtnGroup->addButton(ui->btnSelectScreen, 1);
    m_pCtlBtnGroup->addButton(ui->btnAlarm, 2);
    m_pCtlBtnGroup->addButton(ui->btnExit, 3);
}

void FrameworkWidget::_initStackedWidget()
{
    ui->stackedWidget->addWidget(m_pHomePageWgtInst);
    ui->stackedWidget->addWidget(m_pTextPageWgtInst);
    ui->stackedWidget->addWidget(m_pImagePageWgtInst);
    ui->stackedWidget->addWidget(m_pAudioPageWgtInst);
    ui->stackedWidget->addWidget(m_pVideoPageWgtInst);
    ui->stackedWidget->addWidget(m_pSettingPageWgtInst);
    ui->stackedWidget->addWidget(m_pToolPageWgtInst);

    ui->stackedWidget->setCurrentWidget(m_pHomePageWgtInst);
}

void FrameworkWidget::_initConnections()
{
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slotUpdateRealTime()));

    connect(m_pCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &FrameworkWidget::_slotCtlBtnGroupClicked);

    connect(m_pAppBarBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &FrameworkWidget::_slotAppBarBtnGroupClicked);

    connect(m_pGrpcClient,
            &GrpcClient::SignalGrpcConnected,
            this,
            &FrameworkWidget::_slotGrpcConnected);

    connect(m_pGrpcClient,
            &GrpcClient::SignalGrpcConnectFailed,
            this,
            &FrameworkWidget::_slotGrpcConnectFailed);

    connect(m_pGrpcClient,
            &GrpcClient::SignalQueryResp,
            this,
            &FrameworkWidget::_slotQueryResp);

    connect(ui->comboLang,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(_slotComboLangCurrentChanged(int)));
}

void FrameworkWidget::_initTimer()
{
    m_pTimer->start(1000);
}

void FrameworkWidget::_initServer()
{
    m_pGrpcClient->connect("127.0.0.1:50051");
}

void FrameworkWidget::_initLanguage()
{
    ui->comboLang->setCurrentIndex(1); // Default to English
}

void FrameworkWidget::_minimizeWindow()
{
    this->showMinimized();
}

void FrameworkWidget::_selectScreen()
{
    this->showMinimized(); // Minimize the window to avoid it being captured in the screenshot of the current screen

    // Get the list of available screens
    QList<QScreen *> screens = QGuiApplication::screens();

    // Create a menu to list the screens
    QMenu screenMenu;
    for(int i = 0; i < screens.size(); ++i)
    {
        QAction *action =
            screenMenu.addAction(QString("Screen %1: %2x%3")
                                     .arg(i + 1)
                                     .arg(screens[i]->size().width())
                                     .arg(screens[i]->size().height()));
        action->setData(i); // Store the screen index in the action's data
    }

    // Show the menu at the cursor position
    QAction *selectedAction = screenMenu.exec(QCursor::pos());
    if(selectedAction)
    {
        int screenIndex = selectedAction->data().toInt();
        if(screenIndex >= 0 && screenIndex < screens.size())
        {
            // Move the window to the selected screen
            QScreen *selectedScreen = screens[screenIndex];
            QRect    screenGeometry = selectedScreen->geometry();
            move(screenGeometry.topLeft());
        }
    }
}

void FrameworkWidget::_showAlarmDialog()
{
    QMessageBox::information(this, tr("Alarm"), tr("This is an alarm dialog!"));
}

void FrameworkWidget::_exit()
{
    m_pProcManagerInst->destroy();
    this->close();
}