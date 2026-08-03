#ifndef PLUGINMGR_H
#define PLUGINMGR_H

#include <memory>
#include <functional>
#include <QObject>
#include <QDir>
#include <QVector>
#include <QPluginLoader>
#include <QJsonObject>
#include <QPointer>

#include "PluginInterface.h"

class PluginMgr : public QObject
{
    Q_OBJECT

  public:
    using FilterFunc = std::function<bool(const QJsonObject &metaData)>;

  public:
    explicit PluginMgr(QObject *parent = nullptr);
    ~PluginMgr();

    static QPointer<PluginMgr> instance()
    {
        static QPointer<PluginMgr> inst = new PluginMgr();
        return inst;
    }

    static QMap<QString, QString> Parse(const QString &absDllPath);

    PluginInterface *Get(const QString &pluginId);
    PluginInterface *GetByName(const QString &pluginName);
    PluginInterface *Load(const QString &filePathName);
    void             Unload(const QString &pluginId);
    QStringList      Search(
        const QString    &path,
        const FilterFunc &filter = [](const QJsonObject &) { return true; });
    void Clear();

  signals:
    void signalPluginLoaded(PluginInterface *plugin, const QString &filePath);
    void signalPluginUnloaded(const QString &pluginId,
                              const QString &pluginName);

  private:
    QMap<QString, QPluginLoader *> m_mLoaders;
};

#endif // PLUGINMGR_H