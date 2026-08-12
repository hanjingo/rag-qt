#include <libqt/io/file.h>

#include <hj/encoding/fmt.hpp>

#include <QTimer>
#include <QMessageBox>

#include "Bus.h"
#include "GrpcClient.h"

#include "Account.h"
#include "Error.h"
#include "StyleMgr.h"

#include "SettingPagePlugin.h"
#include "ui_SettingPagePlugin.h"

#include "PluginConfigDialog.h"
#include "PluginUploadDialog.h"
#include "PluginMgr.h"

SettingPagePlugin::SettingPagePlugin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPagePlugin)
    , m_pPluginCtlBtnGroup(new QButtonGroup(this))
    , m_pPluginListModel(new QStandardItemModel(this))
    , m_pUploaders()
{
    ui->setupUi(this);

    _initConnections();
    _initUI();
    _retranslate();
}

SettingPagePlugin::~SettingPagePlugin()
{
}

void SettingPagePlugin::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if(event->type() == QEvent::LanguageChange)
    {
        qDebug() << "SettingPagePlugin language change event received.";
        ui->retranslateUi(this);
        _retranslate();
    }
}

void SettingPagePlugin::_initConnections()
{
    connect(PluginMgr::instance(),
            &PluginMgr::signalPluginLoaded,
            this,
            &SettingPagePlugin::_slotPluginLoaded);

    connect(PluginMgr::instance(),
            &PluginMgr::signalPluginUnloaded,
            this,
            &SettingPagePlugin::_slotPluginUnloaded);

    connect(GrpcClient::instance(),
            &GrpcClient::signalGetPluginInfoResp,
            this,
            &SettingPagePlugin::_slotGetPluginInfoResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalUploadResp,
            this,
            &SettingPagePlugin::_slotUploadResp);

    connect(m_pPluginCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &SettingPagePlugin::_slotPluginCtlBtnClicked);

    connect(ui->editFilter,
            &QLineEdit::textChanged,
            this,
            &SettingPagePlugin::_slotEditFilterTextChanged);
}

void SettingPagePlugin::_slotPluginCtlBtnClicked(int id)
{
    qDebug() << "Control bar button clicked, id:" << id;
    switch(id)
    {
        case 0: // add plugin
        {
            qDebug() << "Add plugin button clicked.";
            Bus::Plugin        conf;
            PluginConfigDialog dlg{conf};
            auto               result = dlg.exec();
            if(result == QDialog::Accepted)
            {
                conf = dlg.GetConfig();
                if(conf.filePath.isEmpty())
                {
                    qDebug() << "Plugin address is empty, cannot load plugin.";
                    QMessageBox::warning(
                        this,
                        tr("Load Plugin"),
                        tr("Plugin address is empty, cannot load plugin."));
                    return;
                }
                emit PluginMgr::instance() -> Load(conf.filePath);
            }
        }
        break;
        case 1: // del plugin
        {
            qDebug() << "Del plugin button clicked.";
            auto rows = ui->tbviewPlugin->selectionModel()->selectedRows();
            QVector<QString> names;
            for(auto row : rows)
                names.append(row.siblingAtColumn(0).data().toString());

            for(auto name : names)
            {
                auto plugin = PluginMgr::instance()->GetByName(name);
                if(plugin)
                    emit PluginMgr::instance() -> Unload(plugin->Id());
            }
        }
        break;
        case 2: // upload plugin
        {
            qDebug() << "Upload plugin button clicked.";
            Bus::Plugin conf;

            auto rows = ui->tbviewPlugin->selectionModel()->selectedRows();
            // if(rows.isEmpty())
            // {
            //     qDebug() << "No plugin selected for upload.";
            //     QMessageBox::warning(this,
            //                          tr("Upload Plugin"),
            //                          tr("Please select a plugin to upload."));
            //     return;
            // }
            for(auto row : rows)
            {
                conf.name      = row.siblingAtColumn(0).data().toString();
                conf.version   = row.siblingAtColumn(1).data().toString();
                conf.timestamp = row.siblingAtColumn(2).data().toString();
                conf.platform  = row.siblingAtColumn(3).data().toInt();
                conf.publisher = row.siblingAtColumn(4).data().toString();
                conf.hash      = row.siblingAtColumn(5).data().toString();
                conf.filePath  = row.siblingAtColumn(6).data().toString();
                conf.desc      = row.siblingAtColumn(7).data().toString();
                break;
            }

            PluginUploadDialog dlg{conf};
            auto               result = dlg.exec();
            conf                      = dlg.GetConfig();
            auto packedFilePath       = dlg.GetPackedFilePath();
            if(result != QDialog::Accepted)
            {
                qDebug() << "Plugin upload dialog canceled.";
                return;
            }

            if(packedFilePath.isEmpty())
            {
                qDebug() << "Packed file path is empty, cannot upload plugin.";
                QMessageBox::warning(
                    this,
                    tr("Upload Plugin"),
                    tr("Packed file path is empty, cannot upload plugin."));
                return;
            }
            _upload(packedFilePath, conf);
        }
        break;
        default:
            break;
    }
}

