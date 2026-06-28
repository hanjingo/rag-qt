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

  protected:
    explicit SettingPageNetwork(QWidget *parent = nullptr);
    ~SettingPageNetwork();

  private slots:
    void _slotPong(const int64_t timestamp);
    void _slotBtnSaveClicked();
    void _slotBtnNetTestClicked();

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _saveConfigFiles();
    void _loadConfigFiles();

  private:
    Ui::SettingPageNetwork    *ui;
    static SettingPageNetwork *m_stSettingPageNetworkInst;
};

#endif // SETTINGPAGENETWORK_H
