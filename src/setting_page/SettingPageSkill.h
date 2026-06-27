#ifndef SETTINGPAGESKILL_H
#define SETTINGPAGESKILL_H

#include <QMap>
#include <QWidget>
#include <QStandardItemModel>
#include <QButtonGroup>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "Bus.h"
#include "PluginMgr.h"

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

  protected:
    void changeEvent(QEvent *event) override;

  private slots:
    void _slotPluginLoaded(PluginInterface *plugin, const QString &filePath);
    void _slotPluginUnloaded(const QString &pluginId);

    void _slotSkillCtlBtnClicked(int id);

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _refreshSkillTable(bool clearFirst = false);
    void _addSkills(const QString &name,
                    const QString &version,
                    const QString &filepath,
                    const QString &tag);
    void _delSkills(const QVector<QString> &hashs);

  private:
  private:
    Ui::SettingPageSkill    *ui;
    static SettingPageSkill *m_stSettingPageSkillInst;

    QButtonGroup *m_pSkillCtlBtnGroup;

    QStandardItemModel *m_pSkillListModel;
};

#endif // SettingPageSkill_H
