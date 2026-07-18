#include "Bus.h"
#include "GrpcClient.h"

#include "Error.h"
#include "StyleMgr.h"

#include "SettingPagePlugin.h"
#include "ui_SettingPagePlugin.h"

#include "PluginConfigDialog.h"
#include "PluginMgr.h"

SettingPagePlugin *SettingPagePlugin::m_stSettingPagePluginInst = nullptr;

SettingPagePlugin *SettingPagePlugin::Instance()
{
    if(nullptr == m_stSettingPagePluginInst)
    {
        m_stSettingPagePluginInst = new SettingPagePlugin();
    }

    return m_stSettingPagePluginInst;
}

SettingPagePlugin::SettingPagePlugin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPagePlugin)
    , m_pPluginCtlBtnGroup(new QButtonGroup(this))
    , m_pPluginListModel(nullptr)
{
    ui->setupUi(this);

    _initUI();
    _initConnections();
    _retranslate();
}

SettingPagePlugin::~SettingPagePlugin()
{
    delete m_pPluginListModel;
    m_pPluginListModel = nullptr;

    delete ui;
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
    connect(PluginMgr::Instance(),
            &PluginMgr::SignalPluginLoaded,
            this,
            &SettingPagePlugin::_slotPluginLoaded);

    connect(PluginMgr::Instance(),
            &PluginMgr::SignalPluginUnloaded,
            this,
            &SettingPagePlugin::_slotPluginUnloaded);

    connect(m_pPluginCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &SettingPagePlugin::_slotPluginCtlBtnClicked);
}

void SettingPagePlugin::_slotPluginCtlBtnClicked(int id)
{
    qDebug() << "Control bar button clicked, id:" << id;
    switch(id)
    {
        case 0: // add plugin
        {
            qDebug() << "Add plugin button clicked.";
            QString            addr;
            PluginConfigDialog dlg{addr};
            auto               result = dlg.exec();
            if(result == QDialog::Accepted)
            {
                dlg.GetAddr(addr);
                emit PluginMgr::Instance() -> Load(addr);
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
                emit PluginMgr::Instance() -> Unload(name);
            }
        }
        break;
        case 2: // setting plugin
        {
            qDebug() << "Setting plugin button clicked.";
        }
        break;
        default:
            break;
    }
}

void SettingPagePlugin::_slotPluginLoaded(PluginInterface *plugin,
                                          const QString   &filePath)
{
    qDebug() << "_slotPluginLoaded";
    QString name    = plugin->Name();
    QString version = plugin->Version();
    _addPlugins(name, version, filePath, "Loaded");
}

void SettingPagePlugin::_slotPluginUnloaded(const QString &pluginId)
{
    qDebug() << "_slotPluginUnloaded";
    _delPlugins({pluginId});
}

void SettingPagePlugin::_initUI()
{
    // init filter edit
    ui->editFilter->setStyleSheet(StyleMgr::ParseFile(":/styles/line_edit"));
    ui->editFilter->setText(tr("Filter"));

    // init model control buttons
    ui->btnAdd->setIcon(QIcon(":/icons/add"));
    ui->btnAdd->setVisible(true);
    ui->btnDel->setIcon(QIcon(":/icons/del"));
    ui->btnDel->setVisible(true);
    ui->btnSetting->setIcon(QIcon(":/icons/settings"));
    ui->btnSetting->setVisible(true);
    m_pPluginCtlBtnGroup->addButton(ui->btnAdd, 0);
    m_pPluginCtlBtnGroup->addButton(ui->btnDel, 1);
    m_pPluginCtlBtnGroup->addButton(ui->btnSetting, 2);
    m_pPluginCtlBtnGroup->setExclusive(true);

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
    m_pPluginListModel->setColumnCount(3);
    m_pPluginListModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
    m_pPluginListModel->setHeaderData(1, Qt::Horizontal, tr("Version"));
    m_pPluginListModel->setHeaderData(2, Qt::Horizontal, tr("Addr"));
    m_pPluginListModel->setHeaderData(3, Qt::Horizontal, tr("Tag"));
}

void SettingPagePlugin::_addPlugins(const QString &name,
                                    const QString &version,
                                    const QString &filepath,
                                    const QString &tag)
{
    if(m_pPluginListModel == nullptr)
        return;

    int n_row = m_pPluginListModel->rowCount();
    m_pPluginListModel->setItem(n_row, 0, new QStandardItem(name));
    m_pPluginListModel->setItem(n_row, 1, new QStandardItem(version));
    m_pPluginListModel->setItem(n_row, 2, new QStandardItem(filepath));
    m_pPluginListModel->setItem(n_row, 3, new QStandardItem(tag));
}

void SettingPagePlugin::_delPlugins(const QVector<QString> &names)
{
    if(m_pPluginListModel == nullptr)
        return;

    for(int i = m_pPluginListModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pIdItem = m_pPluginListModel->item(i, 0);
        if(pIdItem == nullptr)
            continue;

        auto name = pIdItem->text();
        if(names.contains(name))
            m_pPluginListModel->removeRow(i);
    }
}