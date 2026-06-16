#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QTimer>
#include <QThread>
#include <QHeaderView>

#include "HomePageWidget.h"
#include "ui_HomePageWidget.h"

#include "NewSessionDialog.h"
#include "ui_NewSessionDialog.h"

#include "SkillBtn.h"
#include "GrpcClient.h"

HomePageWidget *HomePageWidget::m_stMainHomePageInst = nullptr;

HomePageWidget *HomePageWidget::GetMainHomePageInst()
{
    if(nullptr == m_stMainHomePageInst)
    {
        m_stMainHomePageInst = new HomePageWidget();
    }

    return m_stMainHomePageInst;
}

HomePageWidget::HomePageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePageWidget)
    , m_pSkillsBtnGroup(new QButtonGroup(this))
    , m_pSessionCtlBtnGroup(new QButtonGroup(this))
    , m_pHistoryModel(nullptr)
    , m_colNum(3)
{
    ui->setupUi(this);

#ifdef DEBUG
    m_skillsInfo = {"Skill1",
                    "Skill2",
                    "Skill3",
                    "Skill4",
                    "Skill5",
                    "Skill6",
                    "Skill7",
                    "Skill8",
                    "Skill9",
                    "Skill10",
                    "Skill11",
                    "Skill12",
                    "Skill13",
                    "Skill14",
                    "Skill15",
                    "Skill16",
                    "Skill17"};
#endif

    _initSkillsArea();
    _initHistoryArea();
    _retranslate();
    _initConnections();
}

HomePageWidget::~HomePageWidget()
{
    delete m_pSkillsBtnGroup;
    m_pSkillsBtnGroup = nullptr;

    delete m_pSessionCtlBtnGroup;
    m_pSessionCtlBtnGroup = nullptr;

    delete ui;
}

void HomePageWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if(event->type() == QEvent::LanguageChange)
    {
        qDebug() << "HomePageWidget language change event received.";
        ui->retranslateUi(this);
        _retranslate();
    }
}

void HomePageWidget::_initSkillsArea()
{
    m_pSkillsBtnGroup->setExclusive(false);
    int idx = 0;
    foreach(QString skill, m_skillsInfo)
    {
        SkillBtn *pBtn = new SkillBtn(ui->scrollAreaSkills);
        pBtn->setText(skill);
        m_pSkillsBtnGroup->addButton(pBtn);
        idx++;
    }

    _drawSkillsArea();
}

void HomePageWidget::_initHistoryArea()
{
    // init filter edit
    ui->editFilter->setText(tr("Filter"));

    // init session control buttons
    ui->btnAdd->setIcon(QIcon(":/icons/add"));
    ui->btnAdd->setVisible(true);
    ui->btnDel->setIcon(QIcon(":/icons/del"));
    ui->btnDel->setVisible(true);
    ui->btnSetting->setIcon(QIcon(":/icons/settings"));
    ui->btnSetting->setVisible(true);
    m_pSessionCtlBtnGroup->addButton(ui->btnAdd, 0);
    m_pSessionCtlBtnGroup->addButton(ui->btnDel, 1);
    m_pSessionCtlBtnGroup->addButton(ui->btnSetting, 2);
    m_pSessionCtlBtnGroup->setExclusive(true);

    // init history table
    ui->tbviewHistory->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tbviewHistory->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    if(nullptr == m_pHistoryModel)
    {
        m_pHistoryModel = new QStandardItemModel;
    } else
    {
        m_pHistoryModel->clear();
    }

    ui->tbviewHistory->setModel(m_pHistoryModel);
    ui->tbviewHistory->setVisible(true);
    ui->tbviewHistory->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tbviewHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
    _refreshSessionTable(true);
}

void HomePageWidget::_drawSkillsArea()
{
    int width  = 100;
    int height = width; // make skill button square
    for(int i = 0; i < m_pSkillsBtnGroup->buttons().count(); i++)
    {
        QAbstractButton *pBtn = m_pSkillsBtnGroup->buttons().at(i);
        if(!pBtn)
            continue;

        pBtn->setFixedSize(width, height);
        ui->grid_ProScroll->addWidget(pBtn, i / m_colNum, i % m_colNum);
    }
}

void HomePageWidget::_retranslate()
{
    ui->lblSkillsTitle->setText(tr("Skills"));
    ui->lblHistoryTitle->setText(tr("History"));

    ui->editFilter->setPlaceholderText(tr("Filter"));
    _refreshSessionTable();
}

void HomePageWidget::_initConnections()
{
    connect(m_pSkillsBtnGroup,
            &QButtonGroup::buttonClicked,
            this,
            &HomePageWidget::_slotSkillBtnClicked);

    connect(m_pSessionCtlBtnGroup,
            &QButtonGroup::idClicked,
            this,
            &HomePageWidget::_slotSessionCtlBtnGroupClicked);

    connect(ui->editFilter,
            &QLineEdit::textChanged,
            this,
            &HomePageWidget::_slotEditFilterTextChanged);

    connect(GrpcClient::GetGrpcClientInst(),
            &GrpcClient::SignalGrpcConnected,
            this,
            &HomePageWidget::_slotGrpcConnected);

    connect(GrpcClient::GetGrpcClientInst(),
            &GrpcClient::SignalGetSessionResp,
            this,
            &HomePageWidget::_slotGetSessionResp);

    connect(GrpcClient::GetGrpcClientInst(),
            &GrpcClient::SignalNewSessionResp,
            this,
            &HomePageWidget::_slotNewSessionResp);

    connect(GrpcClient::GetGrpcClientInst(),
            &GrpcClient::SignalModifySessionTitleResp,
            this,
            &HomePageWidget::_slotModifySessionTitleResp);
}

