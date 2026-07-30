#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QMessageBox>

#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>

#include "StyleMgr.h"
#include "SettingPageModel.h"
#include "ui_SettingPageModel.h"
#include "ModelConfigDialog.h"

#include "Global.h"
#include "GrpcClient.h"
#include "Error.h"
#include "Account.h"
#include "BusAdapter.h"
#include "Config.h"

SettingPageModel *SettingPageModel::m_stSettingPageModelInst = nullptr;
SettingPageModel *SettingPageModel::Instance()
{
    if(nullptr == m_stSettingPageModelInst)
    {
        m_stSettingPageModelInst = new SettingPageModel();
    }

    return m_stSettingPageModelInst;
}

SettingPageModel::SettingPageModel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageModel)
    , m_pModelCtlBtnGroup(new QButtonGroup(this))
    , m_pLLMListModel(nullptr)
{
    ui->setupUi(this);

    _initUI();
    _initConnections();
}

SettingPageModel::~SettingPageModel()
{
    delete m_pLLMListModel;
    m_pLLMListModel = nullptr;

    delete ui;
}

QVector<Bus::ModelInfo> SettingPageModel::GetBusModelInfos()
{
    QVector<Bus::ModelInfo> infos;
    if(m_pLLMListModel == nullptr)
        return infos;

    for(int row = 0; row < m_pLLMListModel->rowCount(); ++row)
    {
        Bus::ModelInfo conf;
        conf.id        = m_pLLMListModel->item(row, 0)->text();
        conf.name      = m_pLLMListModel->item(row, 1)->text();
        conf.publisher = m_pLLMListModel->item(row, 2)->text();
        conf.timestamp = m_pLLMListModel->item(row, 3)->text();
        conf.addr      = m_pLLMListModel->item(row, 4)->text();
        conf.pipeline  = m_pLLMListModel->item(row, 5)->text();
        conf.cost      = m_pLLMListModel->item(row, 7)->text().toInt();
        conf.hash      = m_pLLMListModel->item(row, 8)->text();

        conf.ctxWindowSize = m_pLLMListModel->item(row, 9)->text().toInt();
        conf.stopWords     = m_pLLMListModel->item(row, 10)->text();

        conf.prompt = m_pLLMListModel->item(row, 11)->text();

        infos.append(conf);
    }
    return infos;
}

void SettingPageModel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if(event->type() == QEvent::LanguageChange)
    {
        qDebug() << "SettingPageModel language change event received.";
        ui->retranslateUi(this);
        _retranslate();
    }
}

void SettingPageModel::_initUI()
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
    m_pModelCtlBtnGroup->addButton(ui->btnAdd, 0);
    m_pModelCtlBtnGroup->addButton(ui->btnDel, 1);
    m_pModelCtlBtnGroup->addButton(ui->btnSetting, 2);
    m_pModelCtlBtnGroup->addButton(ui->btnSave, 3);
    m_pModelCtlBtnGroup->addButton(ui->btnImport, 4);
    m_pModelCtlBtnGroup->setExclusive(true);
    for(auto btn : m_pModelCtlBtnGroup->buttons())
        btn->setStyleSheet(
            StyleMgr::ParseFile(":/styles/tbview_header_push_button"));

    // init model table
    ui->tbviewModel->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->tbviewModel->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    if(nullptr == m_pLLMListModel)
        m_pLLMListModel = new QStandardItemModel;
    else
        m_pLLMListModel->clear();

    ui->tbviewModel->setModel(m_pLLMListModel);
    ui->tbviewModel->setVisible(true);
    ui->tbviewModel->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tbviewModel->setSelectionBehavior(QAbstractItemView::SelectRows);
    _refreshModelTable(true);

    auto confs = Config::Instance().modelConfigs();
    _addModels(confs, "Staged");
    _retranslate();
}

void SettingPageModel::_retranslate()
{
    _refreshModelTable();
}

void SettingPageModel::_initConnections()
{
    connect(ui->editFilter,
            &QLineEdit::textChanged,
            this,
            &SettingPageModel::_slotEditFilterTextChanged);

    connect(m_pModelCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &SettingPageModel::_slotModelCtlBtnGroupClicked);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalLoginResp,
            this,
            &SettingPageModel::_slotLoginResp);
}

