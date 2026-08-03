#ifndef SETTINGPAGEVERSION_H
#define SETTINGPAGEVERSION_H

#include <QMap>
#include <QWidget>
#include <QPointer>

#include "GrpcClient.h"

namespace Ui
{
class SettingPageVersion;
}

class SettingPageVersion : public QWidget
{
    Q_OBJECT

  public:
    explicit SettingPageVersion(QWidget *parent = nullptr);
    ~SettingPageVersion();

    static QPointer<SettingPageVersion> instance()
    {
        static QPointer<SettingPageVersion> inst = new SettingPageVersion();
        return inst;
    }

  signals:

  private slots:
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
    Ui::SettingPageVersion *ui;
};

#endif // SETTINGPAGEVERSION_H
