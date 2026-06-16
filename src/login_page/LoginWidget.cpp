#include "LoginWidget.h"
#include "ui_LoginWidget.h"

LoginWidget *LoginWidget::m_stLoginWgtInst = nullptr;

LoginWidget *LoginWidget::GetLoginWgtInst()
{
    if(nullptr == m_stLoginWgtInst)
        m_stLoginWgtInst = new LoginWidget();

    return m_stLoginWgtInst;
}

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    _initConnections();

#ifdef DEBUG
    ui->editAccount->setText("admin");
    ui->editPassword->setText("admin");
#endif
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::_slotBtnLoginClicked()
{
    QString username = ui->editAccount->text();
    QString password = ui->editPassword->text();
    emit    SignalLogin(username, password);
}

void LoginWidget::_slotBtnRegisterClicked()
{
    QString username = ui->editAccount->text();
    QString password = ui->editPassword->text();
    emit    SignalRegister(username, password);
}

void LoginWidget::_slotBtnLogoutClicked()
{
    emit SignalLogout();
}

void LoginWidget::_initConnections()
{
    connect(ui->btnLogin,
            SIGNAL(pressed()),
            this,
            SLOT(_slotBtnLoginClicked()));
    connect(ui->btnRegister,
            SIGNAL(pressed()),
            this,
            SLOT(_slotBtnRegisterClicked()));
    connect(ui->btnLogout,
            SIGNAL(pressed()),
            this,
            SLOT(_slotBtnLogoutClicked()));
    connect(ui->editPassword,
            SIGNAL(returnPressed()),
            this,
            SLOT(_slotBtnLoginClicked()));
}
