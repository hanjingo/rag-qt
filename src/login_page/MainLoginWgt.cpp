#include "DbOperation.h"
#include "MainLoginWgt.h"
#include "SystemConfig.h"
#include "ui_MainLoginWgt.h"
#include "FrameworkWidget.h"
#include "MchnStateCtrlLayer.h"
#include "ComPromptDialog.h"
#include "CommBusMaintain.h"
#include "MchnBusinessLayer.h"
#include "DefineVersion.h"

MainLoginWgt *MainLoginWgt::m_stLoginWgtInst = nullptr;

MainLoginWgt *MainLoginWgt::GetLoginWgtInst()
{
    if(nullptr == m_stLoginWgtInst)
    {
        m_stLoginWgtInst = new MainLoginWgt();
    }

    return m_stLoginWgtInst;
}

MainLoginWgt::MainLoginWgt(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , ui(new Ui::MainLoginWgt)
    , m_bDebugState(false)
    , m_pInterfaceWgt(FrameworkWidget::GetFrameworkWidgetInst())
{
    ui->setupUi(this);
    _InitFuncBtnConnect();
    _InitUserNameCombBox();
    ui->edit_Password->setValidator(
        new QRegExpValidator(QRegExp("^[a-zA-Z0-9]+$"), this));
    this->setWindowIcon(QPixmap(":/SVG/logo.svg"));
    //ui->lbl_IntrVersion->setText(QString("V%1.%2.%3.%4").arg(VER_MAJOR_X).arg(VER_MINOR_Y).arg(VER_MINOR_Z).arg(VER_MINOR_B));
    ui->lbl_IntrVersion->setText(QString("V1"));
    qDebug() << MASTERCOMMOUTPUT << "当前版本:"
             << QString("V%1.%2.%3.%4")
                    .arg(VER_MAJOR_X)
                    .arg(VER_MINOR_Y)
                    .arg(VER_MINOR_Z)
                    .arg(VER_MINOR_B);
}

MainLoginWgt::~MainLoginWgt()
{
    qDeleteAll(m_listUserInfo.begin(), m_listUserInfo.end());
    m_listUserInfo.clear();
    delete ui;
}

void MainLoginWgt::_SlotBtnLoginClicked()
{
    // 首先获取用户数据库表中所有的用户名和密码
    // 找到用户选择的账号
    // 对比用户输入的密码和数据库中密码是否对应
    // 对应则初始化主界面窗口
    // 不对应则倒计时单按钮确定弹窗提醒用户后自动清除密码控件文本

    bool bLoginSuc = false;
    foreach(StDBUserData *pUserInfo, m_listUserInfo)
    {
        if(ui->cmb_Account->currentText() == pUserInfo->strAccount)
        {
            if(ui->edit_Password->text() == pUserInfo->strPassword)
            {
                pUserInfo->bLogin = true;
                SystemConfig::GetSysCnfgInst()->SetCurUserInfo(*pUserInfo);
                DbOperation::GetDbOperationInst()->DbInsertUpdateOpt(
                    pUserInfo,
                    DTSTRCT_USER,
                    DBOPT_UPDATE);
                bLoginSuc = true;
                emit SignalLogined();
            }
        } else if(true == pUserInfo->bLogin)
        {
            pUserInfo->bLogin = false;
            DbOperation::GetDbOperationInst()->DbInsertUpdateOpt(pUserInfo,
                                                                 DTSTRCT_USER,
                                                                 DBOPT_UPDATE);
        }
    }
    if(true == bLoginSuc)
    {
        this->hide();
        ui->edit_Password->setText(QString());
        EPermission ePermsn = (EPermission) SystemConfig::GetSysCnfgInst()
                                  ->GetCurUserInfo()
                                  .iPermission;
        m_pInterfaceWgt->SetCurPermission(ePermsn);
        if(false == m_bDebugState)
        {
            MchnStateCtrlLayer::GetMchnStateCtrlLayerInst()->SetMchnState(
                EMSTATE_RESET);
            CommBusMaintain::GetCommBusMaintainInst()->SendMochineResetCmd();
            CommBusMaintain::GetCommBusMaintainInst()->SendCyclGetTempt();
            CommBusMaintain::GetCommBusMaintainInst()
                ->SendReadUnitVer(); // 中位机
            CommBusMaintain::GetCommBusMaintainInst()->SendReadUnitVer(
                0x01); // 加样部
            CommBusMaintain::GetCommBusMaintainInst()->SendReadUnitVer(
                0x02); // 分析部
            CommBusMaintain::GetCommBusMaintainInst()->SendReadUnitVer(
                0x03); // 1号转移部
            CommBusMaintain::GetCommBusMaintainInst()->SendReadUnitVer(
                0x04); // 2号转移部
            CommBusMaintain::GetCommBusMaintainInst()->SendReadUnitVer(
                0x05); // 反应盘
            CommBusMaintain::GetCommBusMaintainInst()->SendReadUnitVer(
                0x06); // 泵阀部
            CommBusMaintain::GetCommBusMaintainInst()->SendReadUnitVer(
                0x07); // 温控部
            MchnBusinessLayer::GetMchnBusinessLayerInst()->ExecutQueryTemp(
                true);
        } else
        {
            // 更新数据库数据
            DbOperation::GetDbOperationInst()->DbSqlOpt(
                QString("UPDATE Sample SET IsHisPro='1' WHERE IsHisPro='0';"));
            DbOperation::GetDbOperationInst()->DbSqlOpt(
                QString("UPDATE Result SET IsHisPro='1' WHERE IsHisPro='0';"));
            DbOperation::GetDbOperationInst()->DbSqlOpt(QString(
                "UPDATE CaliResult SET IsHisPro='1' WHERE IsHisPro='0';"));
            DbOperation::GetDbOperationInst()->DbSqlOpt(QString(
                "UPDATE QCResult SET IsHisPro='1' WHERE IsHisPro='0';"));
        }
    } else
    {
        if(ui->edit_Password->text() == "")
        {
            ComPromptDialog::PromptDialog(tr("错误"),
                                          tr("密码输入为空，请输入正确密码"),
                                          3,
                                          2);
        } else
        {
            ComPromptDialog::PromptDialog(tr("错误"),
                                          tr("密码输入错误，请重新输入"),
                                          3,
                                          2);
        }
        ui->edit_Password->clear();    // 清空文本框内容
        ui->edit_Password->setFocus(); // 激活光标
    }
}

void MainLoginWgt::_SlotBtnCancleClicked()
{
    this->close();
}

void MainLoginWgt::_InitUserNameCombBox()
{
    QStringList   strNameList;
    QList<void *> listUsrInfo;
    DbOperation::GetDbOperationInst()->DbSelectOpt("SELECT * FROM User;",
                                                   DTSTRCT_USER,
                                                   listUsrInfo);
    foreach(void *pUser, listUsrInfo)
    {
        StDBUserData *pUserInfo = static_cast<StDBUserData *>(pUser);
        m_listUserInfo.append(pUserInfo);
        if(true == pUserInfo->bLogin)
        {
            strNameList.push_front(pUserInfo->strAccount);
        } else
        {
            strNameList.append(pUserInfo->strAccount);
        }
    }
    ui->cmb_Account->addItems(strNameList);
    if(strNameList.count() > 0)
    {
        ui->cmb_Account->setCurrentIndex(0);
    }
}

void MainLoginWgt::_InitFuncBtnConnect()
{
    connect(ui->btn_LogIn,
            SIGNAL(pressed()),
            this,
            SLOT(_SlotBtnLoginClicked()));
    connect(ui->btn_cancle,
            SIGNAL(pressed()),
            this,
            SLOT(_SlotBtnCancleClicked()));
    connect(ui->edit_Password,
            SIGNAL(returnPressed()),
            this,
            SLOT(_SlotBtnLoginClicked()));
}
