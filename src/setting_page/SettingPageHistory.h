#ifndef SETTINGPAGEHISTORY_H
#define SETTINGPAGEHISTORY_H

#include <QMap>
#include <QWidget>

namespace Ui
{
class SettingPageHistory;
}

class SettingPageHistory : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageHistory *GetSettingPageHistoryInst();
    explicit SettingPageHistory(QWidget *parent = nullptr);
    ~SettingPageHistory();

  signals:

  private slots:

  private:
    Ui::SettingPageHistory    *ui;
    static SettingPageHistory *m_stSettingPageHistoryInst;
};

#endif // SETTINGPAGEHISTORY_H
