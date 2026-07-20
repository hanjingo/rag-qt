#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QTimer>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QHeaderView>
#include <QMessageBox>

#include <libqt/encoding/zipper.h>
#include <libqt/net/downloader.h>

#include "HomePageWidget.h"
#include "ui_HomePageWidget.h"

#include "NewSessionDialog.h"
#include "ui_NewSessionDialog.h"

#include "HistorySettingDialog.h"
#include "ui_HistorySettingDialog.h"

#include "SettingPageModel.h"

#include "PluginBtn.h"
#include "GrpcClient.h"
#include "StyleMgr.h"
#include "PluginMgr.h"
#include "Error.h"
#include "Account.h"
#include "Config.h"

HomePageWidget *HomePageWidget::m_stMainHomePageInst = nullptr;

HomePageWidget *HomePageWidget::GetMainHomePageInst()
{
    if(nullptr == m_stMainHomePageInst)
    {
        m_stMainHomePageInst = new HomePageWidget();
    }

    return m_stMainHomePageInst;
}

HomePageWidget::HomePageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePageWidget)
    , m_pPluginsBtnGroup(new QButtonGroup(this))
    , m_pSessionCtlBtnGroup(new QButtonGroup(this))
    , m_pHistoryModel(nullptr)
    , m_colNum(3)
    , m_maxRecord(100)
    , m_sortBy(HistorySettingDialog::SortBy::TimeDesc)
{
    ui->setupUi(this);

    _initPluginsArea();
    _initHistoryArea();
    _retranslate();
    _initConnections();
}

HomePageWidget::~HomePageWidget()
{
    delete m_pPluginsBtnGroup;
    m_pPluginsBtnGroup = nullptr;

    delete m_pSessionCtlBtnGroup;
    m_pSessionCtlBtnGroup = nullptr;

    delete ui;
}

QVector<Bus::Plugin> HomePageWidget::GetPluginInfos()
{
    QVector<Bus::Plugin> plugins;
    for(auto item : m_pPluginsBtnGroup->buttons())
    {
        if(!item)
            continue;

        PluginBtn *btn = qobject_cast<PluginBtn *>(item);
        if(!btn || btn->GetState() != PluginBtn::State::Installed)
            continue;

        Bus::Plugin plugin;
        plugin.hash      = btn->Hash();
        plugin.name      = btn->Name();
        plugin.desc      = btn->Desc();
        plugin.publisher = btn->Publisher();
        plugin.version   = btn->Version();
        plugin.timestamp = btn->Timestamp().toString("%Y-%m-%d %H:%M:%S");
        plugins.append(plugin);
    }
    return plugins;
}

void HomePageWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if(event->type() == QEvent::LanguageChange)
    {
        qDebug() << "HomePageWidget language change event received.";
        ui->retranslateUi(this);
        _retranslate();
    }
}

void HomePageWidget::_initPluginsArea()
{
    _clearPlugins();
}

void HomePageWidget::_initHistoryArea()
{
    // init filter edit
    ui->editFilter->setStyleSheet(StyleMgr::ParseFile(":/styles/line_edit"));
    ui->editFilter->setPlaceholderText(tr("Filter"));

    // init session control buttons
    ui->btnAdd->setIcon(QIcon(":/icons/add"));
    ui->btnAdd->setVisible(true);
    ui->btnDel->setIcon(QIcon(":/icons/del"));
    ui->btnDel->setVisible(true);
    ui->btnSetting->setIcon(QIcon(":/icons/settings"));
    ui->btnSetting->setVisible(true);
    m_pSessionCtlBtnGroup->addButton(ui->btnAdd, 0);
    m_pSessionCtlBtnGroup->addButton(ui->btnDel, 1);
    m_pSessionCtlBtnGroup->addButton(ui->btnSetting, 2);
    m_pSessionCtlBtnGroup->setExclusive(true);

    // init history table
    ui->tbviewHistory->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbviewHistory->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    if(nullptr == m_pHistoryModel)
    {
        m_pHistoryModel = new QStandardItemModel;
    } else
    {
        m_pHistoryModel->clear();
    }

    ui->tbviewHistory->setModel(m_pHistoryModel);
    ui->tbviewHistory->setVisible(true);
    ui->tbviewHistory->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tbviewHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
    _refreshSessionTable(true);
}

void HomePageWidget::_retranslate()
{
    ui->lblPluginsTitle->setText(tr("Plugins"));
    ui->lblHistoryTitle->setText(tr("History"));

    ui->editFilter->setPlaceholderText(tr("Filter"));
    _refreshSessionTable();
}

