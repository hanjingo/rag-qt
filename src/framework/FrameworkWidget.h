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

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QStackedWidget;
class LoginWidget;
class HomePageWidget;
class SettingPageWidget;
QT_END_NAMESPACE

namespace Ui
{
class FrameworkWidget;
}

class FrameworkWidget : public QWidget
{
    Q_OBJECT

  public:
    static FrameworkWidget *GetFrameworkWidgetInst();
    explicit FrameworkWidget(QWidget *parent = nullptr);
    ~FrameworkWidget();

  protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void changeEvent(QEvent *event) override;

  private slots:
    void _slotLogin(const QString &username, const QString &password);
    void
    _slotLoginResp(const int errorCode, const int32_t id, const QString &auth);
    void _slotRegister(const QString &username, const QString &password);
    void _slotRegisterResp(const int errorCode, const int32_t user_id);
    void _slotLogout();
    void _slotLogoutResp(const int errorCode, const int user_id);

    void _slotCtlBtnGroupClicked(int);
    void _slotUpdateRealTime();
    void _slotGrpcConnected(const QString &address);
    void _slotGrpcConnectFailed(const QString &address);
    void _slotComboLangCurrentChanged(int iIndex);

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
    void _selectScreen();
    void _showAlarmDialog();
    void _exit();
    void _switchAccount();

    void
    _addAppBarItem(const QString &text, const QString &iconPath, int index);
    void _addPlugin(const QFileInfo &fileInfo, int index = -1);

  private:
    Ui::FrameworkWidget    *ui;
    static FrameworkWidget *m_stFrameworkWidgetInst;

  private:
    QButtonGroup *m_pCtlBtnGroup;
    QTimer       *m_pTimer;

    bool   m_isResizing   = false;
    int    m_resizeRegion = ResizeNone;
    QPoint m_pressGlobalPos;
    QRect  m_pressGeometry;

  private:
    ProcManager *m_pProcManagerInst;
    PluginMgr   *m_pPluginMgrInst;
    QTranslator *m_pTranslator;
    GrpcClient  *m_pGrpcClient;
    Account     *m_pAccount;

    LoginWidget       *m_pLoginWgtInst;
    HomePageWidget    *m_pHomePageWgtInst;
    SettingPageWidget *m_pSettingPageWgtInst;
};


#endif // FRAMEWORKWIDGET_H
