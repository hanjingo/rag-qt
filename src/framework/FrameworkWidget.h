#ifndef FRAMEWORKWIDGET_H
#define FRAMEWORKWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QCloseEvent>
#include <QString>
#include <QPoint>
#include <QMap>
#include <QTranslator>

#include "Account.h"
#include "LoginWidget.h"
#include "HomePageWidget.h"
#include "SettingPageWidget.h"

#include "ProcManager.h"
#include "PluginMgr.h"
#include "GrpcClient.h"
#include "StyleMgr.h"
#include "BusAdapter.h"
#include "Audio.h"

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QStackedWidget;
class LoginWidget;
class HomePageWidget;
class SettingPageWidget;
class BusAdapter;
QT_END_NAMESPACE

namespace Ui
{
class FrameworkWidget;
}

class FrameworkWidget : public QWidget
{
    Q_OBJECT

  public:
    static FrameworkWidget *Instance();
    explicit FrameworkWidget(QWidget *parent = nullptr);
    ~FrameworkWidget();

    void    InitCore();
    void    InitNetwork();
    QString ReadAllStandardOutput();
    bool    IsConnectedToCoreService();

  protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void changeEvent(QEvent *event) override;

  private slots:
    void _slotLogin(const QString &username, const QString &password);
    void _slotLoginResp(const int      errorCode,
                        const int64_t  user_id,
                        const QString &auth,
                        const int32_t  privilege,
                        const QString &account,
                        const QString &lastLoginTime);
    void _slotRegister(const QString &username, const QString &password);
    void _slotRegisterResp(const int errorCode, const int64_t user_id);
    void _slotLogout();
    void _slotLogoutResp(const int errorCode, const int64_t user_id);
    void _slotUserBtnClicked(bool checked);

    void _slotCtlBtnGroupClicked(int);
    void _slotUpdateRealTime();
    void _slotGrpcConnected(const QString &address);
    void _slotGrpcConnectFailed(const QString &address);
    void _slotComboLangCurrentChanged(int iIndex);
    void _slotSwitchAccount();

    void _slotPluginLoaded(PluginInterface *plugin, const QString &filePath);
    void _slotPluginUnloaded(const QString &pluginId);

    void _slotCoreStarted();
    void _slotCoreFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void _slotCoreError(QProcess::ProcessError error);

    void _slotImageCaptured(const QPixmap &pixmap);

  private:
    enum ResizeRegion
    {
        ResizeNone   = 0,
        ResizeLeft   = 0x01,
        ResizeRight  = 0x02,
        ResizeTop    = 0x04,
        ResizeBottom = 0x08,
    };

    int  _hitTestResizeRegion(const QPoint &globalPos) const;
    void _updateResizeCursor(int region);

    void _initProcMgr();
    void _initPluginMgr();
    void _initAppBar();
    void _initControlBar();
    void _initStackedWidget();
    void _initConnections();
    void _initTimer();
    void _initServer();
    void _initLanguage();

    void _minimizeWindow();
    void _audioToggle();
    void _selectScreen();
    void _showAlarmDialog();
    void _exit();
    void _switchAccount();

    void
    _addAppBarItem(const QString &text, const QString &iconPath, int index);

  private:
    Ui::FrameworkWidget    *ui;
    static FrameworkWidget *m_stFrameworkWidgetInst;

  private:
    QButtonGroup *m_pCtlBtnGroup;
    QTimer       *m_pTimer;

    bool m_isAudioEnable = true;

    bool   m_isResizing   = false;
    int    m_resizeRegion = ResizeNone;
    QPoint m_pressGlobalPos;
    QRect  m_pressGeometry;

  private:
    ProcManager *m_pProcManagerInst;
    PluginMgr   *m_pPluginMgrInst;
    QTranslator *m_pTranslator;
    GrpcClient  *m_pGrpcClient;
    BusAdapter  *m_pBusAdapter;
    Account     *m_pAccount;

    LoginWidget       *m_pLoginWgtInst;
    HomePageWidget    *m_pHomePageWgtInst;
    SettingPageWidget *m_pSettingPageWgtInst;
};


#endif // FRAMEWORKWIDGET_H
