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

#include <iostream>

#include <libqt/util/screencapture.h>

#include "Bus.h"
#include "PluginInterface.h"
#include "MainPageWidget.h"
#include "ui_Account.h"
#include "ui_MainPageWidget.h"
#include "Error.h"
#include "SettingPageModel.h"
#include "SettingPageMemory.h"
#include "System.h"
#include "SettingPageNetwork.h"
#include "AudioMgr.h"
#include "Config.h"
#include "Upgrade.h"
#include "Global.h"

MainPageWidget::MainPageWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , ui(new Ui::MainPageWidget)
    , m_pCtlBtnGroup(new QButtonGroup(this))
    , m_pTimer(new QTimer(this))
    , m_pProcManagerInst(ProcManager::instance())
    , m_pPluginMgrInst(PluginMgr::instance())
    , m_pTranslator(new QTranslator(this))
    , m_pGrpcClient(GrpcClient::instance())
    , m_pBusAdapter(BusAdapter::instance())
    , m_pAccount(Account::instance())
    , m_pLoginWgtInst(LoginWidget::instance())
    , m_pHomePageWgtInst(HomePageWidget::instance())
    , m_pSettingPageWgtInst(SettingPageWidget::instance())
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    setMouseTracking(true);

    QCoreApplication::instance()->installEventFilter(this);

    _initConnections();
    _initAppBar();
    _initControlBar();
    _initStackedWidget();
    _initConnStatus();
    _initTimer();
    _initPluginMgr();
    _initLanguage();
}

MainPageWidget::~MainPageWidget()
{
}

void MainPageWidget::InitCore()
{
    _initProcMgr();
}

void MainPageWidget::InitNetwork()
{
    _initServer();
}

QString MainPageWidget::ReadAllStandardOutput()
{
    return m_pProcManagerInst->readAllStandardOutput();
}

bool MainPageWidget::IsConnectedToCoreService()
{
    return m_pGrpcClient ? m_pGrpcClient->IsConnected() : false;
}

void MainPageWidget::closeEvent(QCloseEvent *event)
{
    // stop all plugins before closing the application
    PluginMgr::instance()->Clear();

    // process all pending events to ensure that the plugin shutdown signals are processed
    QCoreApplication::processEvents();

    event->accept();
}

void MainPageWidget::mousePressEvent(QMouseEvent *event)
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
        } else
        {
            QPoint menuBarPos =
                ui->menuBar->mapFromGlobal(event->globalPosition().toPoint());
            if(ui->menuBar->rect().contains(menuBarPos))
            {
                m_isDragging     = true;
                m_pressGlobalPos = event->globalPosition().toPoint();
                m_pressGeometry  = geometry();
                event->accept();
                return;
            }
        }
    }

    QWidget::mousePressEvent(event);
}

void MainPageWidget::mouseMoveEvent(QMouseEvent *event)
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

        //const QSize min = minimumSize();
        const QSize min = minimumSize().expandedTo(minimumSizeHint());
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

    if(m_isDragging)
    {
        const QPoint delta =
            event->globalPosition().toPoint() - m_pressGlobalPos;
        QRect newGeom = m_pressGeometry;
        newGeom.moveTopLeft(newGeom.topLeft() + delta);
        setGeometry(newGeom);
        event->accept();
        return;
    }

    int region = _hitTestResizeRegion(event->globalPosition().toPoint());
    _updateResizeCursor(region);
    QWidget::mouseMoveEvent(event);
}

void MainPageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(m_isResizing)
        {
            m_isResizing   = false;
            m_resizeRegion = ResizeNone;
            _updateResizeCursor(
                _hitTestResizeRegion(event->globalPosition().toPoint()));
            event->accept();
            return;
        }

        if(m_isDragging)
        {
            m_isDragging = false;
            event->accept();
            return;
        }
    }

    QWidget::mouseReleaseEvent(event);
}

void MainPageWidget::leaveEvent(QEvent *event)
{
    if(!m_isResizing)
    {
        unsetCursor();
        m_resizeRegion = ResizeNone;
    }

    QWidget::leaveEvent(event);
}

void MainPageWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

