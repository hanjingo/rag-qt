#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QButtonGroup>
#include <QMessageBox>
#include <QFileDialog>

#include <hj/ai/vector_index.hpp>
#include <hj/algo/uuid.hpp>

#include <libqt/io/filechunker.h>
#include <libqt/io/file.h>

#include "StyleMgr.h"
#include "SettingPageMemory.h"
#include "ui_SettingPageMemory.h"

#include "Global.h"
#include "GrpcClient.h"
#include "Error.h"
#include "Account.h"
#include "BusAdapter.h"
#include "Config.h"
#include "MemMgr.h"

SettingPageMemory::SettingPageMemory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageMemory)
    , m_pMemCtlBtnGroup(new QButtonGroup(this))
    , m_pMemListModel(nullptr)
{
    ui->setupUi(this);

    _initConnections();
    _initUI();
}

SettingPageMemory::~SettingPageMemory()
{
}

void SettingPageMemory::_initUI()
{
    // init filter edit
    ui->editFilter->setStyleSheet(StyleMgr::ParseFile(":/styles/line_edit"));
    ui->editFilter->setPlaceholderText(tr("Filter"));

    // init model control buttons
    ui->btnAdd->setIcon(QIcon(":/icons/add"));
    ui->btnAdd->setVisible(true);
    ui->btnDel->setIcon(QIcon(":/icons/del"));
    ui->btnDel->setVisible(true);
    ui->btnSetting->setIcon(QIcon(":/icons/settings"));
    ui->btnSetting->setVisible(true);
    ui->btnSave->setIcon(QIcon(":/icons/save"));
    ui->btnSave->setVisible(true);
    ui->btnImport->setIcon(QIcon(":/icons/import"));
    ui->btnImport->setVisible(true);
    m_pMemCtlBtnGroup->addButton(ui->btnAdd, 0);
    m_pMemCtlBtnGroup->addButton(ui->btnDel, 1);
    m_pMemCtlBtnGroup->addButton(ui->btnSetting, 2);
    m_pMemCtlBtnGroup->addButton(ui->btnSave, 3);
    m_pMemCtlBtnGroup->addButton(ui->btnImport, 4);
    m_pMemCtlBtnGroup->setExclusive(true);
    for(auto btn : m_pMemCtlBtnGroup->buttons())
        btn->setStyleSheet(
            StyleMgr::ParseFile(":/styles/tbview_header_push_button"));

    // init model table
    ui->tbviewMem->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->tbviewMem->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    if(nullptr == m_pMemListModel)
        m_pMemListModel = new QStandardItemModel;
    else
        m_pMemListModel->clear();

    ui->tbviewMem->setModel(m_pMemListModel);
    ui->tbviewMem->setVisible(true);
    ui->tbviewMem->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tbviewMem->setSelectionBehavior(QAbstractItemView::SelectRows);
    _refreshMemTable(true);

    auto confs = Config::instance()->memoryConfigs();
    _addMemorys(confs, "Staged");
    _retranslate();
}

void SettingPageMemory::_retranslate()
{
}

void SettingPageMemory::_initConnections()
{
    connect(ui->editFilter,
            &QLineEdit::textChanged,
            this,
            &SettingPageMemory::_slotEditFilterTextChanged);

    connect(m_pMemCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &SettingPageMemory::_slotMemCtlBtnGroupClicked);

    connect(Config::instance(),
            &Config::signalMemoryConfigUpdate,
            this,
            &SettingPageMemory::_slotMemoryConfigUpdate);

    connect(MemMgr::instance(),
            &MemMgr::signalEmbeddingProgress,
            this,
            &SettingPageMemory::_slotEmbeddingProgress);
}

void SettingPageMemory::_refreshMemTable(bool clearFirst)
{
    if(m_pMemListModel == nullptr)
        return;

    if(clearFirst)
        m_pMemListModel->clear();

    ui->tbviewMem->setModel(m_pMemListModel);
    m_pMemListModel->setColumnCount(6);
    m_pMemListModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    m_pMemListModel->setHeaderData(1, Qt::Horizontal, tr("Index File Path"));
    m_pMemListModel->setHeaderData(2, Qt::Horizontal, tr("Meta File Path"));
    m_pMemListModel->setHeaderData(3, Qt::Horizontal, tr("Origin File Path"));
    m_pMemListModel->setHeaderData(4, Qt::Horizontal, tr("Dimension"));
    m_pMemListModel->setHeaderData(5, Qt::Horizontal, tr("Flag"));
}

