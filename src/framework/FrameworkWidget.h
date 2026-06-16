#ifndef FRAMEWORKWIDGET_H
#define FRAMEWORKWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QCloseEvent>
#include <QString>
#include <QPoint>
#include <QTranslator>

#include "HomePageWidget.h"
#include "SettingPageWidget.h"
#include "AudioPageWidget.h"
#include "VideoPageWidget.h"
#include "ImagePageWidget.h"
#include "TextPageWidget.h"
#include "ToolPageWidget.h"

#include "ProcManager.h"
#include "GrpcClient.h"

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QStackedWidget;
class HomePageWidget;
class TextPageWidget;
class AudioPageWidget;
class VideoPageWidget;
class ImagePageWidget;
class SettingPageWidget;
class ToolPageWidget;
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
    void _slotCtlBtnGroupClicked(int);
    void _slotAppBarBtnGroupClicked(int);
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

  private:
    Ui::FrameworkWidget    *ui;
    static FrameworkWidget *m_stFrameworkWidgetInst;

  private:
    QButtonGroup *m_pAppBarBtnGroup;
    QButtonGroup *m_pCtlBtnGroup;
    QTimer       *m_pTimer;

    bool   m_isResizing   = false;
    int    m_resizeRegion = ResizeNone;
    QPoint m_pressGlobalPos;
    QRect  m_pressGeometry;

  private:
    ProcManager *m_pProcManagerInst;
    QTranslator *m_pTranslator;
    GrpcClient  *m_pGrpcClient;

    HomePageWidget    *m_pHomePageWgtInst;
    TextPageWidget    *m_pTextPageWgtInst;
    ImagePageWidget   *m_pImagePageWgtInst;
    AudioPageWidget   *m_pAudioPageWgtInst;
    VideoPageWidget   *m_pVideoPageWgtInst;
    SettingPageWidget *m_pSettingPageWgtInst;
    ToolPageWidget    *m_pToolPageWgtInst;
};

#endif // FRAMEWORKWIDGET_H