bool MainPageWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if(!m_isResizing && !m_isDragging)
        {
            int region =
                _hitTestResizeRegion(mouseEvent->globalPosition().toPoint());
            _updateResizeCursor(region);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MainPageWidget::_slotLogin(const QString &username,
                                const QString &password)
{
    GrpcClient::instance()->Login(username, password);
}

void MainPageWidget::_slotLoginResp(const int      errorCode,
                                    const int64_t  user_id,
                                    const QString &auth,
                                    const QString &account,
                                    const QString &lastLoginTime,
                                    const bool     isForceUpdate)
{
    qDebug() << "Login response received. Error code:" << errorCode
             << "ID:" << user_id << "Auth:" << auth;

    if(errorCode != ErrorCode::OK)
    {
        QMessageBox::critical(
            this,
            tr("Login Failed"),
            tr("Login failed with error code: %1").arg(errorCode));
        return;
    }

    if(isForceUpdate)
    {
        QString content = tr("A force update is required. Please "
                             "update the application from: "
                             "<a href=\"%1\">%1</a>")
                              .arg(Upgrade::instance()->GetUpgradeAddr());

        QMessageBox msgBox(QMessageBox::Information,
                           tr("Force Update Required"),
                           content,
                           QMessageBox::Ok,
                           this);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();

        _exit();
        return;
    }

    m_pAccount->setId(user_id);
    m_pAccount->setAuth(auth);
    m_pAccount->setName(account);
    m_pAccount->setLastLoginTime(lastLoginTime);
    m_pLoginWgtInst->hide();
    this->show();

    // switch to home page after login
    ui->listWidgetAppBar->setCurrentRow(0);

    // subscribe rag-core publish msg
    QVector<QString> topics{TOPIC_RAG_CORE};
    GrpcClient::instance()->Subscribe(user_id, auth, topics);

    // query plugin info after login in
    GrpcClient::instance()->GetPluginInfo();

    // query history after login in
    GrpcClient::instance()->GetSession(-1, user_id, auth, 50);

    return;
}

void MainPageWidget::_slotRegister(const QString &username,
                                   const QString &password)
{
    qDebug() << "Register attempt with username:" << username
             << "and password:" << password;
    GrpcClient::instance()->RegAccount(username, password);
}

void MainPageWidget::_slotRegisterResp(const int     errorCode,
                                       const int64_t user_id)
{
    qDebug() << "Register response received. Error code:" << errorCode
             << "User ID:" << user_id;

    if(errorCode != ErrorCode::OK)
    {
        // QMessageBox::critical(
        //     this,
        //     tr("Register Failed"),
        //     tr("Register failed with error code: %1").arg(errorCode));
        return;
    }

    m_pAccount->setId(user_id);
    // QMessageBox::information(
    //     this,
    //     tr("Register Successful"),
    //     tr("Register successful. Please login with your new account."));
}

void MainPageWidget::_slotLogout()
{
    qDebug() << "Logout signal received.";
    if(!m_pAccount->isValid())
    {
        qDebug() << "Account instance is null or invalid. exit";
        _exit();
        return;
    }

    GrpcClient::instance()->Logout(m_pAccount->id(), m_pAccount->auth());
}

void MainPageWidget::_slotLogoutResp(const int errorCode, const int64_t user_id)
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

void MainPageWidget::_slotSwitchAppBar(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
    switch(index)
    {
        case 0: {
            qDebug() << "Switched to Home Page.";
        }
        break;
        case 1: {
            qDebug() << "Switched to Settings Page.";
        }
        break;
        default:
            qDebug() << "Switched to app page index:" << index;
            break;
    }
}

void MainPageWidget::_slotCtlBtnGroupClicked(int id)
{
    qDebug() << "Control bar button clicked, id:" << id;
    m_pCtlBtnGroup->buttons().at(id)->setChecked(true);
    switch(id)
    {
        case 0: // Minimize
        {
            qDebug() << "Minimize button clicked.";
            _minimizeWindow();
        }
        break;
        case 1: // Audio Enable/Disable
        {
            qDebug() << "Audio button clicked.";
            _audioToggle();
        }
        break;
        case 2: // Select Screen
        {
            qDebug() << "Select Screen button clicked.";
            _selectScreen();
        }
        break;
        case 3: // switch account
        {
            qDebug() << "Switch account button clicked.";
            _switchAccount();
        }
        break;
        case 4: // Exit
        {
            qDebug() << "Exit button clicked.";
            _exit();
        }
        break;
        default:
            break;
    }
}

void MainPageWidget::_slotUpdateRealTime()
{
    // qDebug() << "Updating real time...";
    auto curr = QDateTime::currentDateTime();
    ui->lblTime->setText(curr.toString("hh:mm:ss"));
    ui->lblDate->setText(curr.toString("yyyy-MM-dd"));
}

void MainPageWidget::_slotUserBtnClicked(bool checked)
{
    m_pAccount->exec();
}

void MainPageWidget::_slotGrpcConnected(const QString &address)
{
    std::cout << "Core service connected to gRPC server at "
              << address.toStdString() << std::endl;
    ui->lblNetStatus->setText(tr("%1\n%2").arg(address).arg(tr("Connected")));
}

void MainPageWidget::_slotGrpcConnectFailed(const QString &address)
{
    qDebug() << "Fail to Connect gRPC server at " << address;
    ui->lblNetStatus->setText(
        tr("%1\n%2").arg(address).arg(tr("Disconnected")));
}

void MainPageWidget::_slotGrpcDisconnected(const QString &address)
{
    qDebug() << "gRPC server disconnected at " << address;
    ui->lblNetStatus->setText(
        tr("%1\n%2").arg(address).arg(tr("Disconnected")));
}

void MainPageWidget::_slotComboLangCurrentChanged(int iIndex)
{
    qDebug() << "Language combo box current index changed: " << iIndex;
    QApplication::removeTranslator(m_pTranslator);
    switch(iIndex)
    {
        case 0: // Chinese
        {
            qDebug() << "Switching to Chinese.";
            m_pTranslator->load(":/languages/zh_CN");
            emit Bus::instance() -> signalLanguageSwitch(LANG_ZH_CN);
        }
        break;
        case 1: // English
        {
            qDebug() << "Switching to English.";
            m_pTranslator->load(":/languages/en_UK");
            emit Bus::instance() -> signalLanguageSwitch(LANG_EN_UK);
        }
        break;
        case 2: // German
        {
            qDebug() << "Switching to German.";
            m_pTranslator->load(":/languages/de_DE");
            emit Bus::instance() -> signalLanguageSwitch(LANG_DE_DE);
        }
        break;
        default:
            qDebug() << "Unknown language index: " << iIndex;
            return;
    }
    QApplication::installTranslator(m_pTranslator);
}

void MainPageWidget::_slotSwitchAccount()
{
    qDebug() << "Switch account signal received.";
    _switchAccount();
}

void MainPageWidget::_slotPluginLoaded(PluginInterface *plugin,
                                       const QString   &filePath)
{
    if(!plugin)
    {
        qDebug() << "Failed to load null plugin";
        return;
    }

    // init plugin
    auto wgt = plugin->Init(Bus::instance());
    emit BusAdapter::instance() -> signalPing();

    // update model info
    auto infos = SettingPageModel::instance()->GetBusModelInfos();
    emit BusAdapter::instance() -> signalModelInfoUpdateNtf(infos);

    // update memory info
    auto memInfos = SettingPageMemory::instance()->GetBusMemoryInfos();
    emit BusAdapter::instance() -> signalMemoryInfoUpdateNtf(memInfos);

    // update audio param
    auto params = Config::instance()->getBusAudioParams();
    emit BusAdapter::instance() -> signalAudioParamUpdateNtf(params);
    if(wgt)
    {
        // TODO sort icon position
        int       index = ui->stackedWidget->count();
        QFileInfo fileInfo(filePath);
        auto iconPath = fileInfo.absoluteDir().absoluteFilePath(plugin->Icon());
        _addAppBarItem(plugin->Name(), iconPath, index);
        ui->stackedWidget->insertWidget(index, wgt);
    }
}

void MainPageWidget::_slotPluginUnloaded(const QString &pluginId,
                                         const QString &pluginName)
{
    qDebug() << "MainPageWidget::_slotPluginUnloaded with pluginId: "
             << pluginId << ", pluginName: " << pluginName;

    // remove app bar item
    int index = -1;
    for(int i = 0; i < ui->listWidgetAppBar->count(); ++i)
    {
        auto item = ui->listWidgetAppBar->item(i);
        if(item && item->text() == pluginName)
        {
            index = i;
            delete ui->listWidgetAppBar->takeItem(i);
            break;
        }
    }

    // remove stacked widget item
    if(index != -1)
    {
        auto wgt = ui->stackedWidget->widget(index);
        if(wgt)
        {
            ui->stackedWidget->removeWidget(wgt);
            wgt->deleteLater();
        }
    }
}

void MainPageWidget::_slotCoreStarted()
{
}

void MainPageWidget::_slotCoreFinished(int                  exitCode,
                                       QProcess::ExitStatus exitStatus)
{
}

void MainPageWidget::_slotCoreError(QProcess::ProcessError error)
{
}

void MainPageWidget::_slotImageCaptured(const QPixmap &pixmap)
{
    qDebug() << "Image captured from screen capture. Size: " << pixmap.size();
}

void MainPageWidget::_initProcMgr()
{
    m_pProcManagerInst->init();
}

void MainPageWidget::_initPluginMgr()
{
    // scan plugins in the "plugins" directory relative to the executable
    QDir dir = Config::instance()->getPluginFilePath();
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

int MainPageWidget::_hitTestResizeRegion(const QPoint &globalPos) const
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

void MainPageWidget::_updateResizeCursor(int region)
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

void MainPageWidget::_initAppBar()
{
    ui->listWidgetAppBar->setViewMode(QListView::IconMode);
    ui->listWidgetAppBar->setFlow(QListView::TopToBottom);
    ui->listWidgetAppBar->setMovement(QListView::Static);
    ui->listWidgetAppBar->setResizeMode(QListView::Adjust);
    ui->listWidgetAppBar->setWrapping(false);
    ui->listWidgetAppBar->setUniformItemSizes(true);
    ui->listWidgetAppBar->setTextElideMode(Qt::ElideNone);
    ui->listWidgetAppBar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidgetAppBar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidgetAppBar->setStyleSheet(
        StyleMgr::ParseFile(":/styles/slide_bar"));

    _addAppBarItem(tr("Home"), ":/icons/home", 0);
    _addAppBarItem(tr("Setting"), ":/icons/settings", 1);
}

void MainPageWidget::_initControlBar()
{
    ui->btnAudio->setIcon(QIcon(":/icons/microphone_enable"));
    ui->btnAudio->setVisible(true);

    m_pCtlBtnGroup->addButton(ui->btnMinimize, 0);
    m_pCtlBtnGroup->addButton(ui->btnAudio, 1);
    m_pCtlBtnGroup->addButton(ui->btnSelectScreen, 2);
    m_pCtlBtnGroup->addButton(ui->btnSwitch, 3);
    m_pCtlBtnGroup->addButton(ui->btnExit, 4);
    for(auto btn : m_pCtlBtnGroup->buttons())
        btn->setStyleSheet(StyleMgr::ParseFile(":/styles/ctl_push_button"));

    ui->lblNotice->setText(tr("Welcome"));
}

void MainPageWidget::_initStackedWidget()
{
    ui->stackedWidget->addWidget(m_pHomePageWgtInst);
    ui->stackedWidget->addWidget(m_pSettingPageWgtInst);

    ui->stackedWidget->setCurrentIndex(0);

    ui->btnUser->setStyleSheet(StyleMgr::ParseFile(":/styles/headimg_btn"));
}

void MainPageWidget::_initConnections()
{
    connect(m_pLoginWgtInst,
            &LoginWidget::signalLogin,
            this,
            &MainPageWidget::_slotLogin);

    connect(m_pLoginWgtInst,
            &LoginWidget::signalRegister,
            this,
            &MainPageWidget::_slotRegister);

    connect(m_pLoginWgtInst,
            &LoginWidget::signalLogout,
            this,
            &MainPageWidget::_slotLogout);

    connect(SettingPageNetwork::instance(),
            &SettingPageNetwork::signalSwitchAccount,
            this,
            &MainPageWidget::_slotSwitchAccount);

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slotUpdateRealTime()));

    connect(ui->btnUser,
            &QToolButton::clicked,
            this,
            &MainPageWidget::_slotUserBtnClicked);

    connect(ui->listWidgetAppBar,
            &QListWidget::currentRowChanged,
            this,
            &MainPageWidget::_slotSwitchAppBar);

    connect(m_pCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &MainPageWidget::_slotCtlBtnGroupClicked);

    connect(m_pGrpcClient,
            &GrpcClient::signalGrpcConnected,
            this,
            &MainPageWidget::_slotGrpcConnected);

    connect(m_pGrpcClient,
            &GrpcClient::signalGrpcConnectFailed,
            this,
            &MainPageWidget::_slotGrpcConnectFailed);

    connect(m_pGrpcClient,
            &GrpcClient::signalGrpcDisconnected,
            this,
            &MainPageWidget::_slotGrpcDisconnected);

    connect(m_pGrpcClient,
            &GrpcClient::signalLoginResp,
            this,
            &MainPageWidget::_slotLoginResp);

    connect(m_pGrpcClient,
            &GrpcClient::signalRegAccountResp,
            this,
            &MainPageWidget::_slotRegisterResp);

    connect(m_pGrpcClient,
            &GrpcClient::signalLogoutResp,
            this,
            &MainPageWidget::_slotLogoutResp);

    connect(ui->comboLang,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(_slotComboLangCurrentChanged(int)));

    connect(m_pPluginMgrInst,
            &PluginMgr::signalPluginLoaded,
            this,
            &MainPageWidget::_slotPluginLoaded);

    connect(m_pPluginMgrInst,
            &PluginMgr::signalPluginUnloaded,
            this,
            &MainPageWidget::_slotPluginUnloaded);

    connect(m_pProcManagerInst,
            &ProcManager::signalCoreStarted,
            this,
            &MainPageWidget::_slotCoreStarted);
    connect(m_pProcManagerInst,
            &ProcManager::signalCoreFinished,
            this,
            &MainPageWidget::_slotCoreFinished);
    connect(m_pProcManagerInst,
            &ProcManager::signalCoreError,
            this,
            &MainPageWidget::_slotCoreError);
}

