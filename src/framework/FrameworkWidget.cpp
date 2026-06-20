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
#include <QDirIterator>
#include <QFileInfo>
#include <QFileInfoList>
#include <QStringList>
#include <QGuiApplication>

#include "PluginInterface.h"
#include "FrameworkWidget.h"
#include "ui_FrameworkWidget.h"
#include "Error.h"

FrameworkWidget *FrameworkWidget::m_stFrameworkWidgetInst = nullptr;

FrameworkWidget *FrameworkWidget::Instance()
{
    if(nullptr == m_stFrameworkWidgetInst)
        m_stFrameworkWidgetInst = new FrameworkWidget();

    return m_stFrameworkWidgetInst;
}

FrameworkWidget::FrameworkWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , ui(new Ui::FrameworkWidget)
    , m_pCtlBtnGroup(new QButtonGroup(this))
    , m_pTimer(new QTimer(this))
    , m_pProcManagerInst(ProcManager::Instance())
    , m_pPluginMgrInst(PluginMgr::Instance())
    , m_pTranslator(new QTranslator(this))
    , m_pGrpcClient(GrpcClient::Instance())
    , m_pBus(Bus::Instance())
    , m_pAccount(new Account(this))
    , m_pLoginWgtInst(LoginWidget::Instance())
    , m_pHomePageWgtInst(HomePageWidget::GetMainHomePageInst())
    , m_pSettingPageWgtInst(SettingPageWidget::GetMainSettingPageInst())
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

    _initPluginMgr();
}

