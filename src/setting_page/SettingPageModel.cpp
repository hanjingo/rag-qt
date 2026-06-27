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

QVector<Bus::ModelConfig> SettingPageModel::GetModelConfigs()
{
    QVector<Bus::ModelConfig> modelConfigs;
    if(m_pLLMListModel == nullptr)
        return modelConfigs;

    for(int row = 0; row < m_pLLMListModel->rowCount(); ++row)
    {
        Bus::ModelConfig conf;
        conf.id            = m_pLLMListModel->item(row, 0)->text();
        conf.name          = m_pLLMListModel->item(row, 1)->text();
        conf.publisher     = m_pLLMListModel->item(row, 2)->text();
        conf.timestamp     = m_pLLMListModel->item(row, 3)->text();
        conf.addr          = m_pLLMListModel->item(row, 4)->text();
        conf.pipeline      = m_pLLMListModel->item(row, 5)->text();
        conf.ctxWindowSize = m_pLLMListModel->item(row, 6)->text().toInt();
        conf.cost          = m_pLLMListModel->item(row, 7)->text().toInt();

        conf.apiKey      = m_pLLMListModel->item(row, 8)->text();
        conf.temperature = m_pLLMListModel->item(row, 9)->text().toFloat();
        conf.topP        = m_pLLMListModel->item(row, 10)->text().toFloat();
        conf.topK        = m_pLLMListModel->item(row, 11)->text().toFloat();
        conf.reputationPenalty =
            m_pLLMListModel->item(row, 12)->text().toFloat();
        conf.minP      = m_pLLMListModel->item(row, 13)->text().toFloat();
        conf.stopWords = m_pLLMListModel->item(row, 14)->text();

        conf.prompt = m_pLLMListModel->item(row, 15)->text();

        modelConfigs.append(conf);
    }
    return modelConfigs;
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
    m_pModelCtlBtnGroup->addButton(ui->btnAdd, 0);
    m_pModelCtlBtnGroup->addButton(ui->btnDel, 1);
    m_pModelCtlBtnGroup->addButton(ui->btnSetting, 2);
    m_pModelCtlBtnGroup->addButton(ui->btnSave, 3);
    m_pModelCtlBtnGroup->addButton(ui->btnImport, 4);
    m_pModelCtlBtnGroup->setExclusive(true);

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

    _retranslate();
    _importModelConfigs();
}

void SettingPageModel::_retranslate()
{
    _refreshModelTable();
}

void SettingPageModel::_initConnections()
{
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
    m_pLLMListModel->setColumnCount(17);
    m_pLLMListModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    m_pLLMListModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    m_pLLMListModel->setHeaderData(2, Qt::Horizontal, tr("Publisher"));
    m_pLLMListModel->setHeaderData(3, Qt::Horizontal, tr("Time Stamp"));
    m_pLLMListModel->setHeaderData(4, Qt::Horizontal, tr("Addr"));
    m_pLLMListModel->setHeaderData(5, Qt::Horizontal, tr("Pipeline"));
    m_pLLMListModel->setHeaderData(6, Qt::Horizontal, tr("Cost"));
    m_pLLMListModel->setHeaderData(7, Qt::Horizontal, tr("Api KEY"));

    m_pLLMListModel->setHeaderData(8, Qt::Horizontal, tr("Temperature"));
    m_pLLMListModel->setHeaderData(9, Qt::Horizontal, tr("Top-P"));
    m_pLLMListModel->setHeaderData(10, Qt::Horizontal, tr("Top-K"));
    m_pLLMListModel->setHeaderData(11,
                                   Qt::Horizontal,
                                   tr("Reputation Penalty"));
    m_pLLMListModel->setHeaderData(12, Qt::Horizontal, tr("Min-P"));

    m_pLLMListModel->setHeaderData(13, Qt::Horizontal, tr("Window Size"));
    m_pLLMListModel->setHeaderData(14, Qt::Horizontal, tr("Stop Words"));

    m_pLLMListModel->setHeaderData(15, Qt::Horizontal, tr("Prompt"));

    m_pLLMListModel->setHeaderData(16, Qt::Horizontal, tr("Tag"));

    ui->tbviewModel->hideColumn(1); // hide name column
    ui->tbviewModel->hideColumn(2); // hide publisher column
    ui->tbviewModel->hideColumn(3); // hide timestamp column
    // ui->tbviewModel->hideColumn(4);  // hide addr column
    // ui->tbviewModel->hideColumn(7);  // hide API KEY column
    ui->tbviewModel->hideColumn(8);  // hide temperature column
    ui->tbviewModel->hideColumn(9);  // hide top-p column
    ui->tbviewModel->hideColumn(10); // hide top-k column
    // ui->tbviewModel->hideColumn(11); // hide reputation penalty column
    ui->tbviewModel->hideColumn(12); // hide min-p column
    ui->tbviewModel->hideColumn(14); // hide stop words column
    ui->tbviewModel->hideColumn(15); // hide prompt column
}