void MainPageWidget::_initConnStatus()
{
    ui->lblNetStatus->setWordWrap(true);
    ui->lblNetStatus->setAlignment(Qt::AlignCenter);
}

void MainPageWidget::_initTimer()
{
    m_pTimer->start(1000);
}

void MainPageWidget::_initServer()
{
    auto confs = Config::instance()->networkConfigs();
    for(const auto &conf : confs)
    {
        if(!conf.isEnable)
            continue;

        QString address = QString("%1:%2").arg(conf.ip).arg(conf.port);
        qDebug() << "Connecting to gRPC server at " << address;
        m_pGrpcClient->Connect(address);
        return;
    }
    // m_pGrpcClient->Connect("127.0.0.1:50051");
}

void MainPageWidget::_initLanguage()
{
    ui->comboLang->setStyleSheet(StyleMgr::ParseFile(":/styles/combo_box"));

    // get local language
    QString localLang = System::instance()->LocalLang();
    if(localLang.startsWith("zh"))
        emit ui->comboLang->currentIndexChanged(0);
    else if(localLang.startsWith("en"))
        emit ui->comboLang->currentIndexChanged(1);
    else if(localLang.startsWith("de"))
        emit ui->comboLang->currentIndexChanged(2);
    else
        emit ui->comboLang->currentIndexChanged(1); // Default to English
}

