#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QButtonGroup>
#include <QMessageBox>
#include <QFileDialog>

#include <hj/ai/vector_index.hpp>
#include <hj/algo/uuid.hpp>

#include "StyleMgr.h"
#include "SettingPageMemory.h"
#include "ui_SettingPageMemory.h"

#include "Global.h"
#include "GrpcClient.h"
#include "Error.h"
#include "Account.h"
#include "BusAdapter.h"
#include "Config.h"
#include "FileChunker.h"
#include "File.h"

SettingPageMemory::SettingPageMemory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageMemory)
    , m_pMemCtlBtnGroup(new QButtonGroup(this))
    , m_pMemListModel(nullptr)
    , m_mapTaskChunkIds()
{
    ui->setupUi(this);

    _initUI();
    _initConnections();
}

SettingPageMemory::~SettingPageMemory()
{
    delete m_pMemListModel;
    m_pMemListModel = nullptr;

    delete ui;
}

void SettingPageMemory::_initUI()
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

    auto confs = Config::Instance().memoryConfigs();
    _addMemorys(confs, "Staged");
    _retranslate();
}

void SettingPageMemory::_retranslate()
{
}

void SettingPageMemory::_initConnections()
{
    connect(m_pMemCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &SettingPageMemory::_slotMemCtlBtnGroupClicked);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalEmbeddingResp,
            this,
            &SettingPageMemory::_slotEmbeddingResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalStopEmbeddingResp,
            this,
            &SettingPageMemory::_slotEmbeddingStopResp);
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
    emit BusAdapter::Instance() -> SignalMemoryInfoUpdateNtf(busModelInfos);
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

    // save to config file
    auto confs = Config::Instance().memoryConfigs();
    for(auto itr = confs.begin(); itr != confs.end(); ++itr)
    {
        if(hashs.contains(itr->indexFilePath))
            itr = confs.erase(itr);
    }
    Config::Instance().setMemoryConfigs(confs);

    // notify bus
    auto busModelInfos = GetBusMemoryInfos();
    emit BusAdapter::Instance() -> SignalMemoryInfoUpdateNtf(busModelInfos);
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

    // save to config file
    auto confs = Config::Instance().memoryConfigs();
    for(int i = 0; i < confs.size(); i++)
    {
        for(auto item : configs)
        {
            if(confs[i].id != item.id)
                continue;

            confs[i] = item;
        }
    }
    Config::Instance().setMemoryConfigs(confs);

    // notify bus
    auto busModelInfos = GetBusMemoryInfos();
    emit BusAdapter::Instance() -> SignalMemoryInfoUpdateNtf(busModelInfos);
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
                auto confs = Config::Instance().memoryConfigs();
                confs.append(conf);
                Config::Instance().setMemoryConfigs(confs);

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
            auto id   = infos.first().id;
            auto conf = Config::Instance().getMemoryConfigById(id);
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
                conf = dlg->GetConfig();
                QVector<Config::MemoryConfig> newConfs;
                for(auto item : infos)
                {
                    Config::MemoryConfig newConf = conf;
                    newConf.id                   = item.id;
                    newConfs.append(newConf);
                }
                _setMemorys(newConfs, "Staged");
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
    int64_t taskId      = std::abs(static_cast<int64_t>(hj::uuid::gen_u64()));
    auto    totalLength = File::fileSize(conf.originFilePath);

    FileChunker chunker;
    auto        chunkCount = chunker.chunkFile(
        conf.originFilePath,
        conf.chunkSize,
        [this, taskId, conf, totalLength](FileChunker::Chunk &chunk) -> qint64 {
            if(chunk.Id == 0 && chunk.startPos == 0)
            {
                chunk.Id = static_cast<int64_t>(hj::uuid::gen_u64());
                _setChunkProcessState(taskId, -1, false);
                _setTaskConfig(taskId, conf);
                // send chunk
                GrpcClient::Instance()->Embedding(taskId,
                                                  Account::Instance()->Id(),
                                                  Account::Instance()->Auth(),
                                                  chunk.Id,
                                                  chunk.data,
                                                  chunk.startPos,
                                                  chunk.startPos + chunk.offset,
                                                  conf);
                return true;
            }

            // calculate next chunk start position and offset
            qint64 startPos = chunk.startPos + chunk.offset - conf.overlap;
            if(startPos <= chunk.startPos)
                startPos = chunk.startPos + chunk.offset; // ensure progress

            qint64 endPos = qMin(startPos + chunk.offset, totalLength);
            if(conf.respectParagraphs)
            {
                qint64 newEndPos =
                    startPos
                    + File::findParagraphBoundary(QString::fromUtf8(chunk.data),
                                                  0,
                                                  chunk.data.size());
                if(newEndPos < endPos && newEndPos > startPos)
                    endPos = newEndPos;
            }

            if(conf.respectSentences)
            {
                qint64 newEndPos =
                    startPos
                    + File::findSentenceBoundary(QString::fromUtf8(chunk.data),
                                                 0,
                                                 chunk.data.size());
                if(newEndPos < endPos && newEndPos > startPos)
                    endPos = newEndPos;
            }

            chunk.startPos = startPos;
            chunk.offset   = endPos - startPos;
            chunk.data     = chunk.data.mid(0, chunk.offset);
            chunk.Id       = static_cast<int64_t>(hj::uuid::gen_u64());
            qDebug() << "Generated chunk: index=" << chunk.Id
                     << ", startPos=" << chunk.startPos
                     << ", offset=" << chunk.offset
                     << ", dataSize=" << chunk.data.size();

            if(chunk.data.isEmpty() || chunk.offset <= 0)
            {
                qDebug() << "Chunk data is empty or offset is non-positive, "
                            "skipping.";
                return false;
            }

            _setChunkProcessState(taskId, chunk.Id, false);
            // send chunk
            GrpcClient::Instance()->Embedding(taskId,
                                              Account::Instance()->Id(),
                                              Account::Instance()->Auth(),
                                              chunk.Id,
                                              chunk.data,
                                              chunk.startPos,
                                              chunk.startPos + chunk.offset);

            // build meta file
            QString    metaFilePath = conf.metaFilePath;
            QJsonArray metaArray;
            if(!File::isFileExist(metaFilePath))
                File::writeJsonFile(metaFilePath, metaArray);

            File::readJsonFile(metaFilePath, metaArray);
            QJsonObject obj;
            obj["chunk_id"]       = QString::number(chunk.Id);
            obj["content"]        = QString::fromUtf8(chunk.data);
            obj["start_pos"]      = chunk.startPos;
            obj["end_pos"]        = chunk.startPos + chunk.offset;
            obj["chunk_size"]     = chunk.offset;
            obj["file_path_name"] = chunk.filePathName;
            obj["timestamp"] =
                QDateTime::currentDateTime().toString(Qt::ISODate);
            metaArray.append(obj);
            File::writeJsonFile(metaFilePath, metaArray);

            return true;
        });

    qDebug() << "Generated " << chunkCount << " chunks from origin file.";
    if(chunkCount < 1)
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("No chunks were created from the file."));
        return;
    }
}

