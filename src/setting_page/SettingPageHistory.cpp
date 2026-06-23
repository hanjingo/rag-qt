#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QJsonArray>
#include <QJsonObject>

#include "Bus.h"
#include "GrpcClient.h"
#include "Account.h"
#include "Error.h"

#include "SettingPageHistory.h"
#include "ui_SettingPageHistory.h"

SettingPageHistory *SettingPageHistory::m_stSettingPageHistoryInst = nullptr;

SettingPageHistory *SettingPageHistory::GetSettingPageHistoryInst()
{
    if(nullptr == m_stSettingPageHistoryInst)
    {
        m_stSettingPageHistoryInst = new SettingPageHistory();
    }

    return m_stSettingPageHistoryInst;
}

SettingPageHistory::SettingPageHistory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageHistory)
    , m_pHistoryModel(new QStandardItemModel(this))
{
    ui->setupUi(this);

    _initUI();
    _initConnections();
}

SettingPageHistory::~SettingPageHistory()
{
    delete ui;
}

void SettingPageHistory::_initConnections()
{
    connect(GrpcClient::Instance(),
            &GrpcClient::SignalNewSessionResp,
            this,
            &SettingPageHistory::_slotNewSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetSessionResp,
            this,
            &SettingPageHistory::_slotGetSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetMessageInfoResp,
            this,
            &SettingPageHistory::_slotGetMessageInfoResp);

    connect(ui->tbviewCatalog->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            &SettingPageHistory::_slotTbviewCurrentChanged);

    connect(ui->btnDelete,
            &QPushButton::clicked,
            this,
            &SettingPageHistory::_slotBtnDelSessionClicked);

    connect(ui->btnExport,
            &QPushButton::clicked,
            this,
            &SettingPageHistory::_slotBtnExportSessionClicked);

    connect(ui->btnImport,
            &QPushButton::clicked,
            this,
            &SettingPageHistory::_slotBtnImportSessionClicked);
}

void SettingPageHistory::_slotNewSessionResp(const int           errorCode,
                                             const Bus::Session &session)
{
    qDebug()
        << "SettingPageHistory: New session response received, session id: "
        << session.id;

    _addSessions({session});
    _refreshHistoryTable();
}

void SettingPageHistory::_slotGetSessionResp(
    const int errorCode, const QVector<Bus::Session> &sessions)
{
    if(errorCode != ErrorCode::OK)
    {
        qDebug() << "Error in GetSessionResp: " << errorCode;
        return;
    }

    qDebug() << "SettingPageHistory: Get session response received with "
             << sessions.size() << " items.";

    m_pHistoryModel->clear();
    _addSessions(sessions);
    _refreshHistoryTable();
}

void SettingPageHistory::_slotDelSessionResp(const int               errorCode,
                                             const QVector<int64_t> &ids)
{
    if(errorCode != ErrorCode::OK)
    {
        qDebug() << "Error in DelSessionResp: " << errorCode;
        return;
    }

    qDebug() << "SettingPageHistory: Delete session response received with "
             << ids.size() << " items.";
    for(int i = m_pHistoryModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pIdItem = m_pHistoryModel->item(i, 0);
        if(pIdItem == nullptr)
            continue;

        int64_t id = pIdItem->data(Qt::DisplayRole).toLongLong();
        if(ids.contains(id))
        {
            qDebug() << "delete session id = " << id;
            m_pHistoryModel->removeRow(i);
        }
    }
    _refreshHistoryTable();
}

