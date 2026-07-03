#ifndef SETTINGPAGEHARDWARE_H
#define SETTINGPAGEHARDWARE_H

#include <QMap>
#include <QWidget>
#include <QStandardItemModel>

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
    void _slotComboAudioTranslatorCurrentChanged(int iIndex);
    void _slotBtnSaveClicked();
    void _slotBtnAddClicked();

  private:
    void _initUI();
    void _initConnections();

    void _switchAudioConfig(const QString &id);
    void _addAudioConfig(const QString &id);
    void _save();

  private:
    Ui::SettingPageHardware    *ui;
    static SettingPageHardware *m_stInstance;
};

#endif // SETTINGPAGEHARDWARE_H
