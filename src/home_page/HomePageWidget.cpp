#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QTimer>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QHeaderView>
#include <QMessageBox>

#include "HomePageWidget.h"
#include "ui_HomePageWidget.h"

#include "NewSessionDialog.h"
#include "ui_NewSessionDialog.h"

#include "HistorySettingDialog.h"
#include "ui_HistorySettingDialog.h"

#include "SkillBtn.h"
#include "GrpcClient.h"
#include "StyleMgr.h"
#include "Downloader.h"
#include "PluginMgr.h"
#include "Zipper.h"
#include "Error.h"

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
    _clearSkills();
    _drawSkillsArea();
}

void HomePageWidget::_initHistoryArea()
{
    // init filter edit
    ui->editFilter->setStyleSheet(StyleMgr::ParseFile(":/styles/line_edit"));
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

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGrpcConnected,
            this,
            &HomePageWidget::_slotGrpcConnected);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetSessionResp,
            this,
            &HomePageWidget::_slotGetSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalNewSessionResp,
            this,
            &HomePageWidget::_slotNewSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalModifySessionTitleResp,
            this,
            &HomePageWidget::_slotModifySessionTitleResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetSkillInfoResp,
            this,
            &HomePageWidget::_slotGetSkillInfoResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalDownloadResp,
            this,
            &HomePageWidget::_slotDownloadResp);
}

void HomePageWidget::_slotSkillBtnClicked(QAbstractButton *pBtn)
{
    if(!pBtn || !pBtn->isCheckable())
    {
        qDebug() << "Skill button can not be click";
        return;
    }

    qDebug() << "Skill button clicked:" << pBtn;
    SkillBtn *pSkillBtn = qobject_cast<SkillBtn *>(pBtn);
    if(!pSkillBtn)
        return;

    QString hash = pSkillBtn->Hash();
    int     user_id;
    QString auth;

#ifdef DEBUG
    // For testing, use a hardcoded hash value
    user_id = 1;
    auth    = "";
#endif

    GrpcClient::Instance()->Download(hash, user_id, auth);
}