void SettingPagePlugin::_slotEditFilterTextChanged(const QString &content)
{
    qDebug() << "Filter text changed:" << content;
    _filtePluginTable(content);
}

void SettingPagePlugin::_slotPluginLoaded(PluginInterface *plugin,
                                          const QString   &filePath)
{
    qDebug() << "_slotPluginLoaded";
    QString name    = plugin->Name();
    QString version = plugin->Version();

    auto conf     = _findStagedPlugin(name, version);
    conf.name     = name;
    conf.version  = version;
    conf.filePath = filePath;
    _addPlugins({conf}, "Loaded");
}

void SettingPagePlugin::_slotPluginUnloaded(const QString &pluginId,
                                            const QString &pluginName)
{
    qDebug() << "SettingPagePlugin::_slotPluginUnloaded with pluginId: "
             << pluginId << ", pluginName: " << pluginName;

    _delPlugins({pluginName});
}

void SettingPagePlugin::_slotGetPluginInfoResp(
    const int errorCode, const QVector<Bus::Plugin> &plugins)
{
    qDebug() << "Get plugin info response received with " << plugins.size()
             << " items.";
    m_stagedPlugins.clear();
    for(auto plugin : plugins)
    {
        qDebug() << "Get Remote Plugin Info Name: " << plugin.name
                 << ", Hash: " << plugin.hash << ", Version: " << plugin.version
                 << ", Publisher: " << plugin.publisher
                 << ", Platform: " << plugin.platform
                 << ", Description: " << plugin.desc;
        m_stagedPlugins.append(plugin);
    }

    // attach staged plugins to the table with tag "Staged"
    for(int i = 0; i < m_pPluginListModel->rowCount(); i++)
    {
        QStandardItem *pTagItem = m_pPluginListModel->item(i, 8);
        if(pTagItem == nullptr || pTagItem->text() != "Loaded")
            continue;

        QStandardItem *pNameItem = m_pPluginListModel->item(i, 0);
        QStandardItem *pVerItem  = m_pPluginListModel->item(i, 1);
        if(!pNameItem || !pVerItem)
            continue;

        auto name    = pNameItem->text();
        auto version = pVerItem->text();
        auto conf    = _findStagedPlugin(name, version);
        if(conf.name.isEmpty())
            continue;

        m_pPluginListModel->setItem(i, 0, new QStandardItem(conf.name));
        m_pPluginListModel->setItem(i, 1, new QStandardItem(conf.version));
        m_pPluginListModel->setItem(i, 2, new QStandardItem(conf.timestamp));
        m_pPluginListModel->setItem(i, 3, new QStandardItem(conf.platform));
        m_pPluginListModel->setItem(i, 4, new QStandardItem(conf.publisher));
        m_pPluginListModel->setItem(i, 5, new QStandardItem(conf.hash));
        m_pPluginListModel->setItem(i, 7, new QStandardItem(conf.desc));
    }
}

void SettingPagePlugin::_slotUploadResp(const int      errorCode,
                                        const QString &hash)
{
    qDebug() << "SettingPagePlugin::_slotUploadResp with errorCode:"
             << errorCode << ", hash:" << hash;
    if(errorCode != 0)
        return;

    if(m_uploadingPlugin.hash == hash)
    {
        // publish
        // [{topic:"topic1", ...}, {topic:"topic2", ...}, ...]
        QJsonArray  arr;
        QJsonObject obj;
        obj["topic"]     = TOPIC_PLUGIN_PUB;
        obj["hash"]      = m_uploadingPlugin.hash;
        obj["name"]      = m_uploadingPlugin.name;
        obj["desc"]      = m_uploadingPlugin.desc;
        obj["publisher"] = m_uploadingPlugin.publisher;
        obj["version"]   = m_uploadingPlugin.version;
        obj["timestamp"] = m_uploadingPlugin.timestamp;
        obj["platform"]  = m_uploadingPlugin.platform;
        arr.append(obj);
        QJsonDocument doc(arr);
        auto str = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

        GrpcClient::instance()->Publish(Account::instance()->id(),
                                        Account::instance()->auth(),
                                        QVector<QString>(str));
        qDebug() << "Plugin uploaded and published with hash: "
                 << m_uploadingPlugin.hash
                 << ", name: " << m_uploadingPlugin.name
                 << ", desc: " << m_uploadingPlugin.desc
                 << ", publisher: " << m_uploadingPlugin.publisher
                 << ", version: " << m_uploadingPlugin.version
                 << ", timestamp: " << m_uploadingPlugin.timestamp
                 << ", platform: " << m_uploadingPlugin.platform;

        // update plugin info 1000ms after
        QTimer::singleShot(1000, this, []() {
            // query plugin info after publish plugin
            GrpcClient::instance()->GetPluginInfo();
        });
    }
}

