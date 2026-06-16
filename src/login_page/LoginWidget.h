#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class FrameworkWidget;
QT_END_NAMESPACE

namespace Ui
{
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

  public:
    static LoginWidget *GetLoginWgtInst();

  signals:
    void SignalLogin(const QString &username, const QString &password);
    void SignalRegister(const QString &username, const QString &password);
    void SignalLogout();

  private slots:
    void _slotBtnLoginClicked();
    void _slotBtnRegisterClicked();
    void _slotBtnLogoutClicked();

  private:
    void _initConnections();

  protected:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

  private:
    Ui::LoginWidget    *ui;
    static LoginWidget *m_stLoginWgtInst;
};
#endif // LOGINWIDGET_H
