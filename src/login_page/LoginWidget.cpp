#include "LoginWidget.h"
#include "ui_LoginWidget.h"

#include "StyleMgr.h"

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

    // Keep container layers transparent so only LoginPage's panel is visible.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("QWidget#LoginWidget{background: transparent;}"
                  "QStackedWidget#stackedWidget{background: transparent;}");

    ui->stackedWidget->addWidget(m_pLoginPageInst);
    ui->stackedWidget->setCurrentIndex(0);

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
}

LoginWidget::~LoginWidget()
{
    delete ui;
}