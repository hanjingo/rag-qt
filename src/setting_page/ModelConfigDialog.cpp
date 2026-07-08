#include "ModelConfigDialog.h"
#include "ui_ModelConfigDialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include "Global.h"

ModelConfigDialog::ModelConfigDialog(const Config::ModelConfig &conf,
                                     QWidget                   *parent)
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

Config::ModelConfig ModelConfigDialog::GetConfig()
{
    // base info
    m_conf.id        = ui->editId->text();
    m_conf.name      = ui->editName->text();
    m_conf.publisher = ui->editPublisher->text();
    m_conf.timestamp = ui->editCreateTime->text();
    m_conf.addr      = ui->editAddr->text();
    if(ui->ckLocal)
        m_conf.pipeline = PIPELINE_LOCAL;
    else if(ui->ckRemote)
        m_conf.pipeline = PIPELINE_REMOTE;
    else
        m_conf.pipeline = PIPELINE_HYBRID;
    m_conf.cost   = ui->editPrice->text().toFloat();
    m_conf.apiKey = ui->editApiKey->text();
    m_conf.hash   = ui->editHash->text();

    // context params
    m_conf.nCtx          = ui->editNumContext->text().toInt();
    m_conf.nBatch        = ui->editNumBatch->text().toInt();
    m_conf.nUbatch       = ui->editNumUBatch->text().toInt();
    m_conf.nSeqMax       = ui->editNumSeqMax->text().toInt();
    m_conf.nThreads      = ui->editNumThreads->text().toInt();
    m_conf.nThreadsBatch = ui->editNumThreadsBatch->text().toInt();

    m_conf.ropeFreqBase   = ui->editRopeFreqBase->text().toFloat();
    m_conf.ropeFreqScale  = ui->editRopeFreqScale->text().toFloat();
    m_conf.yarnExtFactor  = ui->editYarnExtFactor->text().toFloat();
    m_conf.yarnAttnFactor = ui->editYarnAttnFactor->text().toFloat();
    m_conf.yarnBetaFast   = ui->editYarnBetaFast->text().toFloat();
    m_conf.yarnBetaSlow   = ui->editYarnBetaSlow->text().toFloat();
    m_conf.yarnOrigCtx    = ui->editYarnOrigContext->text().toInt();
    m_conf.defragThold    = ui->editDefragThold->text().toFloat();

    m_conf.embeddings = ui->ckEmbeddings->isChecked();
    m_conf.offloadKQV = ui->ckOffloadKQV->isChecked();
    m_conf.noPerf     = ui->ckNoPerf->isChecked();
    m_conf.opOffload  = ui->ckOPOffload->isChecked();
    m_conf.swaFull    = ui->ckSwaFull->isChecked();
    m_conf.kvUnified  = ui->ckKVUnified->isChecked();

    // sampling parameters
    m_conf.penaltyLastN   = ui->editPenaltyLastN->text().toInt();
    m_conf.penaltyRepeat  = ui->editPenaltyRepeat->text().toFloat();
    m_conf.penaltyFreq    = ui->editPenaltyFreq->text().toFloat();
    m_conf.penaltyPresent = ui->editPenaltyPresent->text().toFloat();

    m_conf.temperature         = ui->editTemperature->text().toFloat();
    m_conf.temperatureExt      = ui->editTemperatureExt->text().toFloat();
    m_conf.temperatureExtDelta = ui->editTemperatureExtDelta->text().toFloat();
    m_conf.temperatureExtExponent =
        ui->editTemperatureExtExponent->text().toFloat();

    m_conf.seed = ui->editSeed->text().toInt();

    m_conf.topP        = ui->editTopP->text().toFloat();
    m_conf.topPMinKeep = ui->editTopPMinKeep->text().toInt();
    m_conf.topK        = ui->editTopK->text().toInt();
    m_conf.minP        = ui->editMinP->text().toFloat();
    m_conf.minPMinKeep = ui->editMinPMinKeep->text().toInt();

    // control parameters
    m_conf.ctxWindowSize = ui->editCtxWindowSize->text().toInt();
    m_conf.stopWords     = ui->editStopWords->text();

    // prompt
    m_conf.prompt = ui->editPrompt->toPlainText();

    return m_conf;
}