void SettingPageMemory::_clearMemorys()
{
    if(m_pMemListModel == nullptr)
        return;

    m_pMemListModel->clear();
}

void SettingPageMemory::_addMemorys(
    const QVector<Config::MemoryConfig> &configs, const QString &tag)
{
    if(m_pMemListModel == nullptr)
        return;

    int n_row = m_pMemListModel->rowCount();
    for(int i = 0; i < configs.size(); i++)
    {
        const auto conf = configs.at(i);
        m_pMemListModel->setItem(n_row, 0, new QStandardItem(conf.id));
        m_pMemListModel->setItem(n_row,
                                 1,
                                 new QStandardItem(conf.indexFilePath));
        m_pMemListModel->setItem(n_row,
                                 2,
                                 new QStandardItem(conf.metaFilePath));
        m_pMemListModel->setItem(n_row,
                                 3,
                                 new QStandardItem(conf.originFilePath));
        m_pMemListModel->setItem(
            n_row,
            4,
            new QStandardItem(QString::number(conf.dimension)));

        m_pMemListModel->setItem(n_row, 5, new QStandardItem(tag));
        n_row++;
    }

    // notify bus
    auto busModelInfos = GetBusMemoryInfos();
    emit BusAdapter::instance() -> signalMemoryInfoUpdateNtf(busModelInfos);
}

void SettingPageMemory::_delMemorys(const QVector<QString> &hashs)
{
    if(m_pMemListModel == nullptr)
        return;

    for(int i = m_pMemListModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pIdItem = m_pMemListModel->item(i, 0);
        if(pIdItem == nullptr)
            continue;

        auto hash = pIdItem->text();
        if(hashs.contains(hash))
            m_pMemListModel->removeRow(i);
    }

    // notify bus
    auto busModelInfos = GetBusMemoryInfos();
    emit BusAdapter::instance() -> signalMemoryInfoUpdateNtf(busModelInfos);
}

void SettingPageMemory::_setMemorys(
    const QVector<Config::MemoryConfig> &configs, const QString &tag)
{
    if(m_pMemListModel == nullptr)
        return;

    QVector<QString> ids;
    for(auto conf : configs)
        ids.append(conf.id);

    for(int i = 0; i < m_pMemListModel->rowCount(); i++)
    {
        // id
        auto pIdItem = m_pMemListModel->item(i, 0);
        if(pIdItem == nullptr || !ids.contains(pIdItem->text()))
            continue;

        Config::MemoryConfig conf;
        conf.id = "";
        for(auto item : configs)
        {
            if(item.id == pIdItem->text())
            {
                conf = item;
                break;
            }
        }
        if(conf.id.isEmpty())
            continue;
        pIdItem->setText(conf.id);

        // index file path
        auto pIndexFilePath = m_pMemListModel->item(i, 1);
        if(pIndexFilePath)
            pIndexFilePath->setText(conf.indexFilePath);

        // meta file path
        auto pMetaFilePath = m_pMemListModel->item(i, 2);
        if(pMetaFilePath)
            pMetaFilePath->setText(conf.metaFilePath);

        // origin file path
        auto pOriginFilePath = m_pMemListModel->item(i, 3);
        if(pOriginFilePath)
            pOriginFilePath->setText(conf.originFilePath);

        // dimension
        auto pDimensionItem = m_pMemListModel->item(i, 4);
        if(pDimensionItem)
            pDimensionItem->setText(QString::number(conf.dimension));

        // flag
        auto pFlagItem = m_pMemListModel->item(i, 5);
        if(pFlagItem)
            pFlagItem->setText(tag);
    }

    // notify bus
    auto busModelInfos = GetBusMemoryInfos();
    emit BusAdapter::instance() -> signalMemoryInfoUpdateNtf(busModelInfos);
}

void SettingPageMemory::_filterMemTable(const QString &filterText)
{
    if(m_pMemListModel == nullptr)
        return;

    for(int i = 0; i < m_pMemListModel->rowCount(); i++)
    {
        QStandardItem *pTitleItem = m_pMemListModel->item(i, 2);
        if(pTitleItem == nullptr)
            continue;

        bool match =
            pTitleItem->text().contains(filterText, Qt::CaseInsensitive);
        ui->tbviewMem->setRowHidden(i, !match);
    }
}

void SettingPageMemory::_slotEditFilterTextChanged(const QString &content)
{
    qDebug() << "Filter text changed:" << content;
    _filteMemTable(content);
}