void SettingPageMemory::_slotEmbeddingResp(const int         errorCode,
                                           const int64_t     taskId,
                                           const int64_t     chunkId,
                                           const QByteArray &vectorIndexs)
{
    if(errorCode != 0)
    {
        qDebug() << "Embedding response error, code: " << errorCode;
        QMessageBox::warning(
            this,
            tr("Embedding Error"),
            tr("Failed to generate embeddings. Error code: %1").arg(errorCode));
        return;
    }

    if(!vectorIndexs.isEmpty())
    {
        // qDebug() << "Embedding response received, taskId: " << taskId
        //          << ", chunkId: " << chunkId
        //          << ", vectorIndexs: " << vectorIndexs;
        // create tmp dir
        QDir tmpDir(Config::Instance().getDefaultIndexPath() + "/"
                    + QString::number(taskId));
        if(!tmpDir.exists() || !tmpDir.isReadable())
        {
            if(!tmpDir.mkpath("."))
            {
                qDebug() << "Failed to create tmp directory: "
                         << tmpDir.absolutePath();
                QMessageBox::warning(
                    this,
                    tr("Directory Error"),
                    tr("Failed to create temporary directory for embeddings."));
                return;
            }
        }

        // write index to tmp file
        QString tmpFilePath =
            tmpDir.absoluteFilePath(QString("chunk_%1.index").arg(chunkId));
        QFile file(tmpFilePath);
        if(!file.open(QIODevice::WriteOnly))
        {
            qDebug() << "Failed to open file for writing: " << tmpFilePath;
            QMessageBox::warning(
                this,
                tr("File Error"),
                tr("Failed to write embedding index to file."));
            return;
        }
        file.write(vectorIndexs);
        file.close();
        qDebug() << "Embedding index saved to: " << tmpFilePath;
    }

    // combine all chunk index files into one index file if all chunks are processed
    if(_isHadTask(taskId))
    {
        qDebug() << "Received embedding response for taskId: " << taskId
                 << ", chunkId: " << chunkId << ", remove record from map.";
        _setChunkProcessState(taskId, chunkId, true);
        if(_isChunkProcessed(taskId, -1))
        {
            qDebug() << "All chunks for taskId " << taskId
                     << " have been processed. stop it.";
            // combine all chunk index files into one index file
            auto conf = _getTaskConfig(taskId);
            if(!conf.id.isEmpty())
            {
                auto idxFilePath  = conf.indexFilePath.toStdString();
                auto metaFilePath = conf.metaFilePath.toStdString();
                hj::vector_index<hj::vindex_flat_l2_t> index;
                index.build(conf.dimension);

                // combine all chunk index files
                QDir tmpDir(Config::Instance().getDefaultIndexPath() + "/"
                            + QString::number(taskId));
                auto chunkIds = _getChunkIdsForTask(taskId);
                std::sort(chunkIds.begin(), chunkIds.end());
                int total_vectors = 0;
                for(auto chunkId : chunkIds)
                {
                    QString tmpFilePath = tmpDir.absoluteFilePath(
                        QString("chunk_%1.index").arg(chunkId));
                    hj::vector_index<hj::vindex_flat_l2_t> chunkIndex;
                    qDebug() << "Load chunk index file:" << tmpFilePath;
                    if(!File::exists(tmpFilePath.toStdString().c_str())
                       || !chunkIndex.load(tmpFilePath.toStdString().c_str()))
                    {
                        qDebug() << "Failed to load chunk index file: "
                                 << tmpFilePath;
                        continue;
                    }

                    int num_vectors = chunkIndex.total();
                    if(num_vectors <= 0)
                        continue;

                    int                dim = conf.dimension;
                    std::vector<float> vectors(num_vectors * dim);
                    for(int i = 0; i < num_vectors; i++)
                        chunkIndex.reconstruct(i, vectors.data() + i * dim);

                    index.add(num_vectors, vectors.data());
                    total_vectors += num_vectors;
                    qDebug() << "Merged chunk index file:" << tmpFilePath
                             << ", vectors:" << num_vectors
                             << ", total vectors:" << total_vectors;
                }

                if(!index.save(idxFilePath.c_str()))
                {
                    qDebug() << "Failed to save combined index file: "
                             << idxFilePath.c_str();
                    QMessageBox::warning(
                        this,
                        tr("Index Save Error"),
                        tr("Failed to save combined index file: %1")
                            .arg(QString::fromStdString(idxFilePath)));
                } else
                {
                    qDebug()
                        << "Combined index file saved: " << idxFilePath.c_str();
                    // remove tmp dir
                    if(tmpDir.exists() && tmpDir.isReadable())
                    {
                        tmpDir.removeRecursively();
                        qDebug() << "Temporary directory removed: "
                                 << tmpDir.absolutePath();
                    }
                    QMessageBox::information(
                        this,
                        tr("Index Save Successful"),
                        tr("Combined index file saved: %1 \nWrite meta file: "
                           "%2")
                            .arg(QString::fromStdString(idxFilePath))
                            .arg(QString::fromStdString(metaFilePath)));
                }
            }

            // remove task record from map
            _removeTaskRecord(taskId);

            // stop embedding for this task
            GrpcClient::Instance()->EmbeddingStop(taskId,
                                                  Account::Instance()->Id(),
                                                  Account::Instance()->Auth());
        }
    };
}

