#include <QDirIterator>
#include <QFileInfo>
#include <QFileInfoList>

#include "PluginMgr.h"

PluginMgr *PluginMgr::m_stPluginMgrInst = nullptr;
PluginMgr *PluginMgr::Instance()
{
    if(nullptr == m_stPluginMgrInst)
        m_stPluginMgrInst = new PluginMgr();

    return m_stPluginMgrInst;
}

PluginMgr::PluginMgr(QObject *parent)
    : QObject(parent)
{
}

PluginMgr::~PluginMgr()
{
}

PluginInterface *PluginMgr::Get(const QString &pluginId)
{
    if(!m_mLoaders.contains(pluginId))
        return nullptr;

    QPluginLoader *loader = m_mLoaders.value(pluginId);
    return qobject_cast<PluginInterface *>(loader->instance());
}

PluginInterface *PluginMgr::Load(const QString &filePathName)
{
    QDir    dir     = QDir::current();
    QString absPath = dir.absoluteFilePath(filePathName);
    if(!QFile::exists(absPath))
        return nullptr;

    auto     loader = new QPluginLoader(absPath, this);
    QObject *plugin = loader->instance();
    if(!plugin)
    {
        qDebug() << "Failed to load plugin from file: " << absPath
                 << ". Error: " << loader->errorString();
        loader->deleteLater();
        return nullptr;
    }

    PluginInterface *intf = qobject_cast<PluginInterface *>(plugin);
    if(!intf)
    {
        loader->deleteLater();
        return nullptr;
    }

    if(m_mLoaders.contains(intf->Name()))
    {
        loader->deleteLater();
        return qobject_cast<PluginInterface *>(
            m_mLoaders.value(intf->Name())->instance());
    }

    m_mLoaders.insert(intf->Name(), loader);
    emit SignalPluginLoaded(intf, absPath);
    return intf;
}

void PluginMgr::Unload(const QString &plugin)
{
    // WARNING: Unloading plugins can be dangerous if there are
    //      still references to the plugin's objects in use.
    //      Ensure that all references to the plugin's objects
    //      are released before unloading.
    if(!m_mLoaders.contains(plugin))
        return;

    QPluginLoader *loader = m_mLoaders.value(plugin);
    loader->unload();
    loader->deleteLater();
    m_mLoaders.remove(plugin);
    emit SignalPluginUnloaded(plugin);
}

QStringList PluginMgr::Search(const QString &path, const FilterFunc &filter)
{
    QStringList result{};
    QDir        dir(path);
    if(!dir.exists())
        return result;

    // QStringList exts{"*.dll", "*.so", "*.dylib"};
    // for(const QString &file : dir.entryList(exts, QDir::Files))
    // {
    //     QString       absPath = dir.absoluteFilePath(file);
    //     QPluginLoader loader(absPath);

    //     QJsonObject metaData = loader.metaData().value("MetaData").toObject();
    //     if(filter(metaData))
    //         result.append(absPath);
    // }

    QStringList  exts{"*.dll", "*.so", "*.dylib"};
    QDirIterator it(dir.absolutePath(),
                    exts,
                    QDir::Files,
                    QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        it.next(); // skip the first empty file

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