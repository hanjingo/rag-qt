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

QVector<Bus::Model> SettingPageModel::GetModelInfos()
{
    QVector<Bus::Model> modelInfos;
    for(int row = 0; row < m_pLLMListModel->rowCount(); ++row)
    {
        Bus::Model modelInfo;
        modelInfo.name         = m_pLLMListModel->item(row, 0)->text();
        modelInfo.contextSize  = m_pLLMListModel->item(row, 1)->text().toInt();
        modelInfo.capabilities = m_pLLMListModel->item(row, 2)->text();
        modelInfo.cost      = m_pLLMListModel->item(row, 3)->text().toDouble();
        modelInfo.timestamp = m_pLLMListModel->item(row, 4)->text();
        modelInfo.addr      = m_pLLMListModel->item(row, 5)->text();
        modelInfo.hash      = m_pLLMListModel->item(row, 6)->text();
        modelInfo.publisher = m_pLLMListModel->item(row, 7)->text();

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
            &SettingPageModel::_slotTbviewSelectionChanged);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalLoginResp,
            this,
            &SettingPageModel::_slotLoginResp);

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
    m_pLLMListModel->setColumnCount(9);
    m_pLLMListModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
    m_pLLMListModel->setHeaderData(1, Qt::Horizontal, tr("Context Size"));
    m_pLLMListModel->setHeaderData(2, Qt::Horizontal, tr("Capabilities"));
    m_pLLMListModel->setHeaderData(3, Qt::Horizontal, tr("Cost"));
    m_pLLMListModel->setHeaderData(4, Qt::Horizontal, tr("Timestamp"));
    m_pLLMListModel->setHeaderData(5, Qt::Horizontal, tr("Address"));
    m_pLLMListModel->setHeaderData(6, Qt::Horizontal, tr("Hash"));
    m_pLLMListModel->setHeaderData(7, Qt::Horizontal, tr("Publisher"));
    m_pLLMListModel->setHeaderData(8, Qt::Horizontal, tr("Tag"));
}

void SettingPageModel::_addModels(const QVector<Bus::Model> &models,
                                  const QString             &tag)
{
    if(m_pLLMListModel == nullptr)
        return;

    int n_row = m_pLLMListModel->rowCount();
    for(int i = 0; i < models.size(); i++)
    {
        const auto item = models.at(i);
        m_pLLMListModel->setItem(n_row, 0, new QStandardItem(item.name));
        m_pLLMListModel->setItem(n_row, 1, new QStandardItem(item.contextSize));
        m_pLLMListModel->setItem(n_row,
                                 2,
                                 new QStandardItem(item.capabilities));
        m_pLLMListModel->setItem(n_row,
                                 3,
                                 new QStandardItem(QString::number(item.cost)));
        m_pLLMListModel->setItem(n_row, 4, new QStandardItem(item.timestamp));
        m_pLLMListModel->setItem(n_row, 5, new QStandardItem(item.addr));
        m_pLLMListModel->setItem(n_row, 6, new QStandardItem(item.hash));
        m_pLLMListModel->setItem(n_row, 7, new QStandardItem(item.publisher));
        m_pLLMListModel->setItem(n_row, 8, new QStandardItem(tag));
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
            _addModels({Bus::Model()}, "Unstaged");
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

void SettingPageModel::_slotTbviewSelectionChanged(
    const QItemSelection &selected, const QItemSelection &deselected)
{
    if(deselected.isEmpty())
        return;

    if(Account::Instance()->Privilege() < Account::PrivilegeType::Admin)
        return;

    QModelIndex prev = deselected.indexes().first();
    if(!prev.isValid())
        return;

    int oldRow = prev.row();
    if(oldRow >= m_pLLMListModel->rowCount())
        return;

    auto pTagItem = m_pLLMListModel->item(oldRow, 8);
    if(!pTagItem)
        return;

    QString tag = pTagItem->text();
    if(tag == "Unstaged")
    {
        qDebug() << "Model info unstaged, need to create. Row: " << oldRow;
        Bus::Model model;
        model.name = m_pLLMListModel->item(oldRow, 0)
                         ? m_pLLMListModel->item(oldRow, 0)->text()
                         : "";
        model.contextSize =
            m_pLLMListModel->item(oldRow, 1)
                ? m_pLLMListModel->item(oldRow, 1)->text().toLongLong()
                : 0;
        model.capabilities = m_pLLMListModel->item(oldRow, 2)
                                 ? m_pLLMListModel->item(oldRow, 2)->text()
                                 : "";
        model.cost      = m_pLLMListModel->item(oldRow, 3)
                              ? m_pLLMListModel->item(oldRow, 3)->text().toInt()
                              : 0;
        model.timestamp = m_pLLMListModel->item(oldRow, 4)
                              ? m_pLLMListModel->item(oldRow, 4)->text()
                              : "";
        model.addr      = m_pLLMListModel->item(oldRow, 5)
                              ? m_pLLMListModel->item(oldRow, 5)->text()
                              : "";
        model.hash      = m_pLLMListModel->item(oldRow, 6)
                              ? m_pLLMListModel->item(oldRow, 6)->text()
                              : "";
        model.publisher = m_pLLMListModel->item(oldRow, 7)
                              ? m_pLLMListModel->item(oldRow, 7)->text()
                              : "";

        if(model.hash.isEmpty() || model.addr.isEmpty() || model.name.isEmpty())
        {
            qDebug() << "Selected model hash or addr is empty, ignore.";
            QMessageBox::warning(
                nullptr,
                tr("Invalid Model Info"),
                tr("Model hash, addr or name is empty, please check."));
            return;
        }

        GrpcClient::Instance()->NewModelInfo(Account::Instance()->Id(),
                                             Account::Instance()->Auth(),
                                             {model});
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

void SettingPageModel::_slotGetModelInfoResp(
    const int errorCode, const QVector<Bus::Model> &modelInfos)
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