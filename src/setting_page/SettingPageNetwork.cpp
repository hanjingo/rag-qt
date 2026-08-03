#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QButtonGroup>

#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <QMessageBox>
#include <QSaveFile>
#include <QDateTime>

#include "SettingPageNetwork.h"
#include "ui_SettingPageNetwork.h"

#include "Global.h"
#include "GrpcClient.h"

#include "StyleMgr.h"

SettingPageNetwork::SettingPageNetwork(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageNetwork)
    , m_pNetConfigCkGroup(new QButtonGroup(this))
{
    ui->setupUi(this);

    m_lastConfigCkboxId = 0;
    _initUI();
    _initConnections();
    _retranslate();
}

SettingPageNetwork::~SettingPageNetwork()
{
}

QVector<Config::NetworkConfig> SettingPageNetwork::GetNetworkConfigs()
{
    QVector<Config::NetworkConfig> configs;
    if(!ui->editCoreServIP->text().isEmpty()
       && !ui->editCoreServPort->text().isEmpty())
    {
        Config::NetworkConfig config;
        config.ip       = ui->editCoreServIP->text();
        config.port     = ui->editCoreServPort->text().toInt();
        config.isEnable = ui->ckCoreEnable->isChecked();
        configs.append(config);
    }

    if(!ui->editCoreServIPBackup1->text().isEmpty()
       && !ui->editCoreServPortBackup1->text().isEmpty())
    {
        Config::NetworkConfig config;
        config.ip       = ui->editCoreServIPBackup1->text();
        config.port     = ui->editCoreServPortBackup1->text().toInt();
        config.isEnable = ui->ckCoreEnableBackup1->isChecked();
        configs.append(config);
    }

    if(!ui->editCoreServIPBackup2->text().isEmpty()
       && !ui->editCoreServPortBackup2->text().isEmpty())
    {
        Config::NetworkConfig config;
        config.ip       = ui->editCoreServIPBackup2->text();
        config.port     = ui->editCoreServPortBackup2->text().toInt();
        config.isEnable = ui->ckCoreEnableBackup2->isChecked();
        configs.append(config);
    }
    return configs;
}

void SettingPageNetwork::_slotPong(const int64_t timestamp)
{
    qDebug() << "Pong received with timestamp:" << timestamp;
    auto ms = QDateTime::currentMSecsSinceEpoch();
    ui->lblCoreServDelay->setText(QString::number(ms - timestamp));
}

void SettingPageNetwork::_slotBtnSaveClicked()
{
    qDebug()
        << "Save button clicked. Implement network settings save logic here.";
    _saveConfigFiles();
}

void SettingPageNetwork::_slotBtnNetTestClicked()
{
    qDebug()
        << "Network Test button clicked. Implement network test logic here.";
    _testNetwork();
}

void SettingPageNetwork::_slotNetConfigCkGroupClicked(int id)
{
    qDebug() << "Network config checkbox clicked. id: " << id;
    if(id == m_lastConfigCkboxId)
        return;

    auto confs = GetNetworkConfigs();
    if(id >= confs.size())
        return;

    m_lastConfigCkboxId = id;
    // logout and switch to the login page
    emit this->signalSwitchAccount();
}

void SettingPageNetwork::_initUI()
{
    _loadConfigFiles();

    ui->btnSave->setStyleSheet(StyleMgr::ParseFile(":/styles/push_button"));
    ui->btnNetTest->setStyleSheet(StyleMgr::ParseFile(":/styles/push_button"));

    m_pNetConfigCkGroup->addButton(ui->ckCoreEnable, 0);
    m_pNetConfigCkGroup->addButton(ui->ckCoreEnableBackup1, 1);
    m_pNetConfigCkGroup->addButton(ui->ckCoreEnableBackup2, 2);
    m_pNetConfigCkGroup->setExclusive(true);
}

void SettingPageNetwork::_retranslate()
{
}

void SettingPageNetwork::_initConnections()
{
    connect(GrpcClient::instance(),
            &GrpcClient::signalPong,
            this,
            &SettingPageNetwork::_slotPong);

    connect(ui->btnSave,
            &QPushButton::clicked,
            this,
            &SettingPageNetwork::_slotBtnSaveClicked);

    connect(ui->btnNetTest,
            &QPushButton::clicked,
            this,
            &SettingPageNetwork::_slotBtnNetTestClicked);

    connect(m_pNetConfigCkGroup,
            &QButtonGroup::idClicked,
            this,
            &SettingPageNetwork::_slotNetConfigCkGroupClicked);
}

void SettingPageNetwork::_saveConfigFiles()
{
    auto confs = GetNetworkConfigs();
    Config::instance()->setNetworkConfigs(confs);
    Config::instance()->save(Config::getConfigFilePath());
}

void SettingPageNetwork::_loadConfigFiles()
{
    auto confs = Config::instance()->networkConfigs();
    for(int i = 0; i < confs.size(); ++i)
    {
        const auto &config   = confs[i];
        QString     ip       = config.ip;
        int         port     = config.port;
        bool        isEnable = config.isEnable;
        switch(i)
        {
            case 0:
                ui->editCoreServIP->setText(ip);
                ui->editCoreServPort->setText(QString::number(port));
                ui->ckCoreEnable->setChecked(isEnable);
                break;
            case 1:
                ui->editCoreServIPBackup1->setText(ip);
                ui->editCoreServPortBackup1->setText(QString::number(port));
                ui->ckCoreEnableBackup1->setChecked(isEnable);
                break;
            case 2:
                ui->editCoreServIPBackup2->setText(ip);
                ui->editCoreServPortBackup2->setText(QString::number(port));
                ui->ckCoreEnableBackup2->setChecked(isEnable);
                break;
            default:
                qDebug() << "More than 3 network configs found in config file.";
                break;
        }
    }
}

void SettingPageNetwork::_testNetwork()
{
    _resetDelayValues();
    auto configs = GetNetworkConfigs();
    for(int i = 0; i < configs.size(); ++i)
    {
        const auto &config = configs[i];
        qDebug() << "Testing network config " << i << ": " << config.ip << ":"
                 << config.port;
        GrpcClient cli;
        connect(&cli,
                &GrpcClient::signalPong,
                [this, i](const int64_t timestamp) {
                    qDebug() << "Client " << i
                             << " Pong received from test network.";
                    auto str = QString::number(
                        QDateTime::currentMSecsSinceEpoch() - timestamp);
                    switch(i)
                    {
                        case 0:
                            ui->lblCoreServDelay->setText(str);
                            break;
                        case 1:
                            ui->lblCoreServDelayBackup1->setText(str);
                            break;
                        case 2:
                            ui->lblCoreServDelayBackup2->setText(str);
                            break;
                        default:
                            break;
                    }
                });

        cli.Connect(config.ip + ":" + QString::number(config.port));
        auto now = QDateTime::currentMSecsSinceEpoch();
        cli.Heartbeat(now);
        qDebug() << "Client " << i << " heartbeat to " << config.ip << ":"
                 << config.port << " finished.";
    }
}

void SettingPageNetwork::_resetDelayValues()
{
    ui->lblCoreServDelay->setText("N/A");
    ui->lblCoreServDelayBackup1->setText("N/A");
    ui->lblCoreServDelayBackup2->setText("N/A");
}