void SettingPageMemory::_slotMemCtlBtnGroupClicked(int id)
{
    auto       indexs = ui->tbviewMem->selectionModel()->selectedIndexes();
    QList<int> rows;
    foreach(const QModelIndex &index, indexs)
        if(!rows.contains(index.row()))
            rows.append(index.row());

    switch(id)
    {
        case 0: // add
        {
            qDebug() << "Add memory button clicked";
            Config::MemoryConfig conf;
            if(!rows.isEmpty())
            {
                auto infos = _getMemoryInfos(rows);
                if(!infos.isEmpty())
                    conf = infos.first();
            }

            MemoryConfigDialog *dlg    = _createMemoryConfigDialog(conf);
            auto                result = dlg->exec();
            if(result == QDialog::Accepted)
            {
                conf       = dlg->GetConfig();
                auto confs = Config::instance()->memoryConfigs();
                confs.append(conf);
                Config::instance()->setMemoryConfigs(confs);

                _addMemorys({conf}, "Unstaged");
            }
        }
        break;
        case 1: // del
        {
            qDebug() << "Delete memory button clicked";
            std::sort(rows.begin(), rows.end(), std::greater<int>());
            foreach(int row, rows)
                m_pMemListModel->removeRow(row);
        }
        break;
        case 2: // setting
        {
            qDebug() << "Memory setting button clicked";
            if(rows.isEmpty())
            {
                QMessageBox::warning(
                    this,
                    tr("No Memory Selected"),
                    tr("Please select a memory to configure."));
                return;
            }
            auto infos = _getMemoryInfos(rows);
            if(infos.isEmpty())
            {
                QMessageBox::warning(
                    this,
                    tr("No Memory Configed"),
                    tr("Please select a memory to configure."));
                return;
            }

            QVector<QString> ids;
            for(auto info : infos)
                ids.append(info.id);

            auto id   = ids.first();
            auto conf = Config::instance()->getMemoryConfigById(id);
            if(conf.id.isEmpty())
            {
                QMessageBox::warning(
                    this,
                    tr("Memory Config Not Found"),
                    tr("Memory config not found for id: %1").arg(id));
                return;
            }

            MemoryConfigDialog *dlg    = _createMemoryConfigDialog(conf);
            auto                result = dlg->exec();
            if(result == QDialog::Accepted)
            {
                conf       = dlg->GetConfig();
                auto confs = Config::instance()->memoryConfigs();
                for(auto &item : confs)
                {
                    if(!ids.contains(item.id))
                        continue;

                    auto tmpId = item.id;
                    item       = conf;
                    item.id    = tmpId;
                }
                Config::instance()->setMemoryConfigs(confs);
            }
            delete dlg;
        }
        break;
        case 3: // save
        {
            qDebug() << "Save memory button clicked";
            _saveMemoryConfigs();
        }
        break;
        case 4: // import
        {
            qDebug() << "Import memory button clicked";
            _importMemoryConfigs();
        }
        break;
        default:
            break;
    }
}

void SettingPageMemory::_slotGenerateMemory(const Config::MemoryConfig &conf)
{
    QStringList files;
    File::walk(
        conf.originFilePath,
        [&](QFileInfo &info, int depth) -> bool {
            if(!info.isFile())
                return true;

            auto suffix = info.suffix().toLower();
            auto types  = Config::instance()->getSupportedDocTypes();
            if(types.contains(suffix))
            {
                files.append(info.absoluteFilePath());
                qDebug() << "find file:" << info.absoluteFilePath();
            }
            return true;
        },
        true);

    if(files.size() < 1)
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("No chunks were created from the file."));
        return;
    }

    m_currTaskId = MemMgr::instance()->asyncEmbedding(files, conf);
}

void SettingPageMemory::_slotMemoryConfigUpdate()
{
    qDebug() << "Memory config updated, refreshing memory table.";
    auto confs = Config::instance()->memoryConfigs();
    _clearMemorys();
    _addMemorys(confs, "Staged");
}

