#ifndef SETTINGPAGEVERSION_H
#define SETTINGPAGEVERSION_H

#include <QMap>
#include <QWidget>

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

    static SettingPageVersion *Instance()
    {
        static SettingPageVersion inst;
        return &inst;
    }

  signals:

  private slots:
    void _slotLoginResp(const int      errorCode,
                        const int64_t  user_id,
                        const QString &auth,
                        const int32_t  privilege,
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