void SettingPageHistory::_slotGetMessageInfoResp(
    const int errorCode, const QVector<Bus::MessageInfo> &messages)
{
    qDebug() << "SettingPageHistory: Get message info response received with "
             << messages.size() << " items.";

    if(errorCode != ErrorCode::OK)
    {
        qDebug() << "Error in GetMessageInfoResp: " << errorCode;
        return;
    }

    // For testing, just display the session content in chat browser; TODO
    // remove it later
    for(int i = 0; i < m_pHistoryModel->rowCount(); i++)
    {
        QStandardItem *pItem = m_pHistoryModel->item(i, 0);
        if(pItem == nullptr)
            continue;

        int64_t        session_id   = pItem->data(Qt::DisplayRole).toLongLong();
        QStandardItem *pContentItem = m_pHistoryModel->item(i, 3);
        if(pContentItem == nullptr)
        {
            QJsonArray content;
            pContentItem = new QStandardItem;
            pContentItem->setData(content, Qt::UserRole);
            m_pHistoryModel->setItem(i, 3, pContentItem);
        }

        for(const auto &msg : messages)
        {
            if(msg.sessionId != session_id)
                continue;

            QJsonObject obj;
            obj["id"]            = QString::number(msg.id);
            obj["sessionId"]     = QString::number(msg.sessionId);
            obj["role"]          = msg.role;
            obj["content"]       = msg.content;
            obj["prevMessageId"] = QString::number(msg.prevMessageId);
            obj["timestamp"]     = msg.timestamp;
            QJsonArray arr = pContentItem->data(Qt::UserRole).toJsonArray();
            arr.append(obj);
            pContentItem->setData(arr, Qt::UserRole);
        }
    }
    _refreshChatBrowser(true);
}

void SettingPageHistory::_slotTbviewCurrentChanged(const QModelIndex &curr,
                                                   const QModelIndex &prev)
{
    Q_UNUSED(prev);

    if(!curr.isValid())
        return;

    int            row     = curr.row();
    QStandardItem *pIdItem = m_pHistoryModel->item(row, 0);
    if(pIdItem == nullptr)
        return;

    int64_t sessionId = pIdItem->data(Qt::DisplayRole).toLongLong();
    qDebug() << "Session selected, id: " << sessionId;
    GrpcClient::Instance()->GetMessageInfo(sessionId,
                                           Account::Instance()->Id(),
                                           Account::Instance()->Auth(),
                                           -1,
                                           100);
}

void SettingPageHistory::_slotBtnDelSessionClicked()
{
    QModelIndexList selectedIndexes =
        ui->tbviewCatalog->selectionModel()->selectedRows();
    if(selectedIndexes.isEmpty())
        return;

    QVector<int64_t> sessionIds;
    for(const auto &index : selectedIndexes)
    {
        int            row     = index.row();
        QStandardItem *pIdItem = m_pHistoryModel->item(row, 0);
        if(pIdItem == nullptr)
            continue;

        int64_t sessionId = pIdItem->data(Qt::DisplayRole).toLongLong();
        sessionIds.append(sessionId);
    }

    if(sessionIds.isEmpty())
        return;

    qDebug() << "Delete sessions with ids: " << sessionIds;
    GrpcClient::Instance()->DelSession(Account::Instance()->Id(),
                                       Account::Instance()->Auth(),
                                       sessionIds);
}

void SettingPageHistory::_slotBtnExportSessionClicked()
{
    qDebug() << "Export session button clicked.";
}

void SettingPageHistory::_slotBtnImportSessionClicked()
{
    qDebug() << "Import session button clicked.";
}

void SettingPageHistory::_initUI()
{
    // init history item table
    ui->tbviewCatalog->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbviewCatalog->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    ui->tbviewCatalog->setModel(m_pHistoryModel);
    ui->tbviewCatalog->setVisible(true);
    ui->tbviewCatalog->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tbviewCatalog->setSelectionBehavior(QAbstractItemView::SelectRows);

    // init chat browser
    ui->txtBrowserChat->setFocusPolicy(Qt::NoFocus);
    ui->txtBrowserChat->setOpenExternalLinks(true);
    ui->txtBrowserChat->setStyleSheet(
        "QTextBrowser {"
        "   background-color: #ffffff;"
        "   border: 1px solid #cccccc;"
        "   font-family: 'Microsoft YaHei', sans-serif;"
        "   font-size: 14px;"
        "}");
}

