#include "PluginMgr.h"

PluginMgr *PluginMgr::m_stPluginMgrInst = nullptr;
PluginMgr *PluginMgr::GetPluginMgrInst()
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
        loader->deleteLater();
        return nullptr;
    }

    PluginInterface *intf = qobject_cast<PluginInterface *>(plugin);
    if(!intf)
    {
        loader->deleteLater();
        return nullptr;
    }

    if(m_mLoaders.contains(intf->Id()))
    {
        loader->deleteLater();
        return qobject_cast<PluginInterface *>(
            m_mLoaders.value(intf->Id())->instance());
    }

    m_mLoaders.insert(intf->Id(), loader);
    emit SignalPluginLoaded(intf);
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