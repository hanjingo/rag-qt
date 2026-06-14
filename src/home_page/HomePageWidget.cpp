#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QTimer>
#include <QThread>
#include <QHeaderView>

#include "HomePageWidget.h"
#include "ui_HomePageWidget.h"

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
    _retranslateTexts();
    _initConnections();
}

HomePageWidget::~HomePageWidget()
{
    delete m_pSkillsBtnGroup;
    m_pSkillsBtnGroup = nullptr;

    delete ui;
}

void HomePageWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if(event->type() == QEvent::LanguageChange)
    {
        qDebug() << "HomePageWidget language change event received.";
        ui->retranslateUi(this);
        _retranslateTexts();
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
    m_pHistoryModel->setColumnCount(2);
    m_pHistoryModel->setHeaderData(0, Qt::Horizontal, tr("Date Time"));
    m_pHistoryModel->setHeaderData(1, Qt::Horizontal, tr("Content"));
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

void HomePageWidget::_retranslateTexts()
{
    ui->lblSkillsTitle->setText(tr("Skills"));
    ui->lblHistoryTitle->setText(tr("History"));
    if(m_pHistoryModel)
    {
        m_pHistoryModel->setHeaderData(0, Qt::Horizontal, tr("Date Time"));
        m_pHistoryModel->setHeaderData(1, Qt::Horizontal, tr("Content"));
    }
}

void HomePageWidget::_initConnections()
{
    connect(m_pSkillsBtnGroup,
            SIGNAL(buttonClicked(QAbstractButton *)),
            this,
            SLOT(_slotSkillBtnClicked(QAbstractButton *)));

    connect(GrpcClient::GetGrpcClientInst(),
            &GrpcClient::SignalGrpcConnected,
            this,
            &HomePageWidget::_slotGrpcConnected);

    connect(GrpcClient::GetGrpcClientInst(),
            &GrpcClient::SignalGetHistoryResp,
            this,
            &HomePageWidget::_slotGetHistoryResp);
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

void HomePageWidget::_slotGrpcConnected(const QString &address)
{
    qDebug() << "HomePageWidget connected to gRPC server at " << address;

    GrpcClient::GetGrpcClientInst()->get_history("1");
}

void HomePageWidget::_slotGetHistoryResp(
    const QVector<GrpcClient::History> &resp)
{
    qDebug() << "Get history response received with " << resp.size()
             << " items.";
    m_pHistoryModel->clear();
    m_pHistoryModel->setColumnCount(2);
    m_pHistoryModel->setHeaderData(0, Qt::Horizontal, tr("Date Time"));
    m_pHistoryModel->setHeaderData(1, Qt::Horizontal, tr("Content"));
    for(int i = 0; i < resp.size(); i++)
    {
        const auto &item = resp.at(i);
        m_pHistoryModel->setItem(i, 0, new QStandardItem(item.datetime));
        m_pHistoryModel->setItem(i, 1, new QStandardItem(item.content));
    }
}