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

SettingPageNetwork *SettingPageNetwork::m_stSettingPageNetworkInst = nullptr;

SettingPageNetwork *SettingPageNetwork::Instance()
{
    if(nullptr == m_stSettingPageNetworkInst)
    {
        m_stSettingPageNetworkInst = new SettingPageNetwork();
    }

    return m_stSettingPageNetworkInst;
}

SettingPageNetwork::SettingPageNetwork(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageNetwork)
{
    ui->setupUi(this);

    _initUI();
    _initConnections();
    _retranslate();
}

SettingPageNetwork::~SettingPageNetwork()
{
    delete ui;
}

QVector<NetworkConfig> SettingPageNetwork::GetNetworkConfigs()
{
    QVector<NetworkConfig> configs;
    if(!ui->editCoreServIP->text().isEmpty()
       && !ui->editCoreServPort->text().isEmpty())
    {
        NetworkConfig config;
        config.ip       = ui->editCoreServIP->text();
        config.port     = ui->editCoreServPort->text().toInt();
        config.isEnable = ui->ckCoreEnable->isChecked();
        configs.append(config);
    }

    if(!ui->editCoreServIPBackup1->text().isEmpty()
       && !ui->editCoreServPortBackup1->text().isEmpty())
    {
        NetworkConfig config;
        config.ip       = ui->editCoreServIPBackup1->text();
        config.port     = ui->editCoreServPortBackup1->text().toInt();
        config.isEnable = ui->ckCoreEnableBackup1->isChecked();
        configs.append(config);
    }

    if(!ui->editCoreServIPBackup2->text().isEmpty()
       && !ui->editCoreServPortBackup2->text().isEmpty())
    {
        NetworkConfig config;
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
    ui->lblCoreServDelay->setText(QString::number(ms - timestamp) + " ms");
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
    GrpcClient::Instance()->Heartbeat(QDateTime::currentMSecsSinceEpoch());
}

void SettingPageNetwork::_initUI()
{
    _loadConfigFiles();
}

void SettingPageNetwork::_retranslate()
{
}

void SettingPageNetwork::_initConnections()
{
    connect(GrpcClient::Instance(),
            &GrpcClient::SignalPong,
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
}

void SettingPageNetwork::_saveConfigFiles()
{
    // Read existing file
    QFile         readFile(CONFIG_FILE);
    QJsonDocument doc;
    if(readFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray      data = readFile.readAll();
        QJsonParseError parseError;
        doc = QJsonDocument::fromJson(data, &parseError);
        if(parseError.error != QJsonParseError::NoError || !doc.isObject())
        {
            doc = QJsonDocument(QJsonObject());
        }
        readFile.close();
    } else
    {
        doc = QJsonDocument(QJsonObject());
    }

    QJsonObject rootObj = doc.object();
    QJsonArray  confArr;
    auto        confs = GetNetworkConfigs();
    for(auto conf : confs)
    {
        QJsonObject confObj;
        confObj["ip"]       = conf.ip;
        confObj["port"]     = conf.port;
        confObj["isEnable"] = conf.isEnable;
        confArr.append(confObj);
    }

    rootObj["network_configs"] = confArr;
    doc.setObject(rootObj);
    QSaveFile saveFile(CONFIG_FILE);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing: "
                 << saveFile.errorString();
        QMessageBox::warning(this,
                             tr("Save Failed"),
                             tr("Failed to open file for writing: %1")
                                 .arg(saveFile.errorString()));
        return;
    }

    QTextStream out(&saveFile);
    out << doc.toJson(QJsonDocument::Indented);
    if(!saveFile.commit()) // Atomically replaces the file
    {
        QMessageBox::warning(
            this,
            tr("Save Failed"),
            tr("Failed to save file: %1").arg(saveFile.errorString()));
        return;
    }

    QMessageBox::information(
        this,
        tr("Save Successful"),
        tr("Network config exported to file: %1").arg(CONFIG_FILE));
}

void SettingPageNetwork::_loadConfigFiles()
{
    QFile         readFile(CONFIG_FILE);
    QJsonDocument doc;
    if(readFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray      data = readFile.readAll();
        QJsonParseError parseError;
        doc = QJsonDocument::fromJson(data, &parseError);
        if(parseError.error != QJsonParseError::NoError || !doc.isObject())
        {
            qDebug() << "Failed to parse JSON from config file: "
                     << parseError.errorString();
            return;
        }
        readFile.close();
    } else
    {
        qDebug() << "Failed to open config file for reading: "
                 << readFile.errorString();
        return;
    }

    QJsonObject rootObj = doc.object();
    if(!rootObj.contains("network_configs")
       || !rootObj["network_configs"].isArray())
    {
        qDebug() << "Config file does not contain 'network_configs' array.";
        return;
    }

    QJsonArray confArr = rootObj["network_configs"].toArray();
    for(int i = 0; i < confArr.size(); ++i)
    {
        QJsonObject confObj  = confArr[i].toObject();
        QString     ip       = confObj["ip"].toString();
        int         port     = confObj["port"].toInt();
        bool        isEnable = confObj["isEnable"].toBool();

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