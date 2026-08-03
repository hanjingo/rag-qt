#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QPointer>

#include "LoginPage.h"

namespace Ui
{
class LoginWidget;
class AccountMgrPage;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

  public:
    static QPointer<LoginWidget> instance()
    {
        static QPointer<LoginWidget> inst = new LoginWidget();
        return inst;
    }

  signals:
    void signalLogin(const QString &username, const QString &password);
    void signalRegister(const QString &username, const QString &password);
    void signalLogout();

  private slots:
    void _slotRegisterResp(const int errorCode, const int64_t user_id);

  private:
    void _initConfig();
    void _initUI();
    void _initConnections();
    void _retranslate();

  protected:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

  private:
    Ui::LoginWidget *ui;
    LoginPage       *m_pLoginPageInst;
};
#endif // LOGINWIDGET_H
