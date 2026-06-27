#include "ModelConfigDialog.h"
#include "ui_ModelConfigDialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include "Global.h"

ModelConfigDialog::ModelConfigDialog(const Bus::ModelConfig &conf,
                                     QWidget                *parent)
    : QDialog(parent)
    , ui(new Ui::ModelConfigDialog)
    , m_pPipelineBtnGroup(new QButtonGroup(this))
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
    m_conf.id        = ui->editID->text();
    m_conf.name      = ui->editName->text();
    m_conf.publisher = ui->editPublisher->text();
    m_conf.timestamp = ui->editTimestamp->text();
    m_conf.addr      = ui->editAddr->text();
    m_conf.cost      = ui->editPrice->text().toInt();

    m_conf.apiKey            = ui->editApiKey->text();
    m_conf.temperature       = ui->editTemperature->text().toFloat();
    m_conf.topP              = ui->editTopP->text().toFloat();
    m_conf.topK              = ui->editTopK->text().toFloat();
    m_conf.reputationPenalty = ui->editPenalty->text().toFloat();
    m_conf.minP              = ui->editMinP->text().toFloat();
    m_conf.stopWords         = ui->editStopWords->text();
    m_conf.prompt            = ui->editPrompt->toPlainText();

    if(ui->ckLocal->isChecked())
        m_conf.pipeline = PIPELINE_LOCAL;
    else if(ui->ckRemote->isChecked())
        m_conf.pipeline = PIPELINE_REMOTE;
    else if(ui->ckHybrid->isChecked())
        m_conf.pipeline = PIPELINE_HYBRID;

    return m_conf;
}

void ModelConfigDialog::_retranslate()
{
}

void ModelConfigDialog::_initUI()
{
    ui->editID->setText(m_conf.id);
    ui->editName->setText(m_conf.name);
    ui->editPublisher->setText(m_conf.publisher);
    ui->editTimestamp->setText(m_conf.timestamp);
    ui->editAddr->setText(m_conf.addr);
    ui->editPrice->setText(QString::number(m_conf.cost));

    ui->editApiKey->setText(m_conf.apiKey);
    ui->editTemperature->setText(QString::number(m_conf.temperature));
    ui->editTopP->setText(QString::number(m_conf.topP));
    ui->editTopK->setText(QString::number(m_conf.topK));
    ui->editPenalty->setText(QString::number(m_conf.reputationPenalty));
    ui->editMinP->setText(QString::number(m_conf.minP));
    ui->editStopWords->setText(m_conf.stopWords);
    ui->editPrompt->setPlainText(m_conf.prompt);

    m_pPipelineBtnGroup->addButton(ui->ckLocal);
    m_pPipelineBtnGroup->addButton(ui->ckRemote);
    m_pPipelineBtnGroup->addButton(ui->ckHybrid);
    m_pPipelineBtnGroup->setExclusive(true);

    if(m_conf.pipeline == PIPELINE_LOCAL)
    {
        ui->ckLocal->setChecked(true);
    } else if(m_conf.pipeline == PIPELINE_REMOTE)
    {
        ui->ckRemote->setChecked(true);
    } else if(m_conf.pipeline == PIPELINE_HYBRID)
    {
        ui->ckHybrid->setChecked(true);
    }
}

void ModelConfigDialog::_initConnections()
{
    connect(ui->btnModelAddr,
            &QPushButton::clicked,
            this,
            &ModelConfigDialog::_slotBtnModelAddrClicked);
}

void ModelConfigDialog::_slotBtnModelAddrClicked()
{
    qDebug() << "Model Addr button clicked.";
    // choose model file path
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Select Model File"),
                                                    "",
                                                    tr("Model Files (*.gguf)"));
    if(filePath.isEmpty())
        return;

    // check if file exists
    QFileInfo fileInfo(filePath);
    if(!fileInfo.exists() || !fileInfo.isFile())
    {
        QMessageBox::warning(this,
                             tr("File Not Found"),
                             tr("The selected file does not exist."));
        return;
    }

    ui->editAddr->setText(filePath);
}