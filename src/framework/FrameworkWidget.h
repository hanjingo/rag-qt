#ifndef FRAMEWORKWIDGET_H
#define FRAMEWORKWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QCloseEvent>

#include "HomePageWidget.h"
#include "SettingPageWidget.h"
#include "AudioPageWidget.h"
#include "ChatPageWidget.h"
#include "ImagePageWidget.h"
#include "TextPageWidget.h"

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QStackedWidget;
class HomePageWidget;
class TextPageWidget;
class AudioPageWidget;
class ChatPageWidget;
class ImagePageWidget;
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
    void closeEvent(QCloseEvent *event);

  private slots:
    void _slotBtnExitClicked();
    void _slotAppBarBtnGroupClicked(int);
    void _slotUpdateRealTime();

  private:
    void _initAppBar();
    void _initControlBar();
    void _initStackedWidget();
    void _initConnections();
    void _initTimer();

  private:
    Ui::FrameworkWidget    *ui;
    static FrameworkWidget *m_stFrameworkWidgetInst;

  private:
    QButtonGroup *m_pAppBarBtnGroup;
    QButtonGroup *m_pCtlBtnGroup;
    QTimer       *m_pTimer;

  private:
    HomePageWidget    *m_pHomePageWgtInst;
    TextPageWidget    *m_pTextPageWgtInst;
    ImagePageWidget   *m_pImagePageWgtInst;
    AudioPageWidget   *m_pAudioPageWgtInst;
    ChatPageWidget    *m_pChatPageWgtInst;
    SettingPageWidget *m_pSettingPageWgtInst;
};

#endif // FRAMEWORKWIDGET_H
