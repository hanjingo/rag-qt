#ifndef MAINLOGINWGT_H
#define MAINLOGINWGT_H

#include <QWidget>

#include "DBPublicH.h"

QT_BEGIN_NAMESPACE
class FrameworkWidget;
QT_END_NAMESPACE

namespace Ui
{
class MainLoginWgt;
}

class MainLoginWgt : public QWidget
{
    Q_OBJECT

  public:
    static MainLoginWgt *GetLoginWgtInst();

  signals:
    void SignalLogined();

  private slots:
    void _SlotBtnLoginClicked();
    void _SlotBtnCancleClicked();

  private:
    /***********************************************
    * 	@ Author	: RJie
    *	@ Date  	: 2021-09-17
    *	@ Describe	: 初始化用户名下拉框
    *	@ Paramters	:
    ************************************************/
    void _InitUserNameCombBox();

    /***********************************************
    * 	@ Author	: RJie
    *	@ Date  	: 2021-09-17
    *	@ Describe	: 初始化信号和曹的链接
    *	@ Paramters	:
    ************************************************/
    void _InitFuncBtnConnect();

  protected:
    explicit MainLoginWgt(QWidget *parent = nullptr);
    ~MainLoginWgt();

  private:
    Ui::MainLoginWgt    *ui;
    static MainLoginWgt *m_stLoginWgtInst;

  private:
    bool                  m_bDebugState;
    QList<StDBUserData *> m_listUserInfo;
    FrameworkWidget      *m_pInterfaceWgt;
};
#endif // MAINLOGINWGT_H