void ModelConfigDialog::_retranslate()
{
}

void ModelConfigDialog::_initUI()
{
    // base info
    ui->editId->setText(m_conf.id);
    ui->editName->setText(m_conf.name);
    ui->editPublisher->setText(m_conf.publisher);
    ui->editCreateTime->setText(m_conf.timestamp);
    ui->editAddr->setText(m_conf.addr);
    if(m_conf.pipeline == PIPELINE_LOCAL)
        ui->ckLocal->setChecked(true);
    else if(m_conf.pipeline == PIPELINE_REMOTE)
        ui->ckRemote->setChecked(true);
    else
        ui->ckHybrid->setChecked(true);
    ui->editPrice->setText(QString::number(m_conf.cost));
    ui->editApiKey->setText(m_conf.apiKey);
    ui->editHash->setText(m_conf.hash);

    // context params
    ui->editNumContext->setText(QString::number(m_conf.nCtx));
    ui->editNumBatch->setText(QString::number(m_conf.nBatch));
    ui->editNumUBatch->setText(QString::number(m_conf.nUbatch));
    ui->editNumSeqMax->setText(QString::number(m_conf.nSeqMax));
    ui->editNumThreads->setText(QString::number(m_conf.nThreads));
    ui->editNumThreadsBatch->setText(QString::number(m_conf.nThreadsBatch));

    ui->editRopeFreqBase->setText(QString::number(m_conf.ropeFreqBase, 'f', 1));
    ui->editRopeFreqScale->setText(
        QString::number(m_conf.ropeFreqScale, 'f', 1));
    ui->editYarnExtFactor->setText(
        QString::number(m_conf.yarnExtFactor, 'f', 1));
    ui->editYarnAttnFactor->setText(
        QString::number(m_conf.yarnAttnFactor, 'f', 1));
    ui->editYarnBetaFast->setText(QString::number(m_conf.yarnBetaFast, 'f', 1));
    ui->editYarnBetaSlow->setText(QString::number(m_conf.yarnBetaSlow, 'f', 1));
    ui->editYarnOrigContext->setText(QString::number(m_conf.yarnOrigCtx));
    ui->editDefragThold->setText(QString::number(m_conf.defragThold, 'f', 1));

    ui->ckEmbeddings->setChecked(m_conf.embeddings);
    ui->ckOffloadKQV->setChecked(m_conf.offloadKQV);
    ui->ckNoPerf->setChecked(m_conf.noPerf);
    ui->ckOPOffload->setChecked(m_conf.opOffload);
    ui->ckSwaFull->setChecked(m_conf.swaFull);
    ui->ckKVUnified->setChecked(m_conf.kvUnified);

    // sampling parameters
    ui->editPenaltyLastN->setText(QString::number(m_conf.penaltyLastN));
    ui->editPenaltyRepeat->setText(
        QString::number(m_conf.penaltyRepeat, 'f', 1));
    ui->editPenaltyFreq->setText(QString::number(m_conf.penaltyFreq, 'f', 1));
    ui->editPenaltyPresent->setText(
        QString::number(m_conf.penaltyPresent, 'f', 1));

    ui->editTemperature->setText(QString::number(m_conf.temperature, 'f', 1));
    ui->editTemperatureExt->setText(
        QString::number(m_conf.temperatureExt, 'f', 1));
    ui->editTemperatureExtDelta->setText(
        QString::number(m_conf.temperatureExtDelta, 'f', 1));
    ui->editTemperatureExtExponent->setText(
        QString::number(m_conf.temperatureExtExponent, 'f', 1));

    ui->editSeed->setText(QString::number(m_conf.seed));

    ui->editTopP->setText(QString::number(m_conf.topP, 'f', 1));
    ui->editTopPMinKeep->setText(QString::number(m_conf.topPMinKeep));
    ui->editTopK->setText(QString::number(m_conf.topK));
    ui->editMinP->setText(QString::number(m_conf.minP, 'f', 1));
    ui->editMinPMinKeep->setText(QString::number(m_conf.minPMinKeep));

    // control parameters
    ui->editCtxWindowSize->setText(QString::number(m_conf.ctxWindowSize));
    ui->editStopWords->setText(m_conf.stopWords);

    // prompt
    ui->editPrompt->setText(m_conf.prompt);
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