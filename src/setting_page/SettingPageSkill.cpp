#include "Bus.h"
#include "GrpcClient.h"

#include "Error.h"
#include "StyleMgr.h"

#include "SettingPageSkill.h"
#include "ui_SettingPageSkill.h"

#include "SkillCOnfigDialog.h"
#include "PluginMgr.h"

SettingPageSkill *SettingPageSkill::m_stSettingPageSkillInst = nullptr;

SettingPageSkill *SettingPageSkill::Instance()
{
    if(nullptr == m_stSettingPageSkillInst)
    {
        m_stSettingPageSkillInst = new SettingPageSkill();
    }

    return m_stSettingPageSkillInst;
}

SettingPageSkill::SettingPageSkill(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageSkill)
    , m_pSkillCtlBtnGroup(new QButtonGroup(this))
    , m_pSkillListModel(nullptr)
{
    ui->setupUi(this);

    _initUI();
    _initConnections();
    _retranslate();
}

SettingPageSkill::~SettingPageSkill()
{
    delete m_pSkillListModel;
    m_pSkillListModel = nullptr;

    delete ui;
}

void SettingPageSkill::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if(event->type() == QEvent::LanguageChange)
    {
        qDebug() << "SettingPageSkill language change event received.";
        ui->retranslateUi(this);
        _retranslate();
    }
}

void SettingPageSkill::_initConnections()
{
    connect(PluginMgr::Instance(),
            &PluginMgr::SignalPluginLoaded,
            this,
            &SettingPageSkill::_slotPluginLoaded);

    connect(PluginMgr::Instance(),
            &PluginMgr::SignalPluginUnloaded,
            this,
            &SettingPageSkill::_slotPluginUnloaded);

    connect(m_pSkillCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &SettingPageSkill::_slotSkillCtlBtnClicked);
}

void SettingPageSkill::_slotSkillCtlBtnClicked(int id)
{
    qDebug() << "Control bar button clicked, id:" << id;
    switch(id)
    {
        case 0: // add skill
        {
            qDebug() << "Add skill button clicked.";
            QString           addr;
            SkillConfigDialog dlg{addr};
            auto              result = dlg.exec();
            if(result == QDialog::Accepted)
            {
                dlg.GetAddr(addr);
                emit PluginMgr::Instance() -> Load(addr);
            }
        }
        break;
        case 1: // del skill
        {
            qDebug() << "Del skill button clicked.";
            auto rows = ui->tbviewSkill->selectionModel()->selectedRows();
            QVector<QString> names;
            for(auto row : rows)
                names.append(row.siblingAtColumn(0).data().toString());

            for(auto name : names)
            {
                emit PluginMgr::Instance() -> Unload(name);
            }
        }
        break;
        case 2: // setting skill
        {
            qDebug() << "Setting skill button clicked.";
        }
        break;
        default:
            break;
    }
}

void SettingPageSkill::_slotPluginLoaded(PluginInterface *plugin,
                                         const QString   &filePath)
{
    qDebug() << "_slotPluginLoaded";
    QString name    = plugin->Name();
    QString version = plugin->Version();
    _addSkills(name, version, filePath, "Loaded");
}

void SettingPageSkill::_slotPluginUnloaded(const QString &pluginId)
{
    qDebug() << "_slotPluginUnloaded";
    _delSkills({pluginId});
}

void SettingPageSkill::_initUI()
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
    m_pSkillCtlBtnGroup->addButton(ui->btnAdd, 0);
    m_pSkillCtlBtnGroup->addButton(ui->btnDel, 1);
    m_pSkillCtlBtnGroup->addButton(ui->btnSetting, 2);
    m_pSkillCtlBtnGroup->setExclusive(true);

    // init model table
    ui->tbviewSkill->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->tbviewSkill->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    if(nullptr == m_pSkillListModel)
        m_pSkillListModel = new QStandardItemModel;
    else
        m_pSkillListModel->clear();

    ui->tbviewSkill->setModel(m_pSkillListModel);
    ui->tbviewSkill->setVisible(true);
    ui->tbviewSkill->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tbviewSkill->setSelectionBehavior(QAbstractItemView::SelectRows);
    _refreshSkillTable(true);

    _retranslate();
}

void SettingPageSkill::_retranslate()
{
    _refreshSkillTable();
}

void SettingPageSkill::_refreshSkillTable(bool clearFirst)
{
    if(m_pSkillListModel == nullptr)
        return;

    if(clearFirst)
        m_pSkillListModel->clear();

    ui->tbviewSkill->setModel(m_pSkillListModel);
    m_pSkillListModel->setColumnCount(3);
    m_pSkillListModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
    m_pSkillListModel->setHeaderData(1, Qt::Horizontal, tr("Version"));
    m_pSkillListModel->setHeaderData(2, Qt::Horizontal, tr("Addr"));
    m_pSkillListModel->setHeaderData(3, Qt::Horizontal, tr("Tag"));
}

void SettingPageSkill::_addSkills(const QString &name,
                                  const QString &version,
                                  const QString &filepath,
                                  const QString &tag)
{
    if(m_pSkillListModel == nullptr)
        return;

    int n_row = m_pSkillListModel->rowCount();
    m_pSkillListModel->setItem(n_row, 0, new QStandardItem(name));
    m_pSkillListModel->setItem(n_row, 1, new QStandardItem(version));
    m_pSkillListModel->setItem(n_row, 2, new QStandardItem(filepath));
    m_pSkillListModel->setItem(n_row, 3, new QStandardItem(tag));
}

void SettingPageSkill::_delSkills(const QVector<QString> &names)
{
    if(m_pSkillListModel == nullptr)
        return;

    for(int i = m_pSkillListModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pIdItem = m_pSkillListModel->item(i, 0);
        if(pIdItem == nullptr)
            continue;

        auto name = pIdItem->text();
        if(names.contains(name))
            m_pSkillListModel->removeRow(i);
    }
}