void SettingPageModel::_refreshModelTable(bool clearFirst)
{
    if(m_pLLMListModel == nullptr)
        return;

    if(clearFirst)
        m_pLLMListModel->clear();

    ui->tbviewModel->setModel(m_pLLMListModel);
    m_pLLMListModel->setColumnCount(12);
    m_pLLMListModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    m_pLLMListModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    m_pLLMListModel->setHeaderData(2, Qt::Horizontal, tr("Publisher"));
    m_pLLMListModel->setHeaderData(3, Qt::Horizontal, tr("Time Stamp"));
    m_pLLMListModel->setHeaderData(4, Qt::Horizontal, tr("Addr"));
    m_pLLMListModel->setHeaderData(5, Qt::Horizontal, tr("Pipeline"));
    m_pLLMListModel->setHeaderData(6, Qt::Horizontal, tr("Cost"));
    m_pLLMListModel->setHeaderData(7, Qt::Horizontal, tr("Api KEY"));
    m_pLLMListModel->setHeaderData(8, Qt::Horizontal, tr("Hash"));

    m_pLLMListModel->setHeaderData(9, Qt::Horizontal, tr("Window Size"));
    m_pLLMListModel->setHeaderData(10, Qt::Horizontal, tr("Stop Words"));

    m_pLLMListModel->setHeaderData(11, Qt::Horizontal, tr("Prompt"));

    m_pLLMListModel->setHeaderData(12, Qt::Horizontal, tr("Tag"));

    ui->tbviewModel->hideColumn(6);  // hide cost column
    ui->tbviewModel->hideColumn(8);  // hide hash column
    ui->tbviewModel->hideColumn(10); // hide stop words column
    ui->tbviewModel->hideColumn(11); // hide prompt column
}

void SettingPageModel::_addModels(const QVector<Config::ModelConfig> &configs,
                                  const QString                      &tag)
{
    if(m_pLLMListModel == nullptr)
        return;

    int n_row = m_pLLMListModel->rowCount();
    for(int i = 0; i < configs.size(); i++)
    {
        const auto conf = configs.at(i);
        m_pLLMListModel->setItem(n_row, 0, new QStandardItem(conf.id));
        m_pLLMListModel->setItem(n_row, 1, new QStandardItem(conf.name));
        m_pLLMListModel->setItem(n_row, 2, new QStandardItem(conf.publisher));
        m_pLLMListModel->setItem(n_row, 3, new QStandardItem(conf.timestamp));
        m_pLLMListModel->setItem(n_row, 4, new QStandardItem(conf.addr));
        m_pLLMListModel->setItem(n_row, 5, new QStandardItem(conf.pipeline));
        m_pLLMListModel->setItem(
            n_row,
            6,
            new QStandardItem(QString::number(conf.cost, 'f', 1)));
        m_pLLMListModel->setItem(n_row, 7, new QStandardItem(conf.apiKey));
        m_pLLMListModel->setItem(n_row, 8, new QStandardItem(conf.hash));

        m_pLLMListModel->setItem(
            n_row,
            9,
            new QStandardItem(QString::number(conf.ctxWindowSize)));
        m_pLLMListModel->setItem(n_row, 10, new QStandardItem(conf.stopWords));
        m_pLLMListModel->setItem(n_row, 11, new QStandardItem(conf.prompt));

        m_pLLMListModel->setItem(n_row, 12, new QStandardItem(tag));
        n_row++;
    }

    // notify bus
    auto busModelInfos = _GetBusModelInfos();
    emit BusAdapter::Instance() -> SignalModelInfoUpdateNtf(busModelInfos);
}

void SettingPageModel::_delModels(const QVector<QString> &hashs)
{
    if(m_pLLMListModel == nullptr)
        return;

    for(int i = m_pLLMListModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pIdItem = m_pLLMListModel->item(i, 0);
        if(pIdItem == nullptr)
            continue;

        auto hash = pIdItem->text();
        if(hashs.contains(hash))
            m_pLLMListModel->removeRow(i);
    }

    // save to config file
    auto confs = Config::Instance().modelConfigs();
    for(auto itr = confs.begin(); itr != confs.end(); ++itr)
    {
        if(hashs.contains(itr->hash))
            itr = confs.erase(itr);
    }
    Config::Instance().setModelConfigs(confs);

    // notify bus
    auto busModelInfos = _GetBusModelInfos();
    emit BusAdapter::Instance() -> SignalModelInfoUpdateNtf(busModelInfos);
}