void SettingPageMemory::_slotEmbeddingStopResp(const int     errorCode,
                                               const int64_t taskId)
{
    if(errorCode != 0)
    {
        qDebug() << "Embedding stop response error, code: " << errorCode;
        QMessageBox::warning(
            this,
            tr("Embedding Stop Error"),
            tr("Failed to stop embedding process. Error code: %1")
                .arg(errorCode));
        return;
    }

    qDebug() << "Embedding stop response success, taskId: " << taskId;
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
        Config::MemoryConfig conf = Config::Instance().getMemoryConfigById(id);

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
    auto confs = Config::Instance().memoryConfigs();
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
    Config::Instance().setMemoryConfigs(confs);
    Config::Instance().saveMemory(MEMORY_CONFIG_FILE);
    QMessageBox::information(
        this,
        tr("Save Successful"),
        tr("Memory config exported to file: %1").arg(MEMORY_CONFIG_FILE));
}

void SettingPageMemory::_importMemoryConfigs()
{
    // choose memory file path
    QString filePath =
        QFileDialog::getOpenFileName(this,
                                     tr("Select Memory Config File"),
                                     "",
                                     tr("Memory Config Files (*.json)"));

    Config::Instance().loadMemory(filePath);
    auto confs = Config::Instance().memoryConfigs();
    _addMemorys(confs, "Staged");
}