void HomePageWidget::_slotSkillBtnClicked(QAbstractButton *pBtn)
{
    qDebug() << "Skill button clicked:" << pBtn;
    if(!pBtn)
        return;

    SkillBtn *pSkillBtn = qobject_cast<SkillBtn *>(pBtn);

#ifdef DEBUG
    emit pSkillBtn->SignalUpdateProgress(50);
#endif
}

void HomePageWidget::_slotSessionCtlBtnGroupClicked(int id)
{
    qDebug() << "Session control button clicked, id: " << id;
    switch(id)
    {
        case 0: {
            qDebug() << "Add session button clicked.";
            NewSessionDialog(this).exec();
        }
        break;
        case 1: {
            qDebug() << "Delete session button clicked.";
            auto rows = ui->tbviewHistory->selectionModel()->selectedRows();
            QVector<int64_t> row_ids;
            for(auto row : rows)
                row_ids.append(row.siblingAtColumn(0).data().toLongLong());
            _delSessions(row_ids);
        }
        break;
        case 2: {
            qDebug() << "Session settings button clicked.";
        }
        break;
        default: {
            qDebug() << "Unknown session control button clicked.";
        }
        break;
    }
}

void HomePageWidget::_slotEditFilterTextChanged(const QString &content)
{
    qDebug() << "Filter text changed:" << content;
    _filterSessionTable(content);
}

void HomePageWidget::_slotGrpcConnected(const QString &address)
{
    qDebug() << "HomePageWidget connected to gRPC server at " << address;

#ifdef DEBUG
    // For testing, query history after connected
    GrpcClient::GetGrpcClientInst()->GetSession(-1, 1, "", 10);
#endif
}

void HomePageWidget::_slotGetSessionResp(
    const int errorCode, const QVector<::GrpcLibrary::Session> &sessions)
{
    qDebug() << "Get session response received with " << sessions.size()
             << " items.";

    m_pHistoryModel->clear();
    _addSessions(sessions);
    _refreshSessionTable();
}

void HomePageWidget::_slotNewSessionResp(const int errorCode,
                                         const ::GrpcLibrary::Session &session)
{
    qDebug() << "New session response received, session id: " << session.id();
    // For testing, just append the new session to history
    _addSessions({session});
    _refreshSessionTable();
}

void HomePageWidget::_slotModifySessionTitleResp(const int      errorCode,
                                                 const int64_t  id,
                                                 const QString &title)
{
    qDebug() << "Modify session title response received, session id: " << id
             << ", new title: " << title;
    // For testing, just update the title in history if session id matches
    for(int i = 0; i < m_pHistoryModel->rowCount(); i++)
    {
        QStandardItem *pIdItem = m_pHistoryModel->item(i, 0);
        if(pIdItem == nullptr || pIdItem->text().toLongLong() != id)
            continue;

        QStandardItem *pTitleItem = m_pHistoryModel->item(i, 2);
        if(pTitleItem != nullptr)
        {
            pTitleItem->setText(title);
            break;
        }
    }

    _refreshSessionTable();
}

void HomePageWidget::_addSessions(
    const QVector<::GrpcLibrary::Session> &sessions)
{
    if(m_pHistoryModel == nullptr)
        return;

    int n_row = m_pHistoryModel->rowCount();
    for(int i = 0; i < sessions.size(); i++)
    {
        const auto &item = sessions.at(i);
        m_pHistoryModel->setItem(n_row, 0, new QStandardItem(item.id()));
        m_pHistoryModel->setItem(
            n_row,
            1,
            new QStandardItem(QString::fromStdString(item.timestamp())));
        m_pHistoryModel->setItem(
            n_row,
            2,
            new QStandardItem(QString::fromStdString(item.title())));
        n_row++;
    }
}

void HomePageWidget::_delSessions(const QVector<int64_t> &sessionIds)
{
    if(m_pHistoryModel == nullptr)
        return;

    for(int i = m_pHistoryModel->rowCount() - 1; i >= 0; i--)
    {
        QStandardItem *pIdItem = m_pHistoryModel->item(i, 0);
        if(pIdItem == nullptr)
            continue;

        int64_t id = pIdItem->text().toLongLong();
        if(sessionIds.contains(id))
            m_pHistoryModel->removeRow(i);
    }
}

void HomePageWidget::_refreshSessionTable(bool clearFirst)
{
    if(m_pHistoryModel == nullptr)
        return;

    if(clearFirst)
        m_pHistoryModel->clear();

    ui->tbviewHistory->setModel(m_pHistoryModel);
    m_pHistoryModel->setColumnCount(3);
    m_pHistoryModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    m_pHistoryModel->setHeaderData(1, Qt::Horizontal, tr("Date Time"));
    m_pHistoryModel->setHeaderData(2, Qt::Horizontal, tr("Title"));

    ui->tbviewHistory->hideColumn(0); // hide session id column
}

void HomePageWidget::_filterSessionTable(const QString &filterText)
{
    if(m_pHistoryModel == nullptr)
        return;

    for(int i = 0; i < m_pHistoryModel->rowCount(); i++)
    {
        QStandardItem *pTitleItem = m_pHistoryModel->item(i, 2);
        if(pTitleItem == nullptr)
            continue;

        bool match =
            pTitleItem->text().contains(filterText, Qt::CaseInsensitive);
        ui->tbviewHistory->setRowHidden(i, !match);
    }
}