FrameworkWidget::~FrameworkWidget()
{
    m_pProcManagerInst->destroy();
    delete m_pProcManagerInst;

    delete m_pGrpcClient;

    delete m_pHomePageWgtInst;
    delete m_pSettingPageWgtInst;
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

void FrameworkWidget::_slotLogin(const QString &username,
                                 const QString &password)
{
    GrpcClient::Instance()->Login(username, password);
}

void FrameworkWidget::_slotLoginResp(const int      errorCode,
                                     const int32_t  id,
                                     const QString &auth)
{
    qDebug() << "Login response received. Error code:" << errorCode
             << "ID:" << id << "Auth:" << auth;

    if(errorCode != ErrorCode::OK)
    {
        QMessageBox::critical(
            this,
            tr("Login Failed"),
            tr("Login failed with error code: %1").arg(errorCode));
        return;
    }

    m_pAccount->SetId(id);
    m_pAccount->SetAuth(auth);
    m_pLoginWgtInst->hide();
    this->show();

    // query history after connected
    GrpcClient::Instance()->GetSession(-1, id, auth, 50);

    // query skill info after connected
    GrpcClient::Instance()->GetSkillInfo();
    return;
}

void FrameworkWidget::_slotRegister(const QString &username,
                                    const QString &password)
{
    qDebug() << "Register attempt with username:" << username
             << "and password:" << password;
    GrpcClient::Instance()->RegAccount(username, password);
}

void FrameworkWidget::_slotRegisterResp(const int     errorCode,
                                        const int32_t user_id)
{
    qDebug() << "Register response received. Error code:" << errorCode
             << "User ID:" << user_id;

    if(errorCode != ErrorCode::OK)
    {
        QMessageBox::critical(
            this,
            tr("Register Failed"),
            tr("Register failed with error code: %1").arg(errorCode));
        return;
    }

    m_pAccount->SetId(user_id);
}

void FrameworkWidget::_slotLogout()
{
    qDebug() << "Logout signal received.";
    if(m_pAccount == nullptr || !m_pAccount->IsValid())
    {
        qDebug() << "Account instance is null or invalid. exit";
        _exit();
        return;
    }

    GrpcClient::Instance()->Logout(m_pAccount->Id(), m_pAccount->Auth());
}

void FrameworkWidget::_slotLogoutResp(const int errorCode, const int user_id)
{
    qDebug() << "Logout response received. Error code:" << errorCode
             << "User ID:" << user_id;

    if(errorCode != ErrorCode::OK)
    {
        QMessageBox::critical(
            nullptr,
            tr("Logout Failed"),
            tr("Logout failed with error code: %1").arg(errorCode));
    }

    m_pLoginWgtInst->close();
    _exit();
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
        case 3: // switch account
            qDebug() << "Switch account button clicked.";
            _switchAccount();
            break;
        case 4: // Exit
            qDebug() << "Exit button clicked.";
            _exit();
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

void FrameworkWidget::_slotPong()
{
    qDebug() << "Received Pong signal from Bus.";
}

void FrameworkWidget::_slotPluginLoaded(PluginInterface *plugin,
                                        const QString   &filePath)
{
    if(!plugin)
    {
        qDebug() << "Failed to load null plugin";
        return;
    }

    //qDebug() << "Plugin loaded: " << plugin->Name()
    //         << ", version: " << plugin->Version()
    //         << ", description: " << plugin->Description();
    auto wgt = plugin->Init(Bus::Instance());
    emit Bus::Instance() -> SignalPing();
    if(wgt)
    {
        // TODO sort icon position
        int index = ui->stackedWidget->count();
        // index         = (index < 0 || index > ui->stackedWidget->count())
        //                     ? ui->stackedWidget->count()
        //                     : index;

        QFileInfo fileInfo(filePath);
        auto iconPath = fileInfo.absoluteDir().absoluteFilePath(plugin->Icon());
        _addAppBarItem(plugin->Name(), iconPath, index);
        ui->stackedWidget->insertWidget(index, wgt);
    }
}

void FrameworkWidget::_initProcMgr()
{
    m_pProcManagerInst->init();
}

void FrameworkWidget::_initPluginMgr()
{
    // scan plugins in the "plugins" directory relative to the executable
    QDir dir = QDir(QCoreApplication::applicationDirPath()).filePath("plugins");
    if(!dir.exists())
    {
        // create the plugins directory if it doesn't exist
        if(!dir.mkpath("."))
        {
            qDebug() << "Failed to create plugins directory at "
                     << dir.absolutePath();
            return;
        }
    }

    // QStringList  filter{"*.dll", "*.so", "*.dylib"};
    // QDirIterator it(dir.absolutePath(),
    //                 filter,
    //                 QDir::Files,
    //                 QDirIterator::Subdirectories);
    // while(it.hasNext())
    // {
    //     it.next(); // skip the first empty file
    //     _addPlugin(it.fileInfo());
    // }

    auto pluginPaths = m_pPluginMgrInst->Search(
        dir.absolutePath(),
        [](const QJsonObject &metaData) -> bool {
            return metaData.contains("PluginId") && metaData.contains("Version")
                   && metaData.contains("Name")
                   && metaData.contains("Description")
                   && metaData.contains("Author")
                   && metaData.contains("Dependencies");
        });
    for(const QString &path : pluginPaths)
    {
        qDebug() << "Found plugin: " << path;
        m_pPluginMgrInst->Load(path);
    }
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
    const QSize appBarItemSize(75, 75);
    ui->listWidgetAppBar->setViewMode(QListView::IconMode);
    ui->listWidgetAppBar->setFlow(QListView::TopToBottom);
    ui->listWidgetAppBar->setMovement(QListView::Static);
    ui->listWidgetAppBar->setResizeMode(QListView::Adjust);
    ui->listWidgetAppBar->setWrapping(false);
    ui->listWidgetAppBar->setUniformItemSizes(true);
    ui->listWidgetAppBar->setGridSize(appBarItemSize);
    ui->listWidgetAppBar->setIconSize(QSize(40, 40));
    ui->listWidgetAppBar->setSpacing(6);
    ui->listWidgetAppBar->setTextElideMode(Qt::ElideNone);
    ui->listWidgetAppBar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidgetAppBar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidgetAppBar->setStyleSheet(
        StyleMgr::ParseFile(":/styles/slide_bar"));

    _addAppBarItem(tr("Home"), ":/icons/home", 0);
    _addAppBarItem(tr("Setting"), ":/icons/settings", 1);
    ui->listWidgetAppBar->setCurrentRow(0);
}

void FrameworkWidget::_initControlBar()
{
    m_pCtlBtnGroup->addButton(ui->btnMinimize, 0);
    m_pCtlBtnGroup->addButton(ui->btnSelectScreen, 1);
    m_pCtlBtnGroup->addButton(ui->btnAlarm, 2);
    m_pCtlBtnGroup->addButton(ui->btnSwitch, 3);
    m_pCtlBtnGroup->addButton(ui->btnExit, 4);

    ui->lblNotice->setText(tr("Welcome!!!"));
}

void FrameworkWidget::_initStackedWidget()
{
    ui->stackedWidget->addWidget(m_pHomePageWgtInst);
    ui->stackedWidget->addWidget(m_pSettingPageWgtInst);

    ui->stackedWidget->setCurrentIndex(0);
}

void FrameworkWidget::_initConnections()
{
    connect(m_pLoginWgtInst,
            &LoginWidget::SignalLogin,
            this,
            &FrameworkWidget::_slotLogin);

    connect(m_pLoginWgtInst,
            &LoginWidget::SignalRegister,
            this,
            &FrameworkWidget::_slotRegister);

    connect(m_pLoginWgtInst,
            &LoginWidget::SignalLogout,
            this,
            &FrameworkWidget::_slotLogout);

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slotUpdateRealTime()));

    connect(ui->listWidgetAppBar,
            &QListWidget::currentRowChanged,
            ui->stackedWidget,
            &QStackedWidget::setCurrentIndex);

    connect(m_pCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &FrameworkWidget::_slotCtlBtnGroupClicked);

    connect(m_pGrpcClient,
            &GrpcClient::SignalGrpcConnected,
            this,
            &FrameworkWidget::_slotGrpcConnected);

    connect(m_pGrpcClient,
            &GrpcClient::SignalGrpcConnectFailed,
            this,
            &FrameworkWidget::_slotGrpcConnectFailed);

    connect(m_pGrpcClient,
            &GrpcClient::SignalLoginResp,
            this,
            &FrameworkWidget::_slotLoginResp);

    connect(m_pGrpcClient,
            &GrpcClient::SignalRegAccountResp,
            this,
            &FrameworkWidget::_slotRegisterResp);

    connect(m_pGrpcClient,
            &GrpcClient::SignalLogoutResp,
            this,
            &FrameworkWidget::_slotLogoutResp);

    connect(ui->comboLang,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(_slotComboLangCurrentChanged(int)));

    connect(Bus::Instance(),
            &Bus::SignalPong,
            this,
            &FrameworkWidget::_slotPong);

    connect(m_pPluginMgrInst,
            &PluginMgr::SignalPluginLoaded,
            this,
            &FrameworkWidget::_slotPluginLoaded);
}

