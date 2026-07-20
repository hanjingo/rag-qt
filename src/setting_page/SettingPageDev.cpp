#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QButtonGroup>
#include <QMessageBox>

#include "Account.h"
#include "GrpcClient.h"

#include "SettingPageDev.h"
#include "ui_SettingPageDev.h"

SettingPageDev::SettingPageDev(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageDev)
    , m_pPluginListModel(new QStandardItemModel(this))
{
    ui->setupUi(this);

    _initUI();
    _initConnections();
    _retranslate();
}

SettingPageDev::~SettingPageDev()
{
    delete ui;
}

void SettingPageDev::_initUI()
{
    // init left side bar
    ui->listWgtCatalog->setViewMode(QListView::ListMode);

    // --------------- plugin page -----------
    QListWidgetItem *itemPluginMgr = new QListWidgetItem(tr("Plugin Manager"));
    itemPluginMgr->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    ui->listWgtCatalog->insertItem(0, itemPluginMgr);

    ui->tbviewPlugin->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->tbviewPlugin->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    ui->tbviewPlugin->setModel(m_pPluginListModel);
    ui->tbviewPlugin->setVisible(true);
    ui->tbviewPlugin->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tbviewPlugin->setSelectionBehavior(QAbstractItemView::SelectRows);
    _refreshPluginTable(true);

    // ------------------ model page --------------
    QListWidgetItem *itemModel = new QListWidgetItem(tr("Model Manager"));
    itemModel->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    ui->listWgtCatalog->insertItem(1, itemModel);

    // ------------------ memory page --------------
    QListWidgetItem *itemMemory = new QListWidgetItem(tr("Memory Manager"));
    itemMemory->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    ui->listWgtCatalog->insertItem(2, itemMemory);

    ui->listWgtCatalog->setCurrentRow(0);
}

void SettingPageDev::_retranslate()
{
}

void SettingPageDev::_initConnections()
{
    connect(ui->listWgtCatalog,
            &QListWidget::currentRowChanged,
            this,
            &SettingPageDev::_slotCatalogChanged);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetPluginInfoResp,
            this,
            &SettingPageDev::_slotGetPluginInfoResp);

    connect(ui->editFilterPlugin,
            &QLineEdit::textChanged,
            this,
            &SettingPageDev::_slotEditFilterPluginTextChanged);
}

void SettingPageDev::_slotCatalogChanged(int index)
{
    switch(index)
    {
        case 0: // plugin
        {
            GrpcClient::Instance()->GetPluginInfo("",
                                                  Account::Instance()->Name(),
                                                  50);
        }
        break;
        case 1: // model
        {
        }
        break;
        case 2: // memory
        {
        }
        break;
        default: // do nothing
        {
            qDebug() << "Unknown catalog index: " << index;
        }
        break;
    }

    ui->stackPage->setCurrentIndex(index);
}

void SettingPageDev::_slotGetPluginInfoResp(const int errorCode,
                                            const QVector<Bus::Plugin> &plugins)
{
    qDebug() << "Get plugin info response received with " << plugins.size()
             << " items.";
    _clearPluginRecords();
    _addPluginRecords(plugins, "Staged");
}

void SettingPageDev::_slotEditFilterPluginTextChanged(const QString &content)
{
    qDebug() << "Filter plugin text changed:" << content;
    _filtePluginTable(content);
}

void SettingPageDev::_refreshPluginTable(bool clearFirst)
{
    if(m_pPluginListModel == nullptr)
        return;

    if(clearFirst)
        m_pPluginListModel->clear();

    ui->tbviewPlugin->setModel(m_pPluginListModel);
    m_pPluginListModel->setColumnCount(7);
    m_pPluginListModel->setHeaderData(0, Qt::Horizontal, tr("Hash"));
    m_pPluginListModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    m_pPluginListModel->setHeaderData(2, Qt::Horizontal, tr("Version"));
    m_pPluginListModel->setHeaderData(3, Qt::Horizontal, tr("Timestamp"));
    m_pPluginListModel->setHeaderData(4, Qt::Horizontal, tr("Platform"));
    m_pPluginListModel->setHeaderData(5, Qt::Horizontal, tr("Desc"));
    m_pPluginListModel->setHeaderData(6, Qt::Horizontal, tr("Tag"));

    ui->tbviewPlugin->hideColumn(1); // hide hash column
}

void SettingPageDev::_addPluginRecords(const QVector<Bus::Plugin> &plugins,
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
                 << ", Description: " << plugin.desc;
        if(plugin.publisher != Account::Instance()->Name())
            continue;

        int n_row = m_pPluginListModel->rowCount();
        m_pPluginListModel->setItem(n_row, 0, new QStandardItem(plugin.hash));
        m_pPluginListModel->setItem(n_row, 1, new QStandardItem(plugin.name));
        m_pPluginListModel->setItem(n_row,
                                    2,
                                    new QStandardItem(plugin.version));
        m_pPluginListModel->setItem(n_row,
                                    3,
                                    new QStandardItem(plugin.timestamp));
        m_pPluginListModel->setItem(n_row,
                                    4,
                                    new QStandardItem(plugin.platform));
        m_pPluginListModel->setItem(n_row, 5, new QStandardItem(plugin.desc));
        m_pPluginListModel->setItem(n_row, 6, new QStandardItem(tag));
    }
}

void SettingPageDev::_delPluginRecords(const QVector<QString> &hashs)
{
    if(m_pPluginListModel == nullptr)
        return;

    for(int i = m_pPluginListModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pHashItem = m_pPluginListModel->item(i, 0);
        if(pHashItem == nullptr)
            continue;

        auto hash = pHashItem->text();
        if(!hashs.contains(hash))
            continue;

        auto tag = m_pPluginListModel->item(i, 6)->text();
        if(tag == "Staged")
        {
            QMessageBox::critical(this,
                                  tr("Delete Failed"),
                                  tr("Cannot delete staged plugin."));
            continue;
        }

        m_pPluginListModel->removeRow(i);
    }
}

void SettingPageDev::_clearPluginRecords()
{
    if(m_pPluginListModel == nullptr)
        return;

    m_pPluginListModel->removeRows(0, m_pPluginListModel->rowCount());
}

void SettingPageDev::_filtePluginTable(const QString &filterText)
{
    if(m_pPluginListModel == nullptr)
        return;

    for(int i = 0; i < m_pPluginListModel->rowCount(); i++)
    {
        QStandardItem *pNameItem = m_pPluginListModel->item(i, 1);
        if(pNameItem == nullptr)
            continue;

        bool match =
            pNameItem->text().contains(filterText, Qt::CaseInsensitive);
        ui->tbviewPlugin->setRowHidden(i, !match);
    }
}