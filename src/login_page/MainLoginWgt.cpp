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
}

MainLoginWgt::~MainLoginWgt()
{
    qDeleteAll(m_listUserInfo.begin(), m_listUserInfo.end());
    m_listUserInfo.clear();
    delete ui;
}

void MainLoginWgt::_SlotBtnLoginClicked()
{
}

void MainLoginWgt::_SlotBtnCancleClicked()
{
    this->close();
}

void MainLoginWgt::_InitUserNameCombBox()
{
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