bool SettingPageMemory::_setChunkProcessState(const int64_t taskId,
                                              const int64_t chunkId,
                                              const bool    isProcessed)
{
    QMutexLocker locker(&m_mu);
    if(!m_mapTaskChunkIds.contains(taskId))
        m_mapTaskChunkIds[taskId] = QMap<int64_t, bool>();

    if(chunkId == -1)
    {
        // If chunkId is -1, we are setting the overall task state
        // If isProcessed is true, we mark all chunks as processed
        // If isProcessed is false, we mark all chunks as not processed
        for(auto it = m_mapTaskChunkIds[taskId].begin();
            it != m_mapTaskChunkIds[taskId].end();
            ++it)
        {
            it.value() = isProcessed;
        }
    } else
    {
        m_mapTaskChunkIds[taskId][chunkId] = isProcessed;
    }

    // count the number of processed chunks for this task
    if(isProcessed)
    {
        int processedCount = 0;
        for(auto it = m_mapTaskChunkIds[taskId].begin();
            it != m_mapTaskChunkIds[taskId].end();
            ++it)
        {
            if(it.value())
                processedCount++;
        }

        if(m_mapTaskConfigs.contains(taskId))
        {
            auto conf = m_mapTaskConfigs[taskId];
            emit SignalEmbeddingProgressUpdate(
                conf,
                processedCount,
                m_mapTaskChunkIds[taskId].size());
        }
        qDebug() << "TaskId: " << taskId
                 << ", Processed Chunks: " << processedCount
                 << ", Total Chunks: " << m_mapTaskChunkIds[taskId].size();
    }
    return true;
}

bool SettingPageMemory::_isChunkProcessed(const int64_t taskId,
                                          const int64_t chunkId)
{
    QMutexLocker locker(&m_mu);
    if(!m_mapTaskChunkIds.contains(taskId))
        return false;

    if(chunkId == -1)
    {
        for(auto it = m_mapTaskChunkIds[taskId].begin();
            it != m_mapTaskChunkIds[taskId].end();
            ++it)
        {
            if(!it.value())
                return false;
        }
        return true;
    }

    if(!m_mapTaskChunkIds[taskId].contains(chunkId))
        return false;

    return m_mapTaskChunkIds[taskId][chunkId];
}

QVector<int64_t> SettingPageMemory::_getChunkIdsForTask(const int64_t taskId)
{
    QMutexLocker     locker(&m_mu);
    QVector<int64_t> chunkIds;
    if(!m_mapTaskChunkIds.contains(taskId))
        return chunkIds;

    for(auto it = m_mapTaskChunkIds[taskId].begin();
        it != m_mapTaskChunkIds[taskId].end();
        ++it)
    {
        chunkIds.append(it.key());
    }
    return chunkIds;
}

bool SettingPageMemory::_isHadTask(const int64_t taskId)
{
    QMutexLocker locker(&m_mu);
    return m_mapTaskChunkIds.contains(taskId);
}

bool SettingPageMemory::_setTaskConfig(const int64_t               taskId,
                                       const Config::MemoryConfig &conf)
{
    QMutexLocker locker(&m_mu);
    m_mapTaskConfigs[taskId] = conf;
    return true;
}

Config::MemoryConfig SettingPageMemory::_getTaskConfig(const int64_t taskId)
{
    QMutexLocker locker(&m_mu);
    if(m_mapTaskConfigs.contains(taskId))
        return m_mapTaskConfigs[taskId];

    return Config::MemoryConfig();
}

void SettingPageMemory::_removeTaskRecord(const int64_t taskId)
{
    QMutexLocker locker(&m_mu);
    m_mapTaskChunkIds.remove(taskId);
    m_mapTaskConfigs.remove(taskId);
}

MemoryConfigDialog *
SettingPageMemory::_createMemoryConfigDialog(const Config::MemoryConfig &conf)
{
    auto dlg = new MemoryConfigDialog(conf, this);

    // connect the SignalGenerateMemory signal to the _slotGenerateMemory slot
    connect(dlg,
            &MemoryConfigDialog::SignalGenerateMemory,
            this,
            &SettingPageMemory::_slotGenerateMemory);

    connect(this,
            &SettingPageMemory::SignalEmbeddingProgressUpdate,
            dlg,
            &MemoryConfigDialog::SlotEmbeddingProgressUpdate);
    return dlg;
}