void SettingPagePlugin::_upload(const QString     &filePath,
                                const Bus::Plugin &conf)
{
    qDebug() << "SettingPagePlugin::_slotUpload with filePath: " << filePath;
    auto urls = Config::instance()->getPluginUploadUrls();
    if(urls.isEmpty())
    {
        qDebug() << "No plugin upload URL configured.";
        QMessageBox::critical(this,
                              tr("Upload Error"),
                              tr("No plugin upload URL configured."));
        return;
    }

    auto url      = QUrl(urls.first());
    auto uploader = new Uploader(this);
    if(!uploader)
    {
        qDebug() << "Failed to create Uploader instance.";
        QMessageBox::critical(this,
                              tr("Upload Error"),
                              tr("Failed to create Uploader instance."));
        return;
    }

    m_uploadingPlugin = conf;
    connect(uploader,
            &Uploader::signalUploadFinished,
            this,
            [uploader, conf, url](bool           success,
                                  const QString &filePath,
                                  const QString &response) {
                SettingPagePlugin::instance()->erase(uploader);

                if(success)
                {
                    qDebug() << "Upload successful, response: " << response;
                    QString       downloadUrl;
                    QJsonDocument doc =
                        QJsonDocument::fromJson(response.toUtf8());
                    if(!doc.isNull() && doc.isObject())
                    {
                        QJsonObject jsonObj = doc.object();
                        QJsonArray  arr     = jsonObj["files"].toArray();
                        for(auto item : arr)
                        {
                            if(item.isObject()
                               && item.toObject().contains("download_url"))
                                downloadUrl =
                                    item.toObject()["download_url"].toString();
                        }
                    }

                    // notify server
                    GrpcClient::instance()->Upload(conf.hash,
                                                   Account::instance()->id(),
                                                   Account::instance()->auth(),
                                                   downloadUrl,
                                                   File::fileSizeKB(filePath));
                    qDebug()
                        << "notify server upload file filePath:" << filePath
                        << ", downloadUrl:" << downloadUrl
                        << ", hash:" << conf.hash;

                    // show message box
                    QMessageBox::information(
                        SettingPagePlugin::instance(),
                        QObject::tr("Upload Successful"),
                        QObject::tr("Plugin uploaded successfully."));
                } else
                {
                    qDebug() << "Upload failed, response: " << response;
                    QMessageBox::critical(
                        SettingPagePlugin::instance(),
                        QObject::tr("Upload Failed"),
                        QObject::tr("Plugin upload failed. Response: %1")
                            .arg(response));
                }
            });
    connect(uploader,
            &Uploader::signalUploadError,
            this,
            [uploader, filePath](const QString &errorString) {
                qDebug() << "Upload error for file: " << filePath
                         << ", error: " << errorString;
                SettingPagePlugin::instance()->erase(uploader);
            });
    m_pUploaders.insert(uploader);
    uploader->upload(url, filePath);
}

void SettingPagePlugin::_initUI()
{
    // init filter edit
    ui->editFilter->setStyleSheet(StyleMgr::ParseFile(":/styles/line_edit"));
    ui->editFilter->setPlaceholderText(tr("Filter"));

    // init model control buttons
    ui->btnAdd->setIcon(QIcon(":/icons/add"));
    ui->btnAdd->setVisible(true);
    ui->btnDel->setIcon(QIcon(":/icons/del"));
    ui->btnDel->setVisible(true);
    ui->btnUpload->setIcon(QIcon(":/icons/upload"));
    ui->btnUpload->setVisible(true);
    m_pPluginCtlBtnGroup->addButton(ui->btnAdd, 0);
    m_pPluginCtlBtnGroup->addButton(ui->btnDel, 1);
    m_pPluginCtlBtnGroup->addButton(ui->btnUpload, 2);
    m_pPluginCtlBtnGroup->setExclusive(true);
    for(auto btn : m_pPluginCtlBtnGroup->buttons())
        btn->setStyleSheet(
            StyleMgr::ParseFile(":/styles/tbview_header_push_button"));

    // init model table
    ui->tbviewPlugin->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->tbviewPlugin->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    if(nullptr == m_pPluginListModel)
        m_pPluginListModel = new QStandardItemModel;
    else
        m_pPluginListModel->clear();

    ui->tbviewPlugin->setModel(m_pPluginListModel);
    ui->tbviewPlugin->setVisible(true);
    ui->tbviewPlugin->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tbviewPlugin->setSelectionBehavior(QAbstractItemView::SelectRows);
    _refreshPluginTable(true);

    _retranslate();
}

void SettingPagePlugin::_retranslate()
{
    _refreshPluginTable();
}

void SettingPagePlugin::_refreshPluginTable(bool clearFirst)
{
    if(m_pPluginListModel == nullptr)
        return;

    if(clearFirst)
        m_pPluginListModel->clear();

    ui->tbviewPlugin->setModel(m_pPluginListModel);
    m_pPluginListModel->setColumnCount(9);
    m_pPluginListModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
    m_pPluginListModel->setHeaderData(1, Qt::Horizontal, tr("Version"));
    m_pPluginListModel->setHeaderData(2, Qt::Horizontal, tr("Timestamp"));
    m_pPluginListModel->setHeaderData(3, Qt::Horizontal, tr("Platform"));
    m_pPluginListModel->setHeaderData(4, Qt::Horizontal, tr("Publisher"));
    m_pPluginListModel->setHeaderData(5, Qt::Horizontal, tr("Hash"));
    m_pPluginListModel->setHeaderData(6, Qt::Horizontal, tr("Address"));
    m_pPluginListModel->setHeaderData(7, Qt::Horizontal, tr("Desc"));
    m_pPluginListModel->setHeaderData(8, Qt::Horizontal, tr("Tag"));
}

void SettingPagePlugin::_addPlugins(const QVector<Bus::Plugin> &plugins,
                                    const QString              &tag)
{
    if(m_pPluginListModel == nullptr)
        return;

    for(auto plugin : plugins)
    {
        qDebug() << "Plugin Name: " << plugin.name << ", Hash: " << plugin.hash
                 << ", Version: " << plugin.version
                 << ", Publisher: " << plugin.publisher
                 << ", Platform: " << plugin.platform
                 << ", addr: " << plugin.filePath
                 << ", Description: " << plugin.desc;
        if(plugin.publisher != Account::instance()->name())
            continue;

        int n_row = m_pPluginListModel->rowCount();
        m_pPluginListModel->setItem(n_row, 0, new QStandardItem(plugin.name));
        m_pPluginListModel->setItem(n_row,
                                    1,
                                    new QStandardItem(plugin.version));
        m_pPluginListModel->setItem(n_row,
                                    2,
                                    new QStandardItem(plugin.timestamp));
        m_pPluginListModel->setItem(
            n_row,
            3,
            new QStandardItem(QString::number(plugin.platform)));
        m_pPluginListModel->setItem(n_row,
                                    4,
                                    new QStandardItem(plugin.publisher));
        m_pPluginListModel->setItem(n_row, 5, new QStandardItem(plugin.hash));
        m_pPluginListModel->setItem(n_row,
                                    6,
                                    new QStandardItem(plugin.filePath));
        m_pPluginListModel->setItem(n_row, 7, new QStandardItem(plugin.desc));
        m_pPluginListModel->setItem(n_row, 8, new QStandardItem(tag));
    }
}

void SettingPagePlugin::_delPlugins(const QVector<QString> &names)
{
    if(m_pPluginListModel == nullptr)
        return;

    for(int i = m_pPluginListModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pNameItem = m_pPluginListModel->item(i, 0);
        if(pNameItem == nullptr)
            continue;

        auto name = pNameItem->text();
        if(names.contains(name))
        {
            auto addr = m_pPluginListModel->item(i, 6)->text();
            m_pPluginListModel->removeRow(i);

            // remove from local
            qDebug() << "Removing plugin file at address: " << addr;
            QTimer::singleShot(500,
                               [addr]() { File::removeIfExists(addr, true); });
        }
    }
}

void SettingPagePlugin::_filtePluginTable(const QString &filterText)
{
    if(m_pPluginListModel == nullptr)
        return;

    for(int i = 0; i < m_pPluginListModel->rowCount(); i++)
    {
        QStandardItem *pNameItem = m_pPluginListModel->item(i, 0);
        if(pNameItem == nullptr)
            continue;

        bool match =
            pNameItem->text().contains(filterText, Qt::CaseInsensitive);
        ui->tbviewPlugin->setRowHidden(i, !match);
    }
}

Bus::Plugin SettingPagePlugin::_findStagedPlugin(const QString &name,
                                                 const QString &version)
{
    for(auto plugin : m_stagedPlugins)
    {
        if(plugin.name == name && plugin.version == version)
            return plugin;
    }

    return Bus::Plugin{};
}