void SettingPageModel::_setModels(const QVector<Config::ModelConfig> &configs,
                                  const QString                      &tag)
{
    if(m_pLLMListModel == nullptr)
        return;

    QVector<QString> ids;
    for(auto conf : configs)
        ids.append(conf.id);

    for(int i = 0; i < m_pLLMListModel->rowCount(); i++)
    {
        // id
        auto pIdItem = m_pLLMListModel->item(i, 0);
        if(pIdItem == nullptr || !ids.contains(pIdItem->text()))
            continue;
        Config::ModelConfig conf;
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

        // name
        auto pNameItem = m_pLLMListModel->item(i, 1);
        if(pNameItem)
            pNameItem->setText(conf.name);

        // publisher
        auto pPublisherItem = m_pLLMListModel->item(i, 2);
        if(pPublisherItem)
            pPublisherItem->setText(conf.publisher);

        // timestamp
        auto pTimestampItem = m_pLLMListModel->item(i, 3);
        if(pTimestampItem)
            pTimestampItem->setText(conf.timestamp);

        // addr
        auto pAddrItem = m_pLLMListModel->item(i, 4);
        if(pAddrItem)
            pAddrItem->setText(conf.addr);

        // pipeline
        auto pPipelineItem = m_pLLMListModel->item(i, 5);
        if(pPipelineItem)
            pPipelineItem->setText(conf.pipeline);

        // cost
        auto pCostItem = m_pLLMListModel->item(i, 6);
        if(pCostItem)
            pCostItem->setText(QString::number(conf.cost));

        // apiKey
        auto pApiKeyItem = m_pLLMListModel->item(i, 7);
        if(pApiKeyItem)
            pApiKeyItem->setText(conf.apiKey);

        // hash
        auto pHashItem = m_pLLMListModel->item(i, 8);
        if(pHashItem)
            pHashItem->setText(conf.hash);

        // ctxWindowSize
        auto pCtxWindowSizeItem = m_pLLMListModel->item(i, 9);
        if(pCtxWindowSizeItem)
            pCtxWindowSizeItem->setText(QString::number(conf.ctxWindowSize));

        // stopWords
        auto pStopWordsItem = m_pLLMListModel->item(i, 10);
        if(pStopWordsItem)
            pStopWordsItem->setText(conf.stopWords);

        // prompt
        auto pPromptItem = m_pLLMListModel->item(i, 11);
        if(pPromptItem)
            pPromptItem->setText(conf.prompt);

        // tag
        auto pTagItem = m_pLLMListModel->item(i, 12);
        if(pTagItem)
            pTagItem->setText(tag);
    }

    // save to config file
    auto confs = Config::Instance().modelConfigs();
    for(int i = 0; i < confs.size(); i++)
    {
        for(auto item : configs)
        {
            if(confs[i].id != item.id)
                continue;

            confs[i] = item;
        }
    }
    Config::Instance().setModelConfigs(confs);

    auto infos = _GetBusModelInfos();
    emit BusAdapter::Instance() -> SignalModelInfoUpdateNtf(infos);
}

QVector<Bus::ModelInfo>
SettingPageModel::_GetBusModelInfos(const QVector<int> &rows)
{
    QVector<Bus::ModelInfo> ret;
    if(m_pLLMListModel == nullptr)
        return ret;

    int n_row = m_pLLMListModel->rowCount();
    for(int i = 0; i < n_row; i++)
    {
        if(!rows.isEmpty() && !rows.contains(i))
            continue;

        Bus::ModelInfo info;
        info.id        = m_pLLMListModel->item(i, 0)->text();
        info.name      = m_pLLMListModel->item(i, 1)->text();
        info.publisher = m_pLLMListModel->item(i, 2)->text();
        info.timestamp = m_pLLMListModel->item(i, 3)->text();
        info.addr      = m_pLLMListModel->item(i, 4)->text();
        info.pipeline  = m_pLLMListModel->item(i, 5)->text();
        info.cost      = m_pLLMListModel->item(i, 6)->text().toFloat();
        info.hash      = m_pLLMListModel->item(i, 8)->text();

        info.ctxWindowSize = m_pLLMListModel->item(i, 9)->text().toInt();
        info.stopWords     = m_pLLMListModel->item(i, 10)->text();

        info.prompt = m_pLLMListModel->item(i, 11)->text();
        if(info.id.isEmpty())
            continue;

        ret.append(info);
    }
    return ret;
}

void SettingPageModel::_filterModelTable(const QString &filterText)
{
    if(m_pLLMListModel == nullptr)
        return;

    for(int i = 0; i < m_pLLMListModel->rowCount(); i++)
    {
        QStandardItem *pTitleItem = m_pLLMListModel->item(i, 2);
        if(pTitleItem == nullptr)
            continue;

        bool match =
            pTitleItem->text().contains(filterText, Qt::CaseInsensitive);
        ui->tbviewModel->setRowHidden(i, !match);
    }
}

