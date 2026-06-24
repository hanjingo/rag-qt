#include "ModelConfigDialog.h"
#include "ui_ModelConfigDialog.h"

ModelConfigDialog::ModelConfigDialog(const Bus::ModelConfig &conf,
                                     QWidget                *parent)
    : QDialog(parent)
    , ui(new Ui::ModelConfigDialog)
    , m_conf(conf)
{
    ui->setupUi(this);

    _initUI();
    _retranslate();
    _initConnections();
}

ModelConfigDialog::~ModelConfigDialog()
{
    delete ui;
}

Bus::ModelConfig ModelConfigDialog::GetConfig()
{
    m_conf.hash      = ui->editHash->text();
    m_conf.name      = ui->editName->text();
    m_conf.publisher = ui->editPublisher->text();
    m_conf.timestamp = ui->editTimestamp->text();
    m_conf.addr      = ui->editAddr->text();
    // m_conf.capabilities = ui->editCapabilities->text();
    m_conf.contextSize = ui->editMaxTokens->text().toLongLong();
    m_conf.cost        = ui->editPrice->text().toInt();

    m_conf.apiKey            = ui->editApiKey->text();
    m_conf.temperature       = ui->editTemperature->text().toFloat();
    m_conf.topP              = ui->editTopP->text().toFloat();
    m_conf.topK              = ui->editTopK->text().toFloat();
    m_conf.reputationPenalty = ui->editPenalty->text().toFloat();
    m_conf.maxTokens         = ui->editMaxTokens->text().toLongLong();
    m_conf.stopWords         = ui->editStopWords->text();
    m_conf.prompt            = ui->editPrompt->toPlainText();
    return m_conf;
}

void ModelConfigDialog::_retranslate()
{
}

void ModelConfigDialog::_initUI()
{
    ui->editHash->setText(m_conf.hash);
    ui->editName->setText(m_conf.name);
    ui->editPublisher->setText(m_conf.publisher);
    ui->editTimestamp->setText(m_conf.timestamp);
    ui->editAddr->setText(m_conf.addr);
    ui->editMaxTokens->setText(QString::number(m_conf.contextSize));
    ui->editPrice->setText(QString::number(m_conf.cost));

    ui->editApiKey->setText(m_conf.apiKey);
    ui->editTemperature->setText(QString::number(m_conf.temperature));
    ui->editTopP->setText(QString::number(m_conf.topP));
    ui->editTopK->setText(QString::number(m_conf.topK));
    ui->editPenalty->setText(QString::number(m_conf.reputationPenalty));
    ui->editMaxTokens->setText(QString::number(m_conf.maxTokens));
    ui->editStopWords->setText(m_conf.stopWords);
    ui->editPrompt->setPlainText(m_conf.prompt);
}

void ModelConfigDialog::_initConnections()
{
}