void FrameworkWidget::_initTimer()
{
    m_pTimer->start(1000);
}

void FrameworkWidget::_initServer()
{
    m_pGrpcClient->Connect("127.0.0.1:50051");
}

void FrameworkWidget::_initLanguage()
{
    ui->comboLang->setStyleSheet(StyleMgr::ParseFile(":/styles/combo_box"));
    ui->comboLang->setCurrentIndex(1); // Default to English
    ui->comboLang->setIconSize(QSize(24, 24));
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
    if(m_pLoginWgtInst)
        m_pLoginWgtInst->close();
}

void FrameworkWidget::_switchAccount()
{
    // clear account info
    if(m_pAccount)
        m_pAccount->Clear();

    m_pLoginWgtInst->show();
    this->hide();
}

void FrameworkWidget::_addAppBarItem(const QString &text,
                                     const QString &iconPath,
                                     int            index)
{
    QIcon icon;
    if(QFile::exists(iconPath))
    {
        qDebug() << "Adding app bar item: " << text
                 << " with icon: " << iconPath;
        icon = QIcon(iconPath);
    } else
    {
        icon = QIcon(":/icons/unknown");
        qDebug() << "Icon file does not exist: " << iconPath
                 << ", use default icon.";
    }

    QListWidgetItem *item = new QListWidgetItem(icon, text);
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    item->setSizeHint(QSize(72, 72));

    ui->listWidgetAppBar->insertItem(index, item);
}