#ifndef SETTINGPAGESKILL_H
#define SETTINGPAGESKILL_H

#include <QMap>
#include <QWidget>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "Bus.h"

namespace Ui
{
class SettingPageSkill;
}

class SettingPageSkill : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageSkill *Instance();
    explicit SettingPageSkill(QWidget *parent = nullptr);
    ~SettingPageSkill();

  signals:

  private slots:

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();

  private:
  private:
    Ui::SettingPageSkill    *ui;
    static SettingPageSkill *m_stSettingPageSkillInst;
};

#endif // SettingPageSkill_H
