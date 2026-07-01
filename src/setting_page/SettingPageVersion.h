#ifndef SETTINGPAGEVERSION_H
#define SETTINGPAGEVERSION_H

#include <QMap>
#include <QWidget>

namespace Ui
{
class SettingPageVersion;
}

class SettingPageVersion : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageVersion *Instance();
    explicit SettingPageVersion(QWidget *parent = nullptr);
    ~SettingPageVersion();

  signals:

  private slots:

  private:
    Ui::SettingPageVersion    *ui;
    static SettingPageVersion *m_stSettingPageVersionInst;
};

#endif // SETTINGPAGEVERSION_H