void HomePageWidget::_slotSessionCtlBtnGroupClicked(int id)
{
    qDebug() << "Session control button clicked, id: " << id;
    switch(id)
    {
        case 0: { // add session
            qDebug() << "Add session button clicked.";
            QVector<::GrpcLibrary::Skill> skills;
            for(auto item : m_pSkillsBtnGroup->buttons())
            {
                if(!item)
                    continue;

                SkillBtn *btn = qobject_cast<SkillBtn *>(item);
                if(!btn || !btn->IsDownloaded())
                    continue;

                ::GrpcLibrary::Skill skill;
                skill.set_name(btn->Name().toStdString());
                skill.set_desc(btn->Desc().toStdString());
                skill.set_publisher(btn->Publisher().toStdString());
                skill.set_version(btn->Version().toStdString());
                skill.set_timestamp(
                    btn->Timestamp().toString(Qt::ISODate).toStdString());
                skill.set_hash(btn->Hash().toStdString());
                skills.append(skill);
            }
            NewSessionDialog(skills, this).exec();
        }
        break;
        case 1: { // delete session
            qDebug() << "Delete session button clicked.";
            auto rows = ui->tbviewHistory->selectionModel()->selectedRows();
            QVector<int64_t> row_ids;
            for(auto row : rows)
                row_ids.append(row.siblingAtColumn(0).data().toLongLong());
            _delSessions(row_ids);
        }
        break;
        case 2: { // session history settings
            qDebug() << "Session settings button clicked.";
            HistorySettingDialog(this).exec();
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
    qDebug() << "HomePageWidget: New session response received, session id: "
             << session.id();
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

void HomePageWidget::_slotGetSkillInfoResp(
    const int errorCode, const QVector<::GrpcLibrary::Skill> &skills)
{
    qDebug() << "Get skill info response received with " << skills.size()
             << " items.";
    _addSkills(skills);
}

void HomePageWidget::_slotDownloadResp(const int      errorCode,
                                       const QString &hash,
                                       const QString &addr,
                                       const int64_t  size_kb)
{
    qDebug() << "Download response received, hash: " << hash
             << ", address: " << addr << ", size: " << size_kb << " KB";

    if(errorCode != OK)
        return;

    // handle downloaded skill content, e.g. save to file and load into UI
    for(auto item : m_pSkillsBtnGroup->buttons())
    {
        SkillBtn *btn = qobject_cast<SkillBtn *>(item);
        if(!btn || btn->Hash() != hash)
            continue;

        btn->SetUrl(addr);
        btn->SetState(SkillBtn::State::WaitDownload);
        break;
    }
}

void HomePageWidget::_slotSkillBtnStateChanged(SkillBtn       *btn,
                                               SkillBtn::State state)
{
    qDebug() << "Skill button state changed, hash: " << btn->Hash()
             << ", new state: " << static_cast<int>(state);
    // Handle skill button state change, e.g. update UI or trigger actions
    switch(state)
    {
        case SkillBtn::State::Unknown: {
            qDebug() << "SkillBtn state is Unknown, hash: " << btn->Hash();
        }
        break;
        case SkillBtn::State::WaitDownload: {
            qDebug() << "SkillBtn is waiting to download, hash: "
                     << btn->Hash();
            _download(btn, btn->Url());
        }
        break;
        case SkillBtn::State::Downloading: {
            qDebug() << "SkillBtn is downloading, hash: " << btn->Hash();
        }
        break;
        case SkillBtn::State::Downloaded: {
            qDebug() << "SkillBtn has been downloaded, hash: " << btn->Hash();

            // start installing the skill plugin
            QString zipPath = QString("%1/tmp/%2.zip")
                                  .arg(QCoreApplication::applicationDirPath())
                                  .arg(btn->Name());
            QString destDir = QString("%1/plugins/%2")
                                  .arg(QCoreApplication::applicationDirPath())
                                  .arg(btn->Name());
            Zipper  zipper{zipPath, this};
            if(!zipper.UnZip(destDir))
            {
                qDebug() << "Failed to unzip skill plugin from " << zipPath
                         << " to " << destDir;
                QDir(destDir).removeRecursively(); // clean up
                QFile::remove(zipPath);            // remove the zip file
                btn->SetState(SkillBtn::State::Unknown);
                QMessageBox::critical(
                    this,
                    tr("Download Failed"),
                    tr("Failed to download skill plugin: %1").arg(btn->Name()));
            } else
            {
                qDebug() << "Successfully unzipped skill plugin to " << destDir;
                btn->SetState(SkillBtn::State::Installing);
                QFile::remove(
                    zipPath); // remove the zip file after installation
            }
        }
        break;
        case SkillBtn::State::Installing: {
            qDebug() << "SkillBtn is installing, hash: " << btn->Hash();
            QString destDir = QString("%1/plugins/%2")
                                  .arg(QCoreApplication::applicationDirPath())
                                  .arg(btn->Name());
            // load the plugin
            auto plugins = PluginMgr::Instance()->Search(
                destDir,
                [](const QJsonObject &metaData) -> bool {
                    return metaData.contains("PluginId")
                           && metaData.contains("Version")
                           && metaData.contains("Name")
                           && metaData.contains("Description")
                           && metaData.contains("Author")
                           && metaData.contains("Organization")
                           && metaData.contains("Dependencies");
                });

            for(auto fpath : plugins)
            {
                qDebug() << "Found plugin file: " << fpath;
                PluginInterface *plugin = PluginMgr::Instance()->Load(fpath);
                if(plugin)
                {
                    qDebug()
                        << "Successfully loaded skill plugin from " << destDir;
                    btn->SetState(SkillBtn::State::Installed);
                } else
                {
                    qDebug() << "Failed to load skill plugin from " << destDir;
                    QDir(destDir).removeRecursively(); // clean up
                    btn->SetState(
                        SkillBtn::State::Unknown); // reset state to allow retry
                    QMessageBox::critical(
                        this,
                        tr("Installation Failed"),
                        tr("Failed to install skill plugin: %1")
                            .arg(btn->Name()));
                }
            }
        }
        break;
        case SkillBtn::State::Installed: {
            qDebug() << "SkillBtn is installed, hash: " << btn->Hash();
        }
        break;
        default:
            break;
    }
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
        m_pHistoryModel->setItem(n_row,
                                 0,
                                 new QStandardItem(QString::number(item.id())));
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

void HomePageWidget::_addSkills(const QVector<::GrpcLibrary::Skill> &skills)
{
    int idx = 0;
    foreach(const ::GrpcLibrary::Skill &skill, skills)
    {
        SkillBtn *btn = new SkillBtn(ui->scrollAreaSkills);
        btn->SetId(skill.id());
        btn->SetName(skill.name().c_str());
        btn->SetDesc(skill.desc().c_str());
        btn->SetPublisher(skill.publisher().c_str());
        btn->SetVersion(skill.version().c_str());
        btn->SetTimestamp(skill.timestamp().c_str());
        btn->SetHash(skill.hash().c_str());
        btn->SetState(SkillBtn::State::Unknown);

        auto name   = btn->Name();
        auto plugin = PluginMgr::Instance()->Get(name);
        if(plugin && plugin->Version() == btn->Version())
        {
            qDebug() << "Skill " << btn->Name() << " is already installed.";
            btn->SetState(SkillBtn::State::Installed);
        }

        connect(btn,
                &SkillBtn::SignalStateChanged,
                this,
                &HomePageWidget::_slotSkillBtnStateChanged);

        m_pSkillsBtnGroup->addButton(btn);
        idx++;
    }

    _drawSkillsArea();
}

void HomePageWidget::_getSkills(
    QVector<::GrpcLibrary::Skill>              &skills,
    std::function<bool(::GrpcLibrary::Skill &)> filter)
{
    for(auto item : m_pSkillsBtnGroup->buttons())
    {
        if(!item)
            continue;

        SkillBtn *btn = qobject_cast<SkillBtn *>(item);
        if(!btn)
            continue;


        ::GrpcLibrary::Skill skill;
        skill.set_name(btn->Name().toStdString());
        skill.set_desc(btn->Desc().toStdString());
        skill.set_publisher(btn->Publisher().toStdString());
        skill.set_version(btn->Version().toStdString());
        skill.set_timestamp(
            btn->Timestamp().toString(Qt::ISODate).toStdString());
        skill.set_hash(btn->Hash().toStdString());
        if(!filter(skill))
            continue;

        skills.append(skill);
    }
}

void HomePageWidget::_clearSkills()
{
    foreach(QAbstractButton *btn, m_pSkillsBtnGroup->buttons())
    {
        ui->grid_ProScroll->removeWidget(btn);
        btn->deleteLater();
    }
    m_pSkillsBtnGroup->buttons().clear();

    _drawSkillsArea();
}

void HomePageWidget::_drawSkillsArea()
{
    int width  = 150;
    width      = qMax(width, ui->scrollAreaSkills->width() / m_colNum - 20);
    int height = width; // make skill button square
    for(int i = 0; i < m_pSkillsBtnGroup->buttons().count(); i++)
    {
        QAbstractButton *item = m_pSkillsBtnGroup->buttons().at(i);
        if(!item)
            continue;

        SkillBtn *pBtn = qobject_cast<SkillBtn *>(item);
        if(!pBtn)
            continue;

        pBtn->Resize(width, height);
        ui->grid_ProScroll->addWidget(pBtn, i / m_colNum, i % m_colNum);
    }
}

void HomePageWidget::_download(SkillBtn *btn, const QUrl &url)
{
    auto saveFilePath = QString("%1/tmp/%2.zip")
                            .arg(QCoreApplication::applicationDirPath())
                            .arg(btn->Name());
    btn->Download(url, saveFilePath);
}