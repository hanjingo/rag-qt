#ifndef SETTINGPAGEHARDWARE_H
#define SETTINGPAGEHARDWARE_H

#include <QMap>
#include <QWidget>
#include <QStandardItemModel>

#include "../framework/Global.h"
#include "../framework/Config.h"

namespace Ui
{
class SettingPageHardware;
}

class SettingPageHardware : public QWidget
{
    Q_OBJECT

  public:
    explicit SettingPageHardware(QWidget *parent = nullptr);
    ~SettingPageHardware();

    static SettingPageHardware *Instance()
    {
        static SettingPageHardware inst;
        return &inst;
    };

  protected:
    void changeEvent(QEvent *event) override;

  signals:

  private slots:
    void _slotComboAudioTranslatorCurrentChanged(int iIndex);
    void _slotBtnSaveClicked();
    void _slotAsrConfigUpdate();

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();

    void _addAudioConfig(const QString &id);

    void _set(const Config::AsrParam &param);
    void _get(Config::AsrParam &param);

  private:
    Ui::SettingPageHardware *ui;
};

#endif // SETTINGPAGEHARDWARE_H
