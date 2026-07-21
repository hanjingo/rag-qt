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

#include <libqt/net/uploader.h>

namespace Ui
{
class SettingPagePlugin;
}

class SettingPagePlugin : public QWidget
{
    Q_OBJECT

  public:
    explicit SettingPagePlugin(QWidget *parent = nullptr);
    ~SettingPagePlugin();

    static SettingPagePlugin *Instance()
    {
        static SettingPagePlugin inst;
        return &inst;
    }

    void erase(Uploader *uploader)
    {
        if(!m_pUploaders.contains(uploader))
            return;

        m_pUploaders.remove(uploader);
        uploader->deleteLater();
    }

    void setDeveloperMode(bool isDeveloper);

  protected:
    void changeEvent(QEvent *event) override;

  private slots:
    void _slotPluginLoaded(PluginInterface *plugin, const QString &filePath);
    void _slotPluginUnloaded(const QString &pluginId,
                             const QString &pluginName);
    void _slotGetPluginInfoResp(const int                   errorCode,
                                const QVector<Bus::Plugin> &plugins);

    void _slotPluginCtlBtnClicked(int id);
    void _slotEditFilterTextChanged(const QString &content);

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _refreshPluginTable(bool clearFirst = false);
    void _addPlugins(const QVector<Bus::Plugin> &plugins, const QString &tag);
    void _delPlugins(const QVector<QString> &names);
    void _filtePluginTable(const QString &filterText);
    Bus::Plugin _findStagedPlugin(const QString &name, const QString &version);
    void        _upload(const QString &filePath);

  private:
    Ui::SettingPagePlugin *ui;

    QButtonGroup        *m_pPluginCtlBtnGroup;
    QStandardItemModel  *m_pPluginListModel;
    QSet<Uploader *>     m_pUploaders;
    QVector<Bus::Plugin> m_stagedPlugins;
};

#endif // SettingPagePlugin_H
