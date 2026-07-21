#include <QDirIterator>
#include <QFileInfo>
#include <QFileInfoList>
#include <QCoreApplication>
#include <QVersionNumber>

#include "PluginMgr.h"

PluginMgr::PluginMgr(QObject *parent)
    : QObject(parent)
{
}

PluginMgr::~PluginMgr()
{
    QStringList pluginIds;
    for(auto key : m_mLoaders.keys())
        pluginIds.append(key);

    for(auto id : pluginIds)
        Unload(id);
}

QMap<QString, QString> PluginMgr::Parse(const QString &absDllPath)
{
    QMap<QString, QString> result;
    if(!QFile::exists(absDllPath))
        return result;

    QPluginLoader loader(absDllPath);
    if(loader.metaData().value("MetaData").isNull())
        return result;

    QJsonObject metaData = loader.metaData().value("MetaData").toObject();
    if(metaData.contains("PluginId"))
        result.insert("PluginId", metaData.value("PluginId").toString());

    if(metaData.contains("Version"))
        result.insert("Version", metaData.value("Version").toString());

    if(metaData.contains("Name"))
        result.insert("Name", metaData.value("Name").toString());

    if(metaData.contains("Icon"))
        result.insert("Icon", metaData.value("Icon").toString());

    if(metaData.contains("Platform"))
        result.insert("Platform",
                      QString::number(metaData.value("Platform").toInt()));

    if(metaData.contains("Description"))
        result.insert("Description", metaData.value("Description").toString());

    if(metaData.contains("Author"))
        result.insert("Author", metaData.value("Author").toString());

    if(metaData.contains("Organization"))
        result.insert("Organization",
                      metaData.value("Organization").toString());

    return result;
}

PluginInterface *PluginMgr::Get(const QString &pluginId)
{
    if(!m_mLoaders.contains(pluginId))
        return nullptr;

    QPluginLoader *loader = m_mLoaders.value(pluginId);
    return qobject_cast<PluginInterface *>(loader->instance());
}

PluginInterface *PluginMgr::GetByName(const QString &pluginName)
{
    for(auto loader : m_mLoaders)
    {
        PluginInterface *intf =
            qobject_cast<PluginInterface *>(loader->instance());
        if(intf && intf->Name() == pluginName)
            return intf;
    }
    return nullptr;
}

PluginInterface *PluginMgr::Load(const QString &filePathName)
{
    QDir    dir     = QDir::current();
    QString absPath = dir.absoluteFilePath(filePathName);
    if(!QFile::exists(absPath))
        return nullptr;

    auto loader = new QPluginLoader(absPath, this);
    // check plugin instance
    QObject *plugin = loader->instance();
    if(!plugin)
    {
        qDebug() << "Failed to load plugin from file: " << absPath
                 << ". Error: " << loader->errorString();
        loader->deleteLater();
        return nullptr;
    }

    // check interface
    PluginInterface *intf = qobject_cast<PluginInterface *>(plugin);
    if(!intf)
    {
        qDebug() << "Failed to load plugin from file: " << absPath
                 << ", interface null!";
        loader->deleteLater();
        return nullptr;
    }

    // check if already loaded
    if(m_mLoaders.contains(intf->Id()))
    {
        loader->deleteLater();
        return qobject_cast<PluginInterface *>(
            m_mLoaders.value(intf->Id())->instance());
    }

    // unload the old version plugin
    QJsonObject metaData  = loader->metaData().value("MetaData").toObject();
    auto        name      = metaData.value("Name").toString();
    auto        oldPlugin = GetByName(name);
    if(oldPlugin)
    {
        auto           newVersion = metaData.value("Version").toString();
        auto           oldVersion = oldPlugin->Version();
        auto           oldId      = oldPlugin->Id();
        QVersionNumber newVer     = QVersionNumber::fromString(newVersion);
        QVersionNumber oldVer     = QVersionNumber::fromString(oldVersion);
        if(newVer > oldVer)
        {
            // new version is greater, unload the old version and load the new one
            qDebug() << "Unload old plugin before load plugin from file: "
                     << absPath;
            Unload(oldId);
        }
    }

    m_mLoaders.insert(intf->Id(), loader);
    emit SignalPluginLoaded(intf, absPath);
    return intf;
}

void PluginMgr::Unload(const QString &pluginId)
{
    // WARNING: Unloading plugins can be dangerous if there are
    //      still references to the plugin's objects in use.
    //      Ensure that all references to the plugin's objects
    //      are released before unloading.
    if(!m_mLoaders.contains(pluginId))
        return;

    QPluginLoader *loader = m_mLoaders.value(pluginId);
    auto           plugin = qobject_cast<PluginInterface *>(loader->instance());
    QString        pluginName;
    if(plugin)
    {
        pluginName = plugin->Name();
        plugin->Shutdown();
    }

    loader->unload();
    loader->setParent(nullptr);
    loader->deleteLater();
    m_mLoaders.remove(pluginId);
    emit SignalPluginUnloaded(pluginId, pluginName);
}

QStringList PluginMgr::Search(const QString &path, const FilterFunc &filter)
{
    QStringList result{};
    QDir        dir(path);
    if(!dir.exists())
        return result;

    QStringList exts;
#ifdef Q_OS_WIN
    exts << "*.dll";
#elif defined(Q_OS_LINUX)
    exts << "*.so";
#elif defined(Q_OS_MAC)
    exts << "*.dylib";
#endif
    // QStringList  exts{"*.dll", "*.so", "*.dylib"};
    QDirIterator it(dir.absolutePath(),
                    exts,
                    QDir::Files,
                    QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        it.next();

        QString       fpath = it.fileInfo().absoluteFilePath();
        QPluginLoader loader(fpath);
        if(loader.metaData().value("MetaData").isNull())
            continue;

        QJsonObject metaData = loader.metaData().value("MetaData").toObject();
        if(filter(metaData))
            result.append(it.filePath());
    }

    return result;
}