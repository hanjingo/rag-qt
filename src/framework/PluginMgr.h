#ifndef PLUGINMGR_H
#define PLUGINMGR_H

#include <memory>
#include <QObject>
#include <QDir>
#include <QVector>
#include <QPluginLoader>

#include "PluginInterface.h"

class PluginMgr : public QObject
{
    Q_OBJECT

  public:
    explicit PluginMgr(QObject *parent = nullptr);
    ~PluginMgr();

    static PluginMgr *GetPluginMgrInst();
    PluginInterface  *Load(const QString &filePathName);
    void              Unload(const QString &plugin);

  signals:
    void SignalPluginLoaded(const PluginInterface *plugin);
    void SignalPluginUnloaded(const QString &pluginId);

  private:
    static PluginMgr              *m_stPluginMgrInst;
    QMap<QString, QPluginLoader *> m_mLoaders;
};

#endif // PLUGINMGR_H