void SettingPageModel::_addModels(const QVector<Bus::ModelConfig> &configs,
                                  const QString                   &tag)
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
        m_pLLMListModel->setItem(n_row,
                                 6,
                                 new QStandardItem(QString::number(conf.cost)));
        m_pLLMListModel->setItem(n_row, 7, new QStandardItem(conf.apiKey));

        m_pLLMListModel->setItem(
            n_row,
            8,
            new QStandardItem(QString::number(conf.temperature)));
        m_pLLMListModel->setItem(n_row,
                                 9,
                                 new QStandardItem(QString::number(conf.topP)));
        m_pLLMListModel->setItem(n_row,
                                 10,
                                 new QStandardItem(QString::number(conf.topK)));
        m_pLLMListModel->setItem(
            n_row,
            11,
            new QStandardItem(QString::number(conf.reputationPenalty)));
        m_pLLMListModel->setItem(n_row,
                                 12,
                                 new QStandardItem(QString::number(conf.minP)));

        m_pLLMListModel->setItem(
            n_row,
            13,
            new QStandardItem(QString::number(conf.ctxWindowSize)));
        m_pLLMListModel->setItem(n_row, 14, new QStandardItem(conf.stopWords));

        m_pLLMListModel->setItem(n_row, 15, new QStandardItem(conf.prompt));

        m_pLLMListModel->setItem(n_row, 16, new QStandardItem(tag));
        n_row++;
    }

    auto busModelInfos = _getModelConfigs();
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

    auto busModelInfos = _getModelConfigs();
    emit BusAdapter::Instance() -> SignalModelInfoUpdateNtf(busModelInfos);
}

void SettingPageModel::_setModels(const QVector<Bus::ModelConfig> &configs,
                                  const QString                   &tag)
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
        Bus::ModelConfig conf;
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

        // temperature
        auto pTemperatureItem = m_pLLMListModel->item(i, 8);
        if(pTemperatureItem)
            pTemperatureItem->setText(QString::number(conf.temperature));

        // topP
        auto pTopPItem = m_pLLMListModel->item(i, 9);
        if(pTopPItem)
            pTopPItem->setText(QString::number(conf.topP));

        // topK
        auto pTopKItem = m_pLLMListModel->item(i, 10);
        if(pTopKItem)
            pTopKItem->setText(QString::number(conf.topK));

        // reputationPenalty
        auto pReputationPenaltyItem = m_pLLMListModel->item(i, 11);
        if(pReputationPenaltyItem)
            pReputationPenaltyItem->setText(
                QString::number(conf.reputationPenalty));

        // minP
        auto pMinPItem = m_pLLMListModel->item(i, 12);
        if(pMinPItem)
            pMinPItem->setText(QString::number(conf.minP));

        // ctxWindowSize
        auto pCtxWindowSizeItem = m_pLLMListModel->item(i, 13);
        if(pCtxWindowSizeItem)
            pCtxWindowSizeItem->setText(QString::number(conf.ctxWindowSize));

        // stopWords
        auto pStopWordsItem = m_pLLMListModel->item(i, 14);
        if(pStopWordsItem)
            pStopWordsItem->setText(conf.stopWords);

        // prompt
        auto pPromptItem = m_pLLMListModel->item(i, 15);
        if(pPromptItem)
            pPromptItem->setText(conf.prompt);

        // tag
        auto pTagItem = m_pLLMListModel->item(i, 16);
        if(pTagItem)
            pTagItem->setText(tag);
    }

    auto busModelInfos = _getModelConfigs();
    emit BusAdapter::Instance() -> SignalModelInfoUpdateNtf(busModelInfos);
}