void SettingPageMemory::_slotEmbeddingProgress(const int64_t  taskId,
                                               const int64_t  chunkId,
                                               const QString &memoryId,
                                               const int      totalChunkNum,
                                               const int      finishedChunkNum,
                                               const QByteArray &vectorIndexs)
{
    qDebug() << "SettingPageMemory::_slotEmbeddingProgress with taskId:"
             << taskId << ", memoryId:" << memoryId
             << ", totalChunkNum:" << totalChunkNum
             << ", finishedChunkNum:" << finishedChunkNum
             << ", vectorIndexs.size():" << vectorIndexs.size()
             << ", m_currTaskId:" << m_currTaskId;
    if(taskId != m_currTaskId)
        return;

    auto conf = Config::instance()->getMemoryConfigById(memoryId);
    // add embedding to storage
    std::vector<uint8_t> embeddings(vectorIndexs.begin(), vectorIndexs.end());
    if(!MemMgr::instance()->add(memoryId.toStdString(),
                                std::move(embeddings),
                                conf.dimension,
                                chunkId))
    {
        qDebug() << "Failed to add embedding index for chunkId " << chunkId;
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("Failed to add embedding index for chunkId %1").arg(chunkId));
        return;
    }

    // check if finished
    if(finishedChunkNum >= totalChunkNum)
    {
        MemMgr::instance()->save(memoryId.toStdString(),
                                 conf.indexFilePath.toStdString(),
                                 conf.metaFilePath.toStdString());
        m_currTaskId = -1;
    }

    emit signalEmbeddingProgressUpdate(conf, finishedChunkNum, totalChunkNum);
}

QVector<Bus::MemoryInfo> SettingPageMemory::GetBusMemoryInfos()
{
    QVector<Bus::MemoryInfo> ret;
    auto                     confs = _getMemoryInfos({});
    for(auto conf : confs)
    {
        Bus::MemoryInfo info;
        info.id = conf.id;
        ret.append(info);
    }
    return ret;
}

void SettingPageMemory::_filteMemTable(const QString &filterText)
{
    if(m_pMemListModel == nullptr)
        return;

    for(int i = 0; i < m_pMemListModel->rowCount(); i++)
    {
        QStandardItem *pIdItem = m_pMemListModel->item(i, 0);
        if(pIdItem == nullptr)
            continue;

        bool match = pIdItem->text().contains(filterText, Qt::CaseInsensitive);
        ui->tbviewMem->setRowHidden(i, !match);
    }
}

QVector<Config::MemoryConfig>
SettingPageMemory::_getMemoryInfos(const QVector<int> &rows)
{
    QVector<Config::MemoryConfig> ret;
    if(m_pMemListModel == nullptr)
        return ret;

    int n_row = m_pMemListModel->rowCount();
    for(int i = 0; i < n_row; i++)
    {
        if(!rows.isEmpty() && !rows.contains(i))
            continue;

        auto                 id   = m_pMemListModel->item(i, 0)->text();
        Config::MemoryConfig conf = Config::instance()->getMemoryConfigById(id);

        conf.indexFilePath  = m_pMemListModel->item(i, 1)->text();
        conf.metaFilePath   = m_pMemListModel->item(i, 2)->text();
        conf.originFilePath = m_pMemListModel->item(i, 3)->text();
        conf.dimension      = m_pMemListModel->item(i, 4)->text().toInt();

        ret.append(conf);
    }
    return ret;
}

void SettingPageMemory::_saveMemoryConfigs()
{
    auto infos = _getMemoryInfos({});
    auto confs = Config::instance()->memoryConfigs();
    for(int i = 0; i < confs.size(); ++i)
    {
        for(auto info : infos)
        {
            if(confs[i].id != info.id)
                continue;

            confs[i].indexFilePath  = info.indexFilePath;
            confs[i].metaFilePath   = info.metaFilePath;
            confs[i].originFilePath = info.originFilePath;
            confs[i].dimension      = info.dimension;
        }
    }
    Config::instance()->setMemoryConfigs(confs);
    Config::instance()->saveMemory(
        Config::instance()->getMemoryConfigFilePath());
    QMessageBox::information(
        this,
        tr("Save Successful"),
        tr("Memory config exported to file: %1")
            .arg(Config::instance()->getMemoryConfigFilePath()));
}

void SettingPageMemory::_importMemoryConfigs()
{
    // choose memory file path
    QString filePath =
        QFileDialog::getOpenFileName(this,
                                     tr("Select Memory Config File"),
                                     "",
                                     tr("Memory Config Files (*.json)"));

    Config::instance()->loadMemory(filePath);
    auto confs = Config::instance()->memoryConfigs();
    _addMemorys(confs, "Staged");
}

MemoryConfigDialog *
SettingPageMemory::_createMemoryConfigDialog(const Config::MemoryConfig &conf)
{
    auto dlg = new MemoryConfigDialog(conf, this);

    // connect the signalGenerateMemory signal to the _slotGenerateMemory slot
    connect(dlg,
            &MemoryConfigDialog::signalGenerateMemory,
            this,
            &SettingPageMemory::_slotGenerateMemory);

    connect(this,
            &SettingPageMemory::signalEmbeddingProgressUpdate,
            dlg,
            &MemoryConfigDialog::slotEmbeddingProgressUpdate);
    return dlg;
}