void MainPageWidget::_minimizeWindow()
{
    this->showMinimized();
}

void MainPageWidget::_audioToggle()
{
    m_isAudioEnable = (m_isAudioEnable) ? false : true;
    if(m_isAudioEnable)
    {
        ui->btnAudio->setIcon(QIcon(":/icons/microphone_enable"));
        ui->btnAudio->setVisible(true);
        AudioMgr::instance()->enable();
    } else
    {
        ui->btnAudio->setIcon(QIcon(":/icons/microphone_disable"));
        ui->btnAudio->setVisible(true);
        AudioMgr::instance()->disable();
    }
}

void MainPageWidget::_selectScreen()
{
    ScreenCapture *capture = new ScreenCapture(this);
    connect(capture,
            &ScreenCapture::signalImageCaptured,
            this,
            &MainPageWidget::_slotImageCaptured);
    capture->start();
}

void MainPageWidget::_showAlarmDialog()
{
    QMessageBox::information(this, tr("Alarm"), tr("This is an alarm dialog!"));
}

void MainPageWidget::_exit()
{
    m_pProcManagerInst->destroy();
    this->close();
    if(m_pLoginWgtInst)
        m_pLoginWgtInst->close();
}

void MainPageWidget::_switchAccount()
{
    // clear account info
    if(m_pAccount)
        m_pAccount->clear();

    m_pLoginWgtInst->show();
    this->hide();
}

void MainPageWidget::_addAppBarItem(const QString &text,
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

    ui->listWidgetAppBar->insertItem(index, item);
}