QVector<Bus::ModelConfig>
SettingPageModel::_getModelConfigs(const QVector<int> &rows)
{
    QVector<Bus::ModelConfig> ret;
    if(m_pLLMListModel == nullptr)
        return ret;

    int n_row = m_pLLMListModel->rowCount();
    for(int i = 0; i < n_row; i++)
    {
        if(!rows.isEmpty() && !rows.contains(i))
            continue;

        Bus::ModelConfig cfg;
        cfg.id        = m_pLLMListModel->item(i, 0)->text();
        cfg.name      = m_pLLMListModel->item(i, 1)->text();
        cfg.publisher = m_pLLMListModel->item(i, 2)->text();
        cfg.timestamp = m_pLLMListModel->item(i, 3)->text();
        cfg.addr      = m_pLLMListModel->item(i, 4)->text();
        cfg.pipeline  = m_pLLMListModel->item(i, 5)->text();
        cfg.cost      = m_pLLMListModel->item(i, 6)->text().toFloat();
        cfg.apiKey    = m_pLLMListModel->item(i, 7)->text();

        cfg.temperature       = m_pLLMListModel->item(i, 8)->text().toFloat();
        cfg.topP              = m_pLLMListModel->item(i, 9)->text().toFloat();
        cfg.topK              = m_pLLMListModel->item(i, 10)->text().toFloat();
        cfg.reputationPenalty = m_pLLMListModel->item(i, 11)->text().toFloat();
        cfg.minP              = m_pLLMListModel->item(i, 12)->text().toFloat();

        cfg.ctxWindowSize = m_pLLMListModel->item(i, 13)->text().toInt();
        cfg.stopWords     = m_pLLMListModel->item(i, 14)->text();

        cfg.prompt = m_pLLMListModel->item(i, 15)->text();

        if(cfg.id.isEmpty())
            continue;

        ret.append(cfg);
    }
    return ret;
}

void SettingPageModel::_convert(QJsonArray                      &jsonArrConfigs,
                                const QVector<Bus::ModelConfig> &configs)
{
    for(auto conf : configs)
    {
        QJsonObject obj;
        obj["id"]        = conf.id;
        obj["name"]      = conf.name;
        obj["publisher"] = conf.publisher;
        obj["timestamp"] = conf.timestamp;
        obj["addr"]      = conf.addr;
        obj["pipeline"]  = conf.pipeline;
        obj["cost"]      = conf.cost;
        obj["api_key"]   = conf.apiKey;

        obj["temperature"] = QString::number(conf.temperature, 'f', 1);
        obj["top_p"]       = QString::number(conf.topP, 'f', 1);
        obj["top_k"]       = QString::number(conf.topK, 'f', 1);
        obj["reputation_penalty"] =
            QString::number(conf.reputationPenalty, 'f', 1);
        obj["min_p"] = QString::number(conf.minP, 'f', 1);

        obj["ctx_window_size"] = conf.ctxWindowSize;
        obj["stop_words"]      = conf.stopWords;

        obj["prompt"] = conf.prompt;

        jsonArrConfigs.append(obj);
    }
}

void SettingPageModel::_convert(QVector<Bus::ModelConfig> &configs,
                                const QJsonArray          &jsonArrConfigs)
{
    for(auto obj : jsonArrConfigs)
    {
        if(!obj.isObject())
            continue;

        QJsonObject      jsonObj = obj.toObject();
        Bus::ModelConfig conf;
        conf.id        = jsonObj["id"].toString();
        conf.name      = jsonObj["name"].toString();
        conf.publisher = jsonObj["publisher"].toString();
        conf.timestamp = jsonObj["timestamp"].toString();
        conf.addr      = jsonObj["addr"].toString();
        conf.pipeline  = jsonObj["pipeline"].toString();
        conf.cost      = jsonObj["cost"].toInt();
        conf.apiKey    = jsonObj["api_key"].toString();

        conf.temperature       = jsonObj["temperature"].toDouble();
        conf.topP              = jsonObj["top_p"].toDouble();
        conf.topK              = jsonObj["top_k"].toDouble();
        conf.reputationPenalty = jsonObj["reputation_penalty"].toDouble();
        conf.minP              = jsonObj["min_p"].toDouble();

        conf.ctxWindowSize = jsonObj["ctx_window_size"].toInt();
        conf.stopWords     = jsonObj["stop_words"].toString();

        conf.prompt = jsonObj["prompt"].toString();

        configs.append(conf);
    }
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
    QFile file(MODEL_CONFIG_FILE);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing: " << file.errorString();
        QMessageBox::warning(
            this,
            tr("Save Failed"),
            tr("Failed to open file for writing: %1").arg(file.errorString()));
        return;
    }

    QJsonArray confArr;
    auto       confs = _getModelConfigs({});
    _convert(confArr, confs);

    QJsonDocument doc(confArr);
    QTextStream   out(&file);
    out << doc.toJson(QJsonDocument::Indented);
    file.close();
    QMessageBox::information(
        this,
        tr("Save Successful"),
        tr("Model config exported to file: %1").arg(MODEL_CONFIG_FILE));
}

