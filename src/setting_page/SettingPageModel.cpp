#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QMessageBox>

#include "StyleMgr.h"
#include "SettingPageModel.h"
#include "ui_SettingPageModel.h"

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

QVector<Bus::ModelInfo> SettingPageModel::GetModelInfos()
{
    QVector<Bus::ModelInfo> modelInfos;
    for(int row = 0; row < m_pLLMListModel->rowCount(); ++row)
    {
        Bus::ModelInfo modelInfo;
        modelInfo.name         = m_pLLMListModel->item(row, 0)->text();
        modelInfo.contextSize  = m_pLLMListModel->item(row, 1)->text().toInt();
        modelInfo.capabilities = m_pLLMListModel->item(row, 2)->text();
        modelInfo.cost      = m_pLLMListModel->item(row, 3)->text().toDouble();
        modelInfo.timestamp = m_pLLMListModel->item(row, 4)->text();
        modelInfo.addr      = m_pLLMListModel->item(row, 5)->text();
        modelInfo.hash      = m_pLLMListModel->item(row, 6)->text();

        modelInfos.append(modelInfo);
    }
    return modelInfos;
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

    // hide delete button if no privilege
    if(event->type() == QEvent::Show)
    {
        if(!Account::Instance()->IsValid()
           || Account::Instance()->Privilege() < Account::PrivilegeType::Admin)
        {
            ui->btnDel->setVisible(false);
            ui->btnSetting->setVisible(false);
        }
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
    m_pModelCtlBtnGroup->addButton(ui->btnAdd, 0);
    m_pModelCtlBtnGroup->addButton(ui->btnDel, 1);
    m_pModelCtlBtnGroup->addButton(ui->btnSetting, 2);
    m_pModelCtlBtnGroup->setExclusive(true);

    // init model table
    ui->tbviewModel->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbviewModel->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    if(nullptr == m_pLLMListModel)
    {
        m_pLLMListModel = new QStandardItemModel;
    } else
    {
        m_pLLMListModel->clear();
    }

    ui->tbviewModel->setModel(m_pLLMListModel);
    ui->tbviewModel->setVisible(true);
    ui->tbviewModel->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tbviewModel->setSelectionBehavior(QAbstractItemView::SelectRows);
    _refreshModelTable(true);

    _retranslate();
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

    connect(ui->tbviewModel->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &SettingPageModel::_slotTbviewModelSelectionChanged);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetModelInfoResp,
            this,
            &SettingPageModel::_slotGetModelInfoResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalNewModelInfoResp,
            this,
            &SettingPageModel::_slotNewModelInfoResp);
}

void SettingPageModel::_refreshModelTable(bool clearFirst)
{
    if(m_pLLMListModel == nullptr)
        return;

    if(clearFirst)
        m_pLLMListModel->clear();

    ui->tbviewModel->setModel(m_pLLMListModel);
    m_pLLMListModel->setColumnCount(7);
    m_pLLMListModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
    m_pLLMListModel->setHeaderData(1, Qt::Horizontal, tr("Context Size"));
    m_pLLMListModel->setHeaderData(2, Qt::Horizontal, tr("Capabilities"));
    m_pLLMListModel->setHeaderData(3, Qt::Horizontal, tr("Cost"));
    m_pLLMListModel->setHeaderData(4, Qt::Horizontal, tr("Timestamp"));
    m_pLLMListModel->setHeaderData(5, Qt::Horizontal, tr("Address"));
    m_pLLMListModel->setHeaderData(6, Qt::Horizontal, tr("Hash"));
}

void SettingPageModel::_addModels(const QVector<::GrpcLibrary::Model> &models)
{
    if(m_pLLMListModel == nullptr)
        return;

    int n_row = m_pLLMListModel->rowCount();
    for(int i = 0; i < models.size(); i++)
    {
        const auto item = models.at(i);
        m_pLLMListModel->setItem(
            n_row,
            0,
            new QStandardItem(QString::fromStdString(item.name())));
        m_pLLMListModel->setItem(
            n_row,
            1,
            new QStandardItem(QString::number(item.context_size())));
        m_pLLMListModel->setItem(
            n_row,
            2,
            new QStandardItem(QString::fromStdString(item.capabilities())));
        m_pLLMListModel->setItem(
            n_row,
            3,
            new QStandardItem(QString::number(item.cost())));
        m_pLLMListModel->setItem(
            n_row,
            4,
            new QStandardItem(QString::fromStdString(item.timestamp())));
        m_pLLMListModel->setItem(
            n_row,
            5,
            new QStandardItem(QString::fromStdString(item.addr())));
        m_pLLMListModel->setItem(
            n_row,
            6,
            new QStandardItem(QString::fromStdString(item.hash())));
        n_row++;
    }
}

