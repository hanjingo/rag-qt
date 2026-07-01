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

#include "Bus.h"
#include "PluginInterface.h"
#include "FrameworkWidget.h"
#include "ui_Account.h"
#include "ui_FrameworkWidget.h"
#include "Error.h"
#include "SettingPageModel.h"
#include "System.h"
#include "ScreenCapture.h"
#include "SettingPageNetwork.h"
#include "Audio.h"

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
    , m_pBusAdapter(BusAdapter::Instance())
    , m_pAccount(Account::Instance())
    , m_pLoginWgtInst(LoginWidget::Instance())
    , m_pHomePageWgtInst(HomePageWidget::GetMainHomePageInst())
    , m_pSettingPageWgtInst(SettingPageWidget::GetMainSettingPageInst())
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    _initConnections();
    _initAppBar();
    _initControlBar();
    _initStackedWidget();
    _initTimer();
    _initPluginMgr();
    _initLanguage();
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

void FrameworkWidget::InitCore()
{
    _initProcMgr();
}

void FrameworkWidget::InitNetwork()
{
    _initServer();
}

QString FrameworkWidget::ReadAllStandardOutput()
{
    return m_pProcManagerInst->readAllStandardOutput();
}

bool FrameworkWidget::IsConnectedToCoreService()
{
    return m_pGrpcClient ? m_pGrpcClient->IsConnected() : false;
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
                                     const int64_t  user_id,
                                     const QString &auth,
                                     const int32_t  privilege,
                                     const QString &account,
                                     const QString &lastLoginTime)
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

    m_pAccount->SetId(user_id);
    m_pAccount->SetAuth(auth);
    m_pAccount->SetPrivilege(privilege);
    m_pAccount->SetName(account);
    m_pAccount->SetLastLoginTime(lastLoginTime);
    m_pLoginWgtInst->hide();
    this->show();

    // query history after login in
    GrpcClient::Instance()->GetSession(-1, user_id, auth, 50);

    // query skill info after login in
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
                                        const int64_t user_id)
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
    QMessageBox::information(
        this,
        tr("Register Successful"),
        tr("Register successful. Please login with your new account."));
}

void FrameworkWidget::_slotLogout()
{
    qDebug() << "Logout signal received.";
    if(!m_pAccount->IsValid())
    {
        qDebug() << "Account instance is null or invalid. exit";
        _exit();
        return;
    }

    GrpcClient::Instance()->Logout(m_pAccount->Id(), m_pAccount->Auth());
}

void FrameworkWidget::_slotLogoutResp(const int     errorCode,
                                      const int64_t user_id)
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
        case 3: // Alarm
        {
            qDebug() << "Alarm button clicked.";
            _showAlarmDialog();
        }
        break;
        case 4: // switch account
        {
            qDebug() << "Switch account button clicked.";
            _switchAccount();
        }
        break;
        case 5: // Exit
        {
            qDebug() << "Exit button clicked.";
            _exit();
        }
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

void FrameworkWidget::_slotUserBtnClicked(bool checked)
{
    m_pAccount->exec();
}

void FrameworkWidget::_slotGrpcConnected(const QString &address)
{
    std::cout << "Core service connected to gRPC server at "
              << address.toStdString() << std::endl;
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
        {
            qDebug() << "Switching to Chinese.";
            m_pTranslator->load(":/languages/zh_CN");
            emit Bus::Instance() -> SignalLanguageSwitch("zh_CN");
        }
        break;
        case 1: // English
        {
            qDebug() << "Switching to English.";
            m_pTranslator->load(":/languages/en_UK");
            emit Bus::Instance() -> SignalLanguageSwitch("en_UK");
        }
        break;
        case 2: // German
        {
            qDebug() << "Switching to German.";
            m_pTranslator->load(":/languages/de_DE");
            emit Bus::Instance() -> SignalLanguageSwitch("de_DE");
        }
        break;
        default:
            qDebug() << "Unknown language index: " << iIndex;
            return;
    }
    QApplication::installTranslator(m_pTranslator);
}

void FrameworkWidget::_slotSwitchAccount()
{
    qDebug() << "Switch account signal received.";
    _switchAccount();
}