void SettingPageModel::_importModelConfigs()
{
    QFile file(MODEL_CONFIG_FILE);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for reading: " << file.errorString();
        QMessageBox::warning(
            this,
            tr("Import Failed"),
            tr("Failed to open file for reading: %1").arg(file.errorString()));
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument   doc = QJsonDocument::fromJson(data, &parseError);
    if(parseError.error != QJsonParseError::NoError)
    {
        qDebug() << "Failed to parse JSON: " << parseError.errorString();
        QMessageBox::warning(
            this,
            tr("Import Failed"),
            tr("Failed to parse JSON: %1").arg(parseError.errorString()));
        return;
    }

    if(!doc.isArray())
    {
        qDebug() << "Invalid JSON format: root is not an array";
        QMessageBox::warning(this,
                             tr("Import Failed"),
                             tr("Invalid JSON format: root is not an array"));
        return;
    }

    QJsonArray arr     = doc.array();
    auto       configs = QVector<Bus::ModelConfig>();
    _convert(configs, arr);
    _addModels(configs, "Imported");
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
            Bus::ModelConfig conf;
            if(!rows.empty())
            {
                auto confs = _getModelConfigs(rows);
                if(!confs.empty())
                    conf = confs.at(0);
            }

            ModelConfigDialog dlg(conf, this);
            auto              result = dlg.exec();
            if(result == QDialog::Accepted)
                _addModels({dlg.GetConfig()}, "Unstaged");
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
            auto confs = _getModelConfigs(rows);
            if(confs.isEmpty())
            {
                QMessageBox::warning(this,
                                     tr("No Model Configed"),
                                     tr("Please select a model to configure."));
                return;
            }

            Bus::ModelConfig conf;
            conf = confs.at(0);
            ModelConfigDialog dlg(conf, this);
            auto              result = dlg.exec();
            if(result != QDialog::Accepted)
                return;

            conf = dlg.GetConfig();
            QVector<Bus::ModelConfig> newConfs;
            for(auto item : confs)
            {
                item.id            = conf.id;
                item.name          = conf.name;
                item.publisher     = conf.publisher;
                item.timestamp     = conf.timestamp;
                item.addr          = conf.addr;
                item.pipeline      = conf.pipeline;
                item.ctxWindowSize = conf.ctxWindowSize;
                item.cost          = conf.cost;
                item.apiKey        = conf.apiKey;

                item.temperature       = conf.temperature;
                item.topP              = conf.topP;
                item.topK              = conf.topK;
                item.reputationPenalty = conf.reputationPenalty;
                item.minP              = conf.minP;
                item.stopWords         = conf.stopWords;

                item.prompt = conf.prompt;
                newConfs.append(item);
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
                                      const int32_t  privilege,
                                      const QString &account,
                                      const QString &lastLoginTime)
{
    // hide add/delete/config button if no privilege
    if(errorCode != ErrorCode::OK
       || static_cast<Account::PrivilegeType>(privilege)
              < Account::PrivilegeType::Admin)
    {
        ui->btnAdd->setEnabled(false);
        ui->btnDel->setEnabled(false);
        ui->btnSetting->setEnabled(false);
    } else
    {
        ui->btnAdd->setEnabled(true);
        ui->btnDel->setEnabled(true);
        ui->btnSetting->setEnabled(true);
    }
}