void SettingPageModel::_delModels(const QVector<int64_t> &modelIds)
{
    if(m_pLLMListModel == nullptr)
        return;

    for(int i = m_pLLMListModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pIdItem = m_pLLMListModel->item(i, 0);
        if(pIdItem == nullptr)
            continue;

        int64_t id = pIdItem->text().toLongLong();
        if(modelIds.contains(id))
            m_pLLMListModel->removeRow(i);
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

void SettingPageModel::_slotModelCtlBtnGroupClicked(int id)
{
    switch(id)
    {
        case 0: // add
        {
            qDebug() << "Add model button clicked";
            _addModels({::GrpcLibrary::Model()});
        }
        break;
        case 1: // del
        {
            qDebug() << "Delete model button clicked";
        }
        break;
        case 2: // setting
        {
            qDebug() << "Model setting button clicked";
        }
        break;
        default:
            break;
    }
}

void SettingPageModel::_slotTbviewModelSelectionChanged(
    const QItemSelection &selected, const QItemSelection &deselected)
{
    if(deselected.indexes().isEmpty()
       || selected.indexes() == deselected.indexes())
        return;

    qDebug() << "Model table selection changed. Selected rows: "
             << selected.indexes().size() / m_pLLMListModel->columnCount()
             << ", Deselected rows: "
             << deselected.indexes().size() / m_pLLMListModel->columnCount();
    auto indexes = selected.indexes();

    ::GrpcLibrary::Model model;
    model.set_hash(
        m_pLLMListModel->item(indexes.at(0).row(), 6)->text().toStdString());
    model.set_name(
        m_pLLMListModel->item(indexes.at(0).row(), 0)->text().toStdString());
    model.set_publisher(
        m_pLLMListModel->item(indexes.at(0).row(), 1)->text().toStdString());
    model.set_timestamp(
        m_pLLMListModel->item(indexes.at(0).row(), 2)->text().toStdString());
    model.set_addr(
        m_pLLMListModel->item(indexes.at(0).row(), 3)->text().toStdString());
    model.set_capabilities(
        m_pLLMListModel->item(indexes.at(0).row(), 4)->text().toStdString());
    model.set_context_size(
        m_pLLMListModel->item(indexes.at(0).row(), 5)->text().toLongLong());
    model.set_cost(
        m_pLLMListModel->item(indexes.at(0).row(), 6)->text().toDouble());

    qDebug() << "Selected model hash: " << model.hash()
             << ", name: " << model.name().c_str()
             << ", publisher: " << model.publisher().c_str()
             << ", timestamp: " << model.timestamp().c_str()
             << ", addr: " << model.addr().c_str()
             << ", capabilities: " << model.capabilities().c_str()
             << ", context size: " << model.context_size()
             << ", cost: " << model.cost();
    if(model.hash().empty() || model.addr().empty())
    {
        qDebug() << "Selected model hash or addr is empty, ignore.";
        QMessageBox::critical(
            this,
            tr("Invalid Model Info"),
            tr("Selected model hash or address is empty, cannot download."));
        m_pLLMListModel->removeRow(indexes.at(0).row());
        return;
    }
    GrpcClient::Instance()->NewModelInfo(Account::Instance()->Id(),
                                         Account::Instance()->Auth(),
                                         {model});
}

void SettingPageModel::_slotGetModelInfoResp(
    const int errorCode, const QVector<::GrpcLibrary::Model> &modelInfos)
{
    if(errorCode != ErrorCode::OK)
    {
        qDebug() << "Failed to get model info, error code: " << errorCode;
        return;
    }

    _addModels(modelInfos);

    // notify to bus
    auto busModelInfos = GetModelInfos();
    emit BusAdapter::Instance() -> SignalModelInfoUpdate(busModelInfos);
}

void SettingPageModel::_slotNewModelInfoResp(const int               errorCode,
                                             const QVector<QString> &hashs)
{
    if(errorCode != ErrorCode::OK)
        qDebug() << "New model added successfully, hash count: "
                 << hashs.size();
    else
        qDebug() << "Failed to add new model, error code: " << errorCode;

    // refresh model table after new model added
    _refreshModelTable(true);
    GrpcClient::Instance()->GetModelInfo(Account::Instance()->Id(),
                                         Account::Instance()->Auth());
    return;
}