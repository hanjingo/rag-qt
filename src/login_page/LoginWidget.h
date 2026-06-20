#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

#include "LoginPage.h"

namespace Ui
{
class LoginWidget;
class LoginPage;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

  public:
    static LoginWidget *Instance();

  signals:
    void SignalLogin(const QString &username, const QString &password);
    void SignalRegister(const QString &username, const QString &password);
    void SignalLogout();

  private:
    void _initConnections();

  protected:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

  private:
    Ui::LoginWidget    *ui;
    static LoginWidget *m_stLoginWgtInst;

    LoginPage *m_pLoginPageInst;
};
#endif // LOGINWIDGET_H