void FrameworkWidget::_slotPluginLoaded(PluginInterface *plugin,
                                        const QString   &filePath)
{
    if(!plugin)
    {
        qDebug() << "Failed to load null plugin";
        return;
    }

    // init plugin
    auto wgt = plugin->Init(Bus::Instance());
    emit BusAdapter::Instance() -> SignalPing();

    // update model info
    auto confs = SettingPageModel::Instance()->GetModelConfigs();
    emit BusAdapter::Instance() -> SignalModelInfoUpdateNtf(confs);
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

void FrameworkWidget::_slotPluginUnloaded(const QString &pluginId)
{
    qDebug() << "Plugin unloaded: " << pluginId;

    // remove app bar item
    int index = -1;
    for(int i = 0; i < ui->listWidgetAppBar->count(); ++i)
    {
        auto item = ui->listWidgetAppBar->item(i);
        if(item && item->text() == pluginId)
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

void FrameworkWidget::_slotCoreStarted()
{
}

void FrameworkWidget::_slotCoreFinished(int                  exitCode,
                                        QProcess::ExitStatus exitStatus)
{
}

void FrameworkWidget::_slotCoreError(QProcess::ProcessError error)
{
}

void FrameworkWidget::_slotImageCaptured(const QPixmap &pixmap)
{
    qDebug() << "Image captured from screen capture. Size: " << pixmap.size();
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
    ui->listWidgetAppBar->setCurrentRow(0);
}

void FrameworkWidget::_initControlBar()
{
    ui->btnAudio->setIcon(QIcon(":/icons/microphone_enable"));
    ui->btnAudio->setVisible(true);

    m_pCtlBtnGroup->addButton(ui->btnMinimize, 0);
    m_pCtlBtnGroup->addButton(ui->btnAudio, 1);
    m_pCtlBtnGroup->addButton(ui->btnSelectScreen, 2);
    m_pCtlBtnGroup->addButton(ui->btnAlarm, 3);
    m_pCtlBtnGroup->addButton(ui->btnSwitch, 4);
    m_pCtlBtnGroup->addButton(ui->btnExit, 5);
    for(auto btn : m_pCtlBtnGroup->buttons())
    {
        btn->setStyleSheet(StyleMgr::ParseFile(":/styles/ctl_push_button"));
    }
    // ui->btnSwitch->setStyleSheet(
    //     StyleMgr::ParseFile(":/styles/ctl_push_button"));

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

    connect(SettingPageNetwork::Instance(),
            &SettingPageNetwork::SignalSwitchAccount,
            this,
            &FrameworkWidget::_slotSwitchAccount);

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slotUpdateRealTime()));

    connect(ui->btnUser,
            &QToolButton::clicked,
            this,
            &FrameworkWidget::_slotUserBtnClicked);

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

    connect(m_pPluginMgrInst,
            &PluginMgr::SignalPluginLoaded,
            this,
            &FrameworkWidget::_slotPluginLoaded);

    connect(m_pPluginMgrInst,
            &PluginMgr::SignalPluginUnloaded,
            this,
            &FrameworkWidget::_slotPluginUnloaded);

    connect(m_pProcManagerInst,
            &ProcManager::SignalCoreStarted,
            this,
            &FrameworkWidget::_slotCoreStarted);
    connect(m_pProcManagerInst,
            &ProcManager::SignalCoreFinished,
            this,
            &FrameworkWidget::_slotCoreFinished);
    connect(m_pProcManagerInst,
            &ProcManager::SignalCoreError,
            this,
            &FrameworkWidget::_slotCoreError);
}

void FrameworkWidget::_initTimer()
{
    m_pTimer->start(1000);
}

void FrameworkWidget::_initServer()
{
    auto confs = SettingPageNetwork::Instance()->GetNetworkConfigs();
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

void FrameworkWidget::_initLanguage()
{
    ui->comboLang->setStyleSheet(StyleMgr::ParseFile(":/styles/combo_box"));
    ui->comboLang->setIconSize(QSize(24, 24));

    // get local language
    QString localLang = System::Instance()->LocalLang();
    if(localLang.startsWith("zh"))
        emit ui->comboLang->currentIndexChanged(0);
    else if(localLang.startsWith("en"))
        emit ui->comboLang->currentIndexChanged(1);
    else if(localLang.startsWith("de"))
        emit ui->comboLang->currentIndexChanged(2);
    else
        emit ui->comboLang->currentIndexChanged(1); // Default to English
}

void FrameworkWidget::_minimizeWindow()
{
    this->showMinimized();
}

void FrameworkWidget::_audioToggle()
{
    m_isAudioEnable = (m_isAudioEnable) ? false : true;
    if(m_isAudioEnable)
    {
        ui->btnAudio->setIcon(QIcon(":/icons/microphone_enable"));
        ui->btnAudio->setVisible(true);
        AudioMgr::Instance()->enable();
    } else
    {
        ui->btnAudio->setIcon(QIcon(":/icons/microphone_disable"));
        ui->btnAudio->setVisible(true);
        AudioMgr::Instance()->disable();
    }
}

void FrameworkWidget::_selectScreen()
{
    ScreenCapture *capture = new ScreenCapture(this);
    connect(capture,
            &ScreenCapture::SignalImageCaptured,
            this,
            &FrameworkWidget::_slotImageCaptured);
    capture->start();
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

    ui->listWidgetAppBar->insertItem(index, item);
}