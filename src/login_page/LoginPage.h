#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>

#include "ui_LoginPage.h"

QT_BEGIN_NAMESPACE
class FrameworkWidget;
QT_END_NAMESPACE

namespace Ui
{
class AccountMgrPage;
}

class LoginPage : public QWidget
{
    Q_OBJECT

  public:
    static LoginPage *Instance();

    void Login();

  signals:
    void SignalLogin(const QString &username, const QString &password);
    void SignalRegister(const QString &username, const QString &password);
    void SignalLogout();

  private slots:
    void _slotBtnLoginClicked();
    void _slotBtnRegisterClicked();
    void _slotBtnLogoutClicked();

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    bool _validateInput(const QString &username, const QString &password);

  protected:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();

  private:
    Ui::LoginPage    *ui;
    static LoginPage *m_stLoginPageInst;
};
#endif // LOGINPAGE_H
