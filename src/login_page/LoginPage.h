#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QPointer>

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
    static QPointer<LoginPage> instance()
    {
        static QPointer<LoginPage> inst = new LoginPage();
        return inst;
    }

    void Login();

  protected:
    void changeEvent(QEvent *event) override;

  signals:
    void signalLogin(const QString &username, const QString &password);
    void signalRegister(const QString &username, const QString &password);
    void signalLogout();

  private slots:
    void _slotBtnLoginClicked();
    void _slotBtnRegisterClicked();
    void _slotBtnLogoutClicked();

  private:
    void    _initUI();
    void    _initConnections();
    void    _retranslate();
    bool    _validateInput(const QString &username, const QString &password);
    QString _encryptPassword(const QString &password);

  protected:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();

  private:
    Ui::LoginPage *ui;
};
#endif // LOGINPAGE_H
