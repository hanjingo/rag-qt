#include "LoginPage.h"
#include "ui_LoginPage.h"

#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

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

    _initUI();
    _initConnections();
    _retranslate();
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
    // TODO: Add validation for username and password before emitting the signal.
    if(!_validateInput(username, password))
        return;

    emit SignalRegister(username, password);
}

void LoginPage::_slotBtnLogoutClicked()
{
    emit SignalLogout();
}

void LoginPage::_initUI()
{
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
        "<a style='color:red;' href=http://8.134.87.167> Forgot "
        "Password?</a>");
    ui->lblForgotPasswd->setOpenExternalLinks(true);
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

void LoginPage::_retranslate()
{
}

bool LoginPage::_validateInput(const QString &username, const QString &password)
{
    QRegularExpression accountRegex("^[a-zA-Z0-9]{6,16}$");
    if(!accountRegex.match(username).hasMatch())
    {
        QMessageBox::warning(nullptr,
                             tr("Input Error"),
                             tr("Username must be 6-16 characters long and "
                                "contain only letters and numbers."));
        return false;
    }

    static const QSet<QString> blacklist = {"root",
                                            "owner",
                                            "admin",
                                            "administrator",
                                            "developer"
                                            "system",
                                            "user"};
    if(blacklist.contains(username.toLower()))
    {
        QMessageBox::warning(nullptr,
                             tr("Input Error"),
                             tr("Username '%1' is not allowed. Please choose a "
                                "different username.")
                                 .arg(username));
        return false;
    }

    QRegularExpression passwdRegex("^.{5,16}$");
    if(!passwdRegex.match(password).hasMatch())
    {
        QMessageBox::warning(nullptr,
                             tr("Input Error"),
                             tr("Password must be 5-16 characters long."));
        return false;
    }

    return true;
}