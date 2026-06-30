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

struct NetworkConfig
{
    QString ip;
    int     port;
    bool    isEnable;
};

class SettingPageNetwork : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageNetwork *Instance();
    QVector<NetworkConfig>     GetNetworkConfigs();

  signals:
    void SignalSwitchAccount();

  protected:
    explicit SettingPageNetwork(QWidget *parent = nullptr);
    ~SettingPageNetwork();

  private slots:
    void _slotPong(const int64_t timestamp);
    void _slotBtnSaveClicked();
    void _slotBtnNetTestClicked();
    void _slotNetConfigCkGroupClicked(int id);

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _saveConfigFiles();
    void _loadConfigFiles();
    void _testNetwork();
    void _resetDelayValues();

  private:
    Ui::SettingPageNetwork    *ui;
    static SettingPageNetwork *m_stSettingPageNetworkInst;

    QButtonGroup *m_pNetConfigCkGroup;

    int m_lastConfigCkboxId = -1;
};

#endif // SETTINGPAGENETWORK_H
