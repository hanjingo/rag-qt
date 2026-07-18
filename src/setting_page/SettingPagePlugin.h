#ifndef SETTINGPAGEPLUGIN_H
#define SETTINGPAGEPLUGIN_H

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
class SettingPagePlugin;
}

class SettingPagePlugin : public QWidget
{
    Q_OBJECT

  public:
    static SettingPagePlugin *Instance();
    explicit SettingPagePlugin(QWidget *parent = nullptr);
    ~SettingPagePlugin();

  protected:
    void changeEvent(QEvent *event) override;

  private slots:
    void _slotPluginLoaded(PluginInterface *plugin, const QString &filePath);
    void _slotPluginUnloaded(const QString &pluginId);

    void _slotPluginCtlBtnClicked(int id);

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _refreshPluginTable(bool clearFirst = false);
    void _addPlugins(const QString &name,
                     const QString &version,
                     const QString &filepath,
                     const QString &tag);
    void _delPlugins(const QVector<QString> &hashs);

  private:
  private:
    Ui::SettingPagePlugin    *ui;
    static SettingPagePlugin *m_stSettingPagePluginInst;

    QButtonGroup *m_pPluginCtlBtnGroup;

    QStandardItemModel *m_pPluginListModel;
};

#endif // SettingPagePlugin_H
