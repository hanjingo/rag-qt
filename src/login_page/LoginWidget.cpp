#include "LoginWidget.h"
#include "ui_LoginWidget.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <QMessageBox>

#include "Global.h"
#include "StyleMgr.h"
#include "GrpcClient.h"
#include "Error.h"

LoginWidget *LoginWidget::m_stLoginWgtInst = nullptr;

LoginWidget *LoginWidget::Instance()
{
    if(nullptr == m_stLoginWgtInst)
        m_stLoginWgtInst = new LoginWidget();

    return m_stLoginWgtInst;
}

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , ui(new Ui::LoginWidget)
    , m_pLoginPageInst(LoginPage::Instance())
{
    ui->setupUi(this);

    _initUI();
    _initConnections();
    _retranslate();
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::_initUI()
{
    // Keep container layers transparent so only LoginPage's panel is visible.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("QWidget#LoginWidget{background: transparent;}"
                  "QStackedWidget#stackedWidget{background: transparent;}");

    ui->stackedWidget->addWidget(m_pLoginPageInst);
    ui->stackedWidget->setCurrentIndex(0);
}

void LoginWidget::_initConnections()
{
    connect(m_pLoginPageInst,
            &LoginPage::SignalLogin,
            this,
            &LoginWidget::SignalLogin);
    connect(m_pLoginPageInst,
            &LoginPage::SignalRegister,
            this,
            &LoginWidget::SignalRegister);
    connect(m_pLoginPageInst,
            &LoginPage::SignalLogout,
            this,
            &LoginWidget::SignalLogout);

    // rpc response
    connect(GrpcClient::Instance(),
            &GrpcClient::SignalRegAccountResp,
            this,
            &LoginWidget::_slotRegisterResp);
}

void LoginWidget::_retranslate()
{
}

void LoginWidget::_slotRegisterResp(const int errorCode, const int64_t user_id)
{
    if(errorCode != ErrorCode::OK)
    {
        QMessageBox::critical(
            this,
            tr("Register Failed"),
            tr("Register failed with error code: %1").arg(errorCode));
        return;
    }

    QMessageBox::information(
        this,
        tr("Register Successful"),
        tr("Register successful. Please login with your new account."));
    m_pLoginPageInst->Login();
}