#include "LoginPage.h"
#include "ui_LoginPage.h"

#include "StyleMgr.h"

LoginPage *LoginPage::m_stLoginPageInst = nullptr;

LoginPage *LoginPage::Instance()
{
    if(nullptr == m_stLoginPageInst)
        m_stLoginPageInst = new LoginPage();

    return m_stLoginPageInst;
}

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , ui(new Ui::LoginPage)
{
    ui->setupUi(this);

    // Keep the page/container transparent and only show the login panel card.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("QWidget#LoginPage{background: transparent;}"
                  "QWidget#wgtLogin{background: transparent;}");

#ifdef DEBUG
    ui->editAccount->setText("admin");
    ui->editPassword->setText("admin");
#endif

    ui->editAccount->setStyleSheet(
        StyleMgr::ParseFile(":/styles/passwd_line_edit"));
    ui->editPassword->setStyleSheet(
        StyleMgr::ParseFile(":/styles/passwd_line_edit"));

    ui->lblLogo->setPixmap(QPixmap(":/icons/logo"));
    ui->lblTitle->setStyleSheet(StyleMgr::ParseFile(":/styles/title_label"));

    ui->btnLogin->setStyleSheet(StyleMgr::ParseFile(":/styles/push_button"));
    ui->btnLogout->setStyleSheet(StyleMgr::ParseFile(":/styles/push_button"));
    ui->btnRegister->setStyleSheet(StyleMgr::ParseFile(":/styles/push_button"));

    ui->lblForgotPasswd->setStyleSheet(
        StyleMgr::ParseFile(":/styles/red_label"));
    ui->lblForgotPasswd->setText(
        "<a style='color:red;' href=http://www.baidu.com> Forgot "
        "Password?</a>");
    ui->lblForgotPasswd->setOpenExternalLinks(true);

    _initConnections();
}

LoginPage::~LoginPage()
{
    delete ui;
}

void LoginPage::_slotBtnLoginClicked()
{
    QString username = ui->editAccount->text();
    QString password = ui->editPassword->text();
    emit    SignalLogin(username, password);
}

void LoginPage::_slotBtnRegisterClicked()
{
    QString username = ui->editAccount->text();
    QString password = ui->editPassword->text();
    emit    SignalRegister(username, password);
}

void LoginPage::_slotBtnLogoutClicked()
{
    emit SignalLogout();
}

void LoginPage::_initConnections()
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