void SettingPageHistory::_retranslate()
{
}

void SettingPageHistory::_addSessions(const QVector<Bus::Session> &sessions)
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

        // content
        QJsonArray content;
        auto       contentItem = new QStandardItem;
        contentItem->setData(content, Qt::UserRole);
        m_pHistoryModel->setItem(n_row, 3, contentItem);
        n_row++;
    }
}

void SettingPageHistory::_refreshHistoryTable(bool clearFirst)
{
    if(m_pHistoryModel == nullptr)
        return;

    if(clearFirst)
        m_pHistoryModel->clear();

    ui->tbviewCatalog->setModel(m_pHistoryModel);
    m_pHistoryModel->setColumnCount(4);
    m_pHistoryModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    m_pHistoryModel->setHeaderData(1, Qt::Horizontal, tr("Date Time"));
    m_pHistoryModel->setHeaderData(2, Qt::Horizontal, tr("Title"));
    m_pHistoryModel->setHeaderData(3, Qt::Horizontal, tr("Content"));

    ui->tbviewCatalog->hideColumn(0); // hide session id column
    ui->tbviewCatalog->hideColumn(3); // hide session content column
}

void SettingPageHistory::_refreshChatBrowser(bool clearFirst)
{
    QModelIndex curr = ui->tbviewCatalog->currentIndex();
    if(!curr.isValid())
        return;

    int            row          = curr.row();
    QStandardItem *pContentItem = m_pHistoryModel->item(row, 3);
    if(pContentItem == nullptr)
        return;

    if(clearFirst)
        ui->txtBrowserChat->clear();

    QJsonArray arr = pContentItem->data(Qt::UserRole).toJsonArray();
    for(int i = arr.size() - 1; i >= 0; i--)
    {
        QJsonObject obj       = arr.at(i).toObject();
        QString     role      = obj["role"].toString();
        QString     content   = obj["content"].toString();
        QString     timestamp = obj["timestamp"].toString();

        if(role == "user")
            _addQueryRecord(content, timestamp);
        else
            _addAnswerRecord(content, timestamp);
    }
}

void SettingPageHistory::_addQueryRecord(const QString &query,
                                         const QString &timestamp)
{
    qDebug() << "Add Query Record: " << query;
    QString html = QString(R"(
        <table width="100%" border="0" cellspacing="0" cellpadding="0" style="margin-bottom: 10px;">
            <tr>
                <td align="right">
                    <span style="color: #888888; font-size: 11px; margin-right: 5px;">%1</span>
                    <br>
                    <table bgcolor="#d5f5e3" style="border-radius: 10px; margin-top: 3px;" cellpadding="6">
                        <tr>
                            <td style="color: #000000; font-size: 14px; text-align: left;">%2</td>
                        </tr>
                    </table>
                </td>
            </tr>
        </table>
    )")
                       .arg(timestamp, query);

    ui->txtBrowserChat->append(html);

    QTextCursor cursor = ui->txtBrowserChat->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->txtBrowserChat->setTextCursor(cursor);
}

void SettingPageHistory::_addAnswerRecord(const QString &answer,
                                          const QString &timestamp)
{
    qDebug() << "Add answer record: " << answer;
    QString html = QString(R"(
        <table width="100%" border="0" cellspacing="0" cellpadding="0" style="margin-bottom: 10px;">
            <tr>
                <td align="left">
                    <span style="color: #888888; font-size: 11px; margin-left: 5px;">%1</span>
                    <br>
                    <table bgcolor="#f1f1f1" style="border-radius: 10px; margin-top: 3px;" cellpadding="6">
                        <tr>
                            <td style="color: #000000; font-size: 14px; text-align: left;">%2</td>
                        </tr>
                    </table>
                </td>
            </tr>
        </table>
    )")
                       .arg(timestamp, answer);

    ui->txtBrowserChat->append(html);

    QTextCursor cursor = ui->txtBrowserChat->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->txtBrowserChat->setTextCursor(cursor);
}