void HomePageWidget::_initConnections()
{
    connect(m_pPluginsBtnGroup,
            &QButtonGroup::buttonClicked,
            this,
            &HomePageWidget::_slotPluginBtnClicked);

    connect(m_pSessionCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &HomePageWidget::_slotSessionCtlBtnGroupClicked);

    connect(ui->editFilter,
            &QLineEdit::textChanged,
            this,
            &HomePageWidget::_slotEditFilterTextChanged);

    connect(PluginMgr::Instance(),
            &PluginMgr::SignalPluginUnloaded,
            this,
            &HomePageWidget::_slotPluginUnloaded);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGrpcConnected,
            this,
            &HomePageWidget::_slotGrpcConnected);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetSessionResp,
            this,
            &HomePageWidget::_slotGetSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalNewSessionResp,
            this,
            &HomePageWidget::_slotNewSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalModifySessionTitleResp,
            this,
            &HomePageWidget::_slotModifySessionTitleResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalDelSessionResp,
            this,
            &HomePageWidget::_slotDelSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetPluginInfoResp,
            this,
            &HomePageWidget::_slotGetPluginInfoResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalDownloadResp,
            this,
            &HomePageWidget::_slotDownloadResp);
}

void HomePageWidget::_slotPluginBtnClicked(QAbstractButton *pBtn)
{
    if(!pBtn || !pBtn->isCheckable())
    {
        qDebug() << "Plugin button can not be click";
        return;
    }

    qDebug() << "Plugin button clicked:" << pBtn;
    PluginBtn *pPluginBtn = qobject_cast<PluginBtn *>(pBtn);
    if(!pPluginBtn)
        return;

    QString hash    = pPluginBtn->Hash();
    int64_t user_id = Account::Instance()->Id();
    QString auth    = Account::Instance()->Auth();

    GrpcClient::Instance()->Download(hash, user_id, auth);
}

void HomePageWidget::_slotSessionCtlBtnGroupClicked(int id)
{
    qDebug() << "Session control button clicked, id: " << id;
    switch(id)
    {
        case 0: { // add session
            qDebug() << "Add session button clicked.";
            auto             confs = Config::Instance().modelConfigs();
            QVector<QString> models;
            for(const auto &conf : confs)
                models.append(conf.name);

            NewSessionDialog dlg(models, this);
            auto             result = dlg.exec();
            if(result == QDialog::Accepted)
            {
                qDebug() << "New session dialog accepted.";
                Bus::Session session;
                QString      model;
                QString      prompt;
                bool         isLocal;
                bool         isRemote;
                dlg.GetConfig(session, model, prompt, isLocal, isRemote);
                GrpcClient::Instance()->NewSession(Account::Instance()->Id(),
                                                   Account::Instance()->Auth(),
                                                   session.title,
                                                   prompt,
                                                   model);
            }
        }
        break;
        case 1: { // delete session
            qDebug() << "Delete session button clicked.";
            auto rows = ui->tbviewHistory->selectionModel()->selectedRows();
            QVector<int64_t> sessionIds;
            for(auto row : rows)
                sessionIds.append(row.siblingAtColumn(0).data().toLongLong());

            GrpcClient::Instance()->DelSession(Account::Instance()->Id(),
                                               Account::Instance()->Auth(),
                                               sessionIds);
        }
        break;
        case 2: { // session history settings
            qDebug() << "Session settings button clicked.";
            HistorySettingDialog dlg(this);
            auto                 result = dlg.exec();
            if(result == QDialog::Accepted)
            {
                m_maxRecord = dlg.MaxRecord();
                m_sortBy    = dlg.SortByType();
                GrpcClient::Instance()->GetSession(-1,
                                                   Account::Instance()->Id(),
                                                   Account::Instance()->Auth(),
                                                   m_maxRecord);
            }
        }
        break;
        default: {
            qDebug() << "Unknown session control button clicked.";
        }
        break;
    }
}

void HomePageWidget::_slotEditFilterTextChanged(const QString &content)
{
    qDebug() << "Filter text changed:" << content;
    _filterSessionTable(content);
}

void HomePageWidget::_slotGrpcConnected(const QString &address)
{
    qDebug() << "HomePageWidget connected to gRPC server at " << address;
}

void HomePageWidget::_slotGetSessionResp(const int                    errorCode,
                                         const QVector<Bus::Session> &sessions)
{
    qDebug() << "Get session response received with " << sessions.size()
             << " items.";

    m_pHistoryModel->clear();
    _addSessions(sessions);
    _refreshSessionTable();
}

