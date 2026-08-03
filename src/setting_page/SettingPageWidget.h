#ifndef SETTINGPAGEWIDGET_H
#define SETTINGPAGEWIDGET_H

#include <QMap>
#include <QWidget>
#include <QVector>
#include <QPointer>

#include "Bus.h"
namespace Ui
{
class SettingPageWidget;
}

class SettingPageWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit SettingPageWidget(QWidget *parent = nullptr);
    ~SettingPageWidget();

    static QPointer<SettingPageWidget> instance()
    {
        static QPointer<SettingPageWidget> inst = new SettingPageWidget();
        return inst;
    }

  protected:
    void changeEvent(QEvent *event) override;

  signals:

  private slots:
    void _slotTabCurrentChanged(int iIndex);
    void _slotLoginResp(const int      errorCode,
                        const int64_t  user_id,
                        const QString &auth,
                        const QString &account,
                        const QString &lastLoginTime,
                        const bool     isForceUpdate);

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();

  private:
    Ui::SettingPageWidget *ui;
};

#endif // SETTINGPAGEWIDGET_H