void SettingPageModel::_saveModelConfigs()
{
    auto infos = _GetBusModelInfos();
    auto confs = Config::Instance().modelConfigs();
    for(int i = 0; i < confs.size(); ++i)
    {
        for(auto info : infos)
        {
            if(confs[i].id != info.id)
                continue;

            confs[i].name      = info.name;
            confs[i].publisher = info.publisher;
            confs[i].timestamp = info.timestamp;
            confs[i].addr      = info.addr;
            confs[i].pipeline  = info.pipeline;
            confs[i].cost      = info.cost;
            confs[i].hash      = info.hash;

            confs[i].ctxWindowSize = info.ctxWindowSize;
            confs[i].stopWords     = info.stopWords;

            confs[i].prompt = info.prompt;
        }
    }
    Config::Instance().setModelConfigs(confs);
    Config::Instance().saveModel(Config::getModelConfigFilePath());
    QMessageBox::information(this,
                             tr("Save Successful"),
                             tr("Model config exported to file: %1")
                                 .arg(Config::getModelConfigFilePath()));
}

void SettingPageModel::_importModelConfigs()
{
    // choose model file path
    QString filePath =
        QFileDialog::getOpenFileName(this,
                                     tr("Select Model Config File"),
                                     "",
                                     tr("Model Config Files (*.json)"));

    Config::Instance().loadModel(filePath);
    auto confs = Config::Instance().modelConfigs();
    _addModels(confs, "Staged");
}

void SettingPageModel::_slotEditFilterTextChanged(const QString &content)
{
    qDebug() << "Filter text changed:" << content;
    _filterModelTable(content);
}

void SettingPageModel::_slotModelCtlBtnGroupClicked(int id)
{
    auto       indexs = ui->tbviewModel->selectionModel()->selectedIndexes();
    QList<int> rows;
    foreach(const QModelIndex &index, indexs)
        if(!rows.contains(index.row()))
            rows.append(index.row());

    switch(id)
    {
        case 0: // add
        {
            qDebug() << "Add model button clicked";
            Config::ModelConfig conf;
            if(!rows.empty())
            {
                auto infos = _GetBusModelInfos(rows);
                if(!infos.empty())
                {
                    auto info      = infos.at(0);
                    conf.id        = info.id;
                    conf.name      = info.name;
                    conf.publisher = info.publisher;
                    conf.timestamp = info.timestamp;
                    conf.addr      = info.addr;
                    conf.pipeline  = info.pipeline;
                    conf.cost      = info.cost;
                    conf.hash      = info.hash;

                    conf.ctxWindowSize = info.ctxWindowSize;
                    conf.stopWords     = info.stopWords;

                    conf.prompt = info.prompt;
                }
            }

            ModelConfigDialog dlg(conf, this);
            auto              result = dlg.exec();
            conf                     = dlg.GetConfig();
            if(result == QDialog::Accepted)
            {
                _addModels({conf}, "Unstaged");

                // save to config file
                auto confs = Config::Instance().modelConfigs();
                confs.append(conf);
                Config::Instance().setModelConfigs(confs);
            }
        }
        break;
        case 1: // del
        {
            qDebug() << "Delete model button clicked";
            std::sort(rows.begin(), rows.end(), std::greater<int>());
            foreach(int row, rows)
                m_pLLMListModel->removeRow(row);
        }
        break;
        case 2: // setting
        {
            qDebug() << "Model setting button clicked";
            if(rows.isEmpty())
            {
                QMessageBox::warning(this,
                                     tr("No Model Selected"),
                                     tr("Please select a model to configure."));
                return;
            }
            auto infos = _GetBusModelInfos(rows);
            if(infos.isEmpty())
            {
                QMessageBox::warning(this,
                                     tr("No Model Configed"),
                                     tr("Please select a model to configure."));
                return;
            }
            auto id = infos.at(0).id;

            // get model conf info
            auto                confs = Config::Instance().modelConfigs();
            Config::ModelConfig conf;
            for(auto item : confs)
            {
                if(item.id != id)
                    continue;

                conf = item;
            }
            ModelConfigDialog dlg(conf, this);
            auto              result = dlg.exec();
            if(result != QDialog::Accepted)
                return;

            conf = dlg.GetConfig();
            QVector<Config::ModelConfig> newConfs;
            for(auto item : infos)
            {
                Config::ModelConfig newConf = conf;
                newConf.id                  = item.id;
                newConfs.append(newConf);
            }
            _setModels(newConfs, "Staged");
        }
        break;
        case 3: // save
        {
            qDebug() << "Save model button clicked";
            _saveModelConfigs();
        }
        break;
        case 4: // import
        {
            qDebug() << "Import model button clicked";
            _importModelConfigs();
        }
        break;
        default:
            break;
    }
}

void SettingPageModel::_slotLoginResp(const int      errorCode,
                                      const int64_t  userId,
                                      const QString &auth,
                                      const QString &account,
                                      const QString &lastLoginTime,
                                      const bool     isForceUpdate)
{
}