void HomePageWidget::_slotNewSessionResp(const int           errorCode,
                                         const Bus::Session &session)
{
    qDebug() << "HomePageWidget: New session response received, session id: "
             << session.id;
    // For testing, just append the new session to history
    _addSessions({session});
    _refreshSessionTable();
}

void HomePageWidget::_slotModifySessionTitleResp(const int      errorCode,
                                                 const int64_t  id,
                                                 const QString &title)
{
    qDebug() << "Modify session title response received, session id: " << id
             << ", new title: " << title;
    // For testing, just update the title in history if session id matches
    for(int i = 0; i < m_pHistoryModel->rowCount(); i++)
    {
        QStandardItem *pIdItem = m_pHistoryModel->item(i, 0);
        if(pIdItem == nullptr || pIdItem->text().toLongLong() != id)
            continue;

        QStandardItem *pTitleItem = m_pHistoryModel->item(i, 2);
        if(pTitleItem != nullptr)
        {
            pTitleItem->setText(title);
            break;
        }
    }

    _refreshSessionTable();
}

void HomePageWidget::_slotDelSessionResp(const int               errorCode,
                                         const QVector<int64_t> &ids)
{
    qDebug() << "Delete session response received with " << ids.size()
             << " items.";

    // repull the session list from the server to refresh the history table
    GrpcClient::Instance()->GetSession(-1,
                                       Account::Instance()->Id(),
                                       Account::Instance()->Auth(),
                                       50);
}

void HomePageWidget::_slotGetPluginInfoResp(const int errorCode,
                                            const QVector<Bus::Plugin> &plugins)
{
    if(errorCode != OK)
    {
        qDebug() << "Error in getting plugin info, error code: " << errorCode;
        return;
    }

    qDebug() << "Get plugin info response received with " << plugins.size()
             << " items.";
    _clearPlugins();
    _addPlugins(plugins);
}

void HomePageWidget::_slotDownloadResp(const int      errorCode,
                                       const QString &hash,
                                       const QString &addr,
                                       const int64_t  size_kb)
{
    qDebug() << "Download response received, hash: " << hash
             << ", address: " << addr << ", size: " << size_kb << " KB";

    if(errorCode != OK)
        return;

    // handle downloaded plugin content, e.g. save to file and load into UI
    for(auto item : m_pPluginsBtnGroup->buttons())
    {
        PluginBtn *btn = qobject_cast<PluginBtn *>(item);
        if(!btn || btn->Hash() != hash)
            continue;

        btn->SetUrl(addr);
        btn->SetState(PluginBtn::State::WaitDownload);
        break;
    }
}

void HomePageWidget::_slotPluginBtnStateChanged(PluginBtn       *btn,
                                                PluginBtn::State state)
{
    qDebug() << "Plugin button state changed, hash: " << btn->Hash()
             << ", new state: " << static_cast<int>(state);
    // Handle plugin button state change, e.g. update UI or trigger actions
    switch(state)
    {
        case PluginBtn::State::Unknown: {
            qDebug() << "PluginBtn state is Unknown, hash: " << btn->Hash();
        }
        break;
        case PluginBtn::State::WaitDownload: {
            qDebug() << "PluginBtn is waiting to download, hash: "
                     << btn->Hash();
            _download(btn, btn->Url());
        }
        break;
        case PluginBtn::State::Downloading: {
            qDebug() << "PluginBtn is downloading, hash: " << btn->Hash();
        }
        break;
        case PluginBtn::State::Downloaded: {
            qDebug() << "PluginBtn has been downloaded, hash: " << btn->Hash();

            // start installing the plugin plugin
            QString zipPath = QString("%1/tmp/%2.zip")
                                  .arg(QCoreApplication::applicationDirPath())
                                  .arg(btn->Name());
            QString destDir = QString("%1/plugins/%2")
                                  .arg(QCoreApplication::applicationDirPath())
                                  .arg(btn->Name());
            Zipper  zipper{zipPath, this};
            if(!zipper.UnZip(destDir))
            {
                qDebug() << "Failed to unzip plugin plugin from " << zipPath
                         << " to " << destDir;
                QDir(destDir).removeRecursively(); // clean up
                QFile::remove(zipPath);            // remove the zip file
                btn->SetState(PluginBtn::State::Unknown);
                QMessageBox::critical(this,
                                      tr("Download Failed"),
                                      tr("Failed to download plugin plugin: %1")
                                          .arg(btn->Name()));
            } else
            {
                qDebug() << "Successfully unzipped plugin plugin to "
                         << destDir;
                btn->SetState(PluginBtn::State::Installing);
                QFile::remove(
                    zipPath); // remove the zip file after installation
            }
        }
        break;
        case PluginBtn::State::Installing: {
            qDebug() << "PluginBtn is installing, hash: " << btn->Hash();
            QString destDir = QString("%1/plugins/%2")
                                  .arg(QCoreApplication::applicationDirPath())
                                  .arg(btn->Name());
            // load the plugin
            auto plugins = PluginMgr::Instance()->Search(
                destDir,
                [](const QJsonObject &metaData) -> bool {
                    return metaData.contains("PluginId")
                           && metaData.contains("Version")
                           && metaData.contains("Name")
                           && metaData.contains("Description")
                           && metaData.contains("Author")
                           && metaData.contains("Organization")
                           && metaData.contains("Dependencies");
                });

            for(auto fpath : plugins)
            {
                qDebug() << "Found plugin file: " << fpath;
                PluginInterface *plugin = PluginMgr::Instance()->Load(fpath);
                if(plugin)
                {
                    qDebug()
                        << "Successfully loaded plugin plugin from " << destDir;
                    btn->SetState(PluginBtn::State::Installed);
                } else
                {
                    qDebug() << "Failed to load plugin plugin from " << destDir;
                    QDir(destDir).removeRecursively(); // clean up
                    btn->SetState(PluginBtn::State::
                                      Unknown); // reset state to allow retry
                    QMessageBox::critical(
                        this,
                        tr("Installation Failed"),
                        tr("Failed to install plugin plugin: %1")
                            .arg(btn->Name()));
                }
            }
        }
        break;
        case PluginBtn::State::Installed: {
            qDebug() << "PluginBtn is installed, hash: " << btn->Hash();
        }
        break;
        default:
            break;
    }
}

void HomePageWidget::_slotPluginUnloaded(const QString &pluginId,
                                         const QString &pluginName)
{
    qDebug() << "HomePageWidget::_slotPluginUnloaded with pluginId: "
             << pluginId << ", pluginName: " << pluginName;

    // remove the plugin button associated with the unloaded plugin
    for(auto item : m_pPluginsBtnGroup->buttons())
    {
        PluginBtn *btn = qobject_cast<PluginBtn *>(item);
        if(!btn || btn->Name() != pluginName)
            continue;

        qDebug() << "Removing plugin button for unloaded plugin: "
                 << pluginName;
        btn->Reset();
        break;
    }
}

void HomePageWidget::_addSessions(const QVector<Bus::Session> &sessions)
{
    if(m_pHistoryModel == nullptr)
        return;

    int n_row = m_pHistoryModel->rowCount();
    for(int i = 0; i < sessions.size(); i++)
    {
        const auto &item = sessions.at(i);

        // ID
        auto *idItem = new QStandardItem;
        idItem->setData(QVariant::fromValue<qlonglong>(item.id),
                        Qt::DisplayRole);
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable); // uneditable
        m_pHistoryModel->setItem(n_row, 0, idItem);

        // TimeStamp
        auto *tmItem = new QStandardItem(item.timestamp);
        tmItem->setFlags(tmItem->flags() & ~Qt::ItemIsEditable);
        m_pHistoryModel->setItem(n_row, 1, tmItem);

        // Title
        m_pHistoryModel->setItem(n_row, 2, new QStandardItem(item.title));
        n_row++;
    }
}

void HomePageWidget::_delSessions(const QVector<int64_t> &sessionIds)
{
    if(m_pHistoryModel == nullptr)
        return;

    for(int i = m_pHistoryModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pIdItem = m_pHistoryModel->item(i, 0);
        if(pIdItem == nullptr)
            continue;

        int64_t id = pIdItem->data(Qt::DisplayRole).toLongLong();
        if(sessionIds.contains(id))
        {
            qDebug() << "delete session id = " << id;
            m_pHistoryModel->removeRow(i);
        }
    }
}

void HomePageWidget::_refreshSessionTable(bool clearFirst)
{
    if(m_pHistoryModel == nullptr)
        return;

    if(clearFirst)
        m_pHistoryModel->clear();

    ui->tbviewHistory->setModel(m_pHistoryModel);
    m_pHistoryModel->setColumnCount(3);
    m_pHistoryModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    m_pHistoryModel->setHeaderData(1, Qt::Horizontal, tr("Date Time"));
    m_pHistoryModel->setHeaderData(2, Qt::Horizontal, tr("Title"));
    switch(m_sortBy)
    {
        case HistorySettingDialog::SortBy::TimeAsc: {
            m_pHistoryModel->sort(1, Qt::SortOrder::AscendingOrder);
        }
        break;
        case HistorySettingDialog::SortBy::TimeDesc: {
            m_pHistoryModel->sort(1, Qt::SortOrder::DescendingOrder);
        }
        break;
        default:
            break;
    }

    ui->tbviewHistory->hideColumn(0); // hide session id column
}

void HomePageWidget::_filterSessionTable(const QString &filterText)
{
    if(m_pHistoryModel == nullptr)
        return;

    for(int i = 0; i < m_pHistoryModel->rowCount(); i++)
    {
        QStandardItem *pTitleItem = m_pHistoryModel->item(i, 2);
        if(pTitleItem == nullptr)
            continue;

        bool match =
            pTitleItem->text().contains(filterText, Qt::CaseInsensitive);
        ui->tbviewHistory->setRowHidden(i, !match);
    }
}

int HomePageWidget::_addPlugins(const QVector<Bus::Plugin> &plugins)
{
    int idx = 0;
    foreach(const Bus::Plugin &item, plugins)
    {
        PluginBtn *btn = new PluginBtn(ui->scrollAreaPlugins);
        btn->SetHash(item.hash);
        btn->SetName(item.name);
        btn->SetDesc(item.desc);
        btn->SetPublisher(item.publisher);
        btn->SetVersion(item.version);
        btn->SetTimestamp(item.timestamp);
        btn->SetState(PluginBtn::State::Unknown);
        connect(btn,
                &PluginBtn::SignalStateChanged,
                this,
                &HomePageWidget::_slotPluginBtnStateChanged);

        // if the plugin is already installed a higher version,
        // reset the button state to installed
        auto plugin = PluginMgr::Instance()->GetByName(item.name);
        if(plugin)
        {
            auto oldVer = QVersionNumber::fromString(plugin->Version());
            auto newVer = QVersionNumber::fromString(item.version);
            if(oldVer >= newVer)
            {
                qDebug() << "Plugin:" << item.name
                         << " is already installed a higher version:"
                         << plugin->Version();

                btn->SetVersion(plugin->Version());
                btn->SetState(PluginBtn::State::Installed);
            }
        }

        m_pPluginsBtnGroup->addButton(btn);
        idx++;
    }

    _drawPluginsArea();
    return idx;
}

void HomePageWidget::_getPlugins(QVector<Bus::Plugin>              &plugins,
                                 std::function<bool(Bus::Plugin &)> filter)
{
    for(auto item : m_pPluginsBtnGroup->buttons())
    {
        if(!item)
            continue;

        PluginBtn *btn = qobject_cast<PluginBtn *>(item);
        if(!btn)
            continue;


        Bus::Plugin plugin;
        plugin.hash      = btn->Hash();
        plugin.name      = btn->Name();
        plugin.desc      = btn->Desc();
        plugin.publisher = btn->Publisher();
        plugin.version   = btn->Version();
        plugin.timestamp = btn->Timestamp().toString("%Y-%m-%d %H:%M:%S");
        if(!filter(plugin))
            continue;

        plugins.append(plugin);
    }
}

void HomePageWidget::_clearPlugins()
{
    auto buttons = m_pPluginsBtnGroup->buttons();
    for(auto btn : buttons)
    {
        m_pPluginsBtnGroup->removeButton(btn);
        ui->gridPluginScroll->removeWidget(btn);
        btn->deleteLater();
    }

    _drawPluginsArea();
}

void HomePageWidget::_drawPluginsArea()
{
    qDebug() << "Drawing plugins area with "
             << m_pPluginsBtnGroup->buttons().count() << " buttons.";
    int width  = 150;
    width      = qMax(width, ui->scrollAreaPlugins->width() / m_colNum - 20);
    int height = width; // make plugin button square
    for(int i = 0; i < m_pPluginsBtnGroup->buttons().count(); i++)
    {
        QAbstractButton *item = m_pPluginsBtnGroup->buttons().at(i);
        if(!item)
            continue;

        PluginBtn *pBtn = qobject_cast<PluginBtn *>(item);
        if(!pBtn)
            continue;

        pBtn->Resize(width, height);
        qDebug() << "Adding plugin button to grid layout at row: "
                 << i / m_colNum << ", column: " << i % m_colNum;
        ui->gridPluginScroll->addWidget(pBtn, i / m_colNum, i % m_colNum);
    }
}

void HomePageWidget::_download(PluginBtn *btn, const QUrl &url)
{
    auto saveFilePath = QString("%1/tmp/%2.zip")
                            .arg(QCoreApplication::applicationDirPath())
                            .arg(btn->Name());
    btn->Download(url, saveFilePath);
}