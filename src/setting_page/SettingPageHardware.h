#ifndef SETTINGPAGEHARDWARE_H
#define SETTINGPAGEHARDWARE_H

#include <QMap>
#include <QWidget>

namespace Ui
{
class SettingPageHardware;
}

class SettingPageHardware : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageHardware *Instance();
    explicit SettingPageHardware(QWidget *parent = nullptr);
    ~SettingPageHardware();

  signals:

  private slots:

  private:
    Ui::SettingPageHardware    *ui;
    static SettingPageHardware *m_stInstance;
};

#endif // SETTINGPAGEHARDWARE_H
