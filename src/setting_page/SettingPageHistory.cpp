#include <QtMath>
#include <QPainter>
#include <QAction>

#include "Bus.h"
#include "GrpcClient.h"

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

void SettingPageHistory::_slotGetSessionResp(
    const int errorCode, const QVector<Bus::Session> &sessions)
{
    qDebug() << "SettingPageHistory: Get session response received with "
             << sessions.size() << " items.";

    m_pHistoryModel->clear();
    _addSessions(sessions);
    _refreshHistoryTable();
}

void SettingPageHistory::_slotTbviewCurrentChanged(const QModelIndex &curr,
                                                   const QModelIndex &prev)
{
    Q_UNUSED(prev);

    if(!curr.isValid())
        return;

    ui->txtBrowserChat->clear();

    int            row     = curr.row();
    QStandardItem *pIdItem = m_pHistoryModel->item(row, 0);
    if(pIdItem == nullptr)
        return;

    int64_t sessionId = pIdItem->data(Qt::DisplayRole).toLongLong();
    qDebug() << "Session selected, id: " << sessionId;

    // For testing, just display the session content in chat browser; TODO
    // remove it later
    for(int i = 0; i < m_pHistoryModel->rowCount(); i++)
    {
        QStandardItem *pItem = m_pHistoryModel->item(i, 0);
        if(pItem == nullptr)
            continue;

        int64_t id = pItem->data(Qt::DisplayRole).toLongLong();
        if(id != sessionId)
            continue;

        QString         sessionContent = m_pHistoryModel->item(i, 3)->text();
        QJsonParseError parseError;
        QJsonDocument   doc =
            QJsonDocument::fromJson(sessionContent.toUtf8(), &parseError);
        if(parseError.error != QJsonParseError::NoError || !doc.isArray())
        {
            qDebug() << "Failed to parse session content as JSON. content: "
                     << sessionContent
                     << ", error: " << parseError.errorString();
            return;
        }
        QJsonArray arr = doc.array();
        for(const auto &item : arr)
        {
            if(!item.isObject())
                continue;

            QJsonObject obj        = item.toObject();
            QString     content    = obj["content"].toString();
            QString     tm         = obj["timestamp"].toString();
            bool        isQuestion = obj["is_question"].toBool();
            if(isQuestion)
                _addQueryRecord(content, tm);
            else
                _addAnswerRecord(content, tm);
        }
    }
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

    // for testing, just refresh the history table;
    // TODO remove it later
    QVector<Bus::Session> sessions;
    int                   incSec = 0;
    for(int i = 0; i < 100; i++)
    {
        Bus::Session sess{};
        sess.id        = i + 1;
        sess.userId    = 1;
        sess.title     = QString("Session %1").arg(i + 1);
        sess.timestamp = QDateTime::currentDateTime().addSecs(-i * 60).toString(
            "yyyy-MM-dd hh:mm:ss");

        QJsonArray content;
        for(int j = 0; j < 10; j++)
        {
            QJsonObject obj;
            obj["id"]        = QString::number(sess.id);
            obj["userId"]    = QString::number(sess.userId);
            obj["content"]   = (j % 2 == 0) ? QString("question %1").arg(j + 1)
                                            : QString("answer %1").arg(j + 1);
            obj["timestamp"] = QDateTime::currentDateTime()
                                   .addSecs(-i * 60 - incSec)
                                   .toString("hh:mm:ss");
            obj["is_question"] = (j % 2 == 0);
            content.append(obj);
            incSec += 1;
        }
        sess.content = QJsonDocument(content).toJson(QJsonDocument::Compact);
        sessions.append(sess);
    }
    _slotGetSessionResp(0, sessions);
}

void SettingPageHistory::_initConnections()
{
    // Connect signals and slots here if needed
    // connect(GrpcClient::Instance(),
    //         &GrpcClient::SignalGetSessionResp,
    //         this,
    //         &SettingPageHistory::_slotGetSessionResp);

    connect(ui->tbviewCatalog->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            &SettingPageHistory::_slotTbviewCurrentChanged);
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
        const auto &item   = sessions.at(i);
        auto       *idItem = new QStandardItem;
        idItem->setData(QVariant::fromValue<qlonglong>(item.id),
                        Qt::DisplayRole);
        m_pHistoryModel->setItem(n_row, 0, idItem);
        m_pHistoryModel->setItem(n_row, 1, new QStandardItem(item.timestamp));
        m_pHistoryModel->setItem(n_row, 2, new QStandardItem(item.title));
        m_pHistoryModel->setItem(n_row, 3, new QStandardItem(item.content));
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