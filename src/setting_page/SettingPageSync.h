#ifndef SETTINGPAGESYNC_H
#define SETTINGPAGESYNC_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
class QButtonGroup;
QT_END_NAMESPACE

namespace Ui
{
class SettingPageSync;
}

class SettingPageSync : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageSync *Instance();

  protected:
    explicit SettingPageSync(QWidget *parent = nullptr);
    ~SettingPageSync();

    void _initUI();
    void _initConnections();
    void _retranslate();

  private:
    Ui::SettingPageSync    *ui;
    static SettingPageSync *m_stSettingPageSyncInst;
};

#endif // SETTINGPAGESYNC_H
