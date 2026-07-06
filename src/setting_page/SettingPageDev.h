#ifndef SettingPageDev_H
#define SettingPageDev_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
class QButtonGroup;
QT_END_NAMESPACE

namespace Ui
{
class SettingPageDev;
}

class SettingPageDev : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageDev *Instance();

  protected:
    explicit SettingPageDev(QWidget *parent = nullptr);
    ~SettingPageDev();

    void _initUI();
    void _initConnections();
    void _retranslate();

  private:
    Ui::SettingPageDev    *ui;
    static SettingPageDev *m_stSettingPageDevInst;
};

#endif // SettingPageDev_H
