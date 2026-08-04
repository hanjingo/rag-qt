#ifndef SETTINGPAGENETWORK_H
#define SETTINGPAGENETWORK_H

#include <QWidget>
#include <QPointer>
#include <QTimer>

#include "Config.h"

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
    static QPointer<SettingPageNetwork> instance()
    {
        static QPointer<SettingPageNetwork> inst = new SettingPageNetwork();
        return inst;
    }
    QVector<Config::NetworkConfig> GetNetworkConfigs();

  signals:
    void signalSwitchAccount();

  protected:
    explicit SettingPageNetwork(QWidget *parent = nullptr);
    ~SettingPageNetwork();

  private slots:
    void _slotPong(const int64_t timestamp);
    void _slotBtnSaveClicked();
    void _slotBtnNetTestClicked();
    void _slotNetConfigCkGroupClicked(int id);
    void _slotHeartbeatTimeout();

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _saveConfigFiles();
    void _loadConfigFiles();
    void _testNetwork();
    void _resetDelayValues();

  private:
    Ui::SettingPageNetwork *ui;

    QTimer       *m_pTimer;
    QButtonGroup *m_pNetConfigCkGroup;
    int           m_lastConfigCkboxId = -1;
};

#endif // SETTINGPAGENETWORK_H
