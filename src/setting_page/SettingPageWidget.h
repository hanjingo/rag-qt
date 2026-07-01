#ifndef SETTINGPAGEWIDGET_H
#define SETTINGPAGEWIDGET_H

#include <QMap>
#include <QWidget>
#include <QVector>

#include "Bus.h"
namespace Ui
{
class SettingPageWidget;
}

class SettingPageWidget : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageWidget *Instance();
    explicit SettingPageWidget(QWidget *parent = nullptr);
    ~SettingPageWidget();

  signals:

  private slots:
    void _slotTabCurrentChanged(int iIndex);

  private:
    Ui::SettingPageWidget    *ui;
    static SettingPageWidget *m_stMainSettingPageInst;
};

#endif // SETTINGPAGEWIDGET_H
