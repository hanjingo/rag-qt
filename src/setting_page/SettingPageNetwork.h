#ifndef SETTINGPAGENETWORK_H
#define SETTINGPAGENETWORK_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
class QButtonGroup;
QT_END_NAMESPACE

namespace Ui
{
class SettingPageNetwork;
}

class SettingPageNetwork : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageNetwork *GetSettingPageNetworkInst();

  protected:
    explicit SettingPageNetwork(QWidget *parent = nullptr);
    ~SettingPageNetwork();

    void _initUI();
    void _initConnections();
    void _retranslate();

  private:
    Ui::SettingPageNetwork    *ui;
    static SettingPageNetwork *m_stSettingPageNetworkInst;
};

#endif // SETTINGPAGENETWORK_H
