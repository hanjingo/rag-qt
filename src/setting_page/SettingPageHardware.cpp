#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QStandardItemModel>
#include <QListView>

#include "SettingPageHardware.h"
#include "ui_SettingPageHardware.h"

#include "StyleMgr.h"
#include "Config.h"

SettingPageHardware *SettingPageHardware::m_stInstance = nullptr;
SettingPageHardware *SettingPageHardware::Instance()
{
    if(nullptr == m_stInstance)
        m_stInstance = new SettingPageHardware();

    return m_stInstance;
}

SettingPageHardware::SettingPageHardware(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageHardware)
{
    ui->setupUi(this);

    _initConnections();
    _initUI();
}

SettingPageHardware::~SettingPageHardware()
{
    delete ui;
}

void SettingPageHardware::_initUI()
{
    // init left side bar
    ui->listWidgetCatalog->setViewMode(QListView::ListMode);

    QListWidgetItem *itemAudio = new QListWidgetItem(tr("Audio"));
    itemAudio->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    ui->listWidgetCatalog->insertItem(0, itemAudio);

    QListWidgetItem *itemGPU = new QListWidgetItem(tr("GPU"));
    itemGPU->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    ui->listWidgetCatalog->insertItem(1, itemGPU);

    ui->listWidgetCatalog->setCurrentRow(0);

    // init audio config
    auto confs = Config::Instance().translatorParams();
    for(auto conf : confs)
    {
        ui->comboAudioTranslator->addItem(conf.id);
    }
    ui->comboAudioTranslator->setCurrentIndex(0);
}

void SettingPageHardware::_initConnections()
{
    connect(ui->listWidgetCatalog,
            &QListWidget::currentRowChanged,
            ui->stackPage,
            &QStackedWidget::setCurrentIndex);

    connect(ui->comboAudioTranslator,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(_slotComboAudioTranslatorCurrentChanged(int)));
}

void SettingPageHardware::_slotComboAudioTranslatorCurrentChanged(int iIndex)
{
    qDebug() << "Audio translator combo box current index changed: " << iIndex;
    auto id = ui->comboAudioTranslator->itemText(iIndex);
    _switchAudioConfig(id);
}

void SettingPageHardware::_switchAudioConfig(const QString &id)
{
    auto currId = ui->comboAudioTranslator->currentText();
    auto confs  = Config::Instance().translatorParams();
    for(auto conf : confs)
    {
        if(id != currId)
            continue;

        ui->editModelPath->setText(conf.modelPath);
        ui->ckUseGPU->setChecked(conf.useGPU);
        for(int i = 0; i < ui->comboGPU->count(); ++i)
        {
            if(ui->comboGPU->itemText(i).toInt() == conf.gpuDevice)
                ui->comboGPU->setCurrentIndex(i);
        }
        ui->ckFlashAttention->setChecked(conf.flashAttention);

        ui->editThreadsNum->setText(QString::number(conf.nThreads));
        ui->editMaxTextCtx->setText(QString::number(conf.nMaxTextCtx));
        ui->ckTranslate->setChecked(conf.translate);
        ui->ckDetectLanguage->setChecked(conf.detectLanguage);
        ui->editLanguage->setText(conf.language);
        ui->ckNoCtx->setChecked(conf.noCtx);
        ui->ckNoTimestamps->setChecked(conf.noTimestamps);
        ui->ckSingleSegment->setChecked(conf.singleSegment);
        ui->ckPrintSpecial->setChecked(conf.printSpecial);
        ui->ckPrintProgress->setChecked(conf.printProgress);
        ui->ckPrintRealTime->setChecked(conf.printRealtime);
        ui->ckCarryInitPrompt->setChecked(conf.carryInitialPrompt);
        ui->editInitPrompt->setText(conf.initialPrompt);
        ui->ckSuppressBlank->setChecked(conf.suppressBlank);
        ui->ckSuppressNst->setChecked(conf.suppressBlank);
        ui->editSuppressRegex->setText(conf.suppressRegex);
        ui->editTemperature->setText(QString::number(conf.temperature));
        ui->editTemperatureInc->setText(QString::number(conf.temperatureInc));
        ui->editMaxInitTs->setText(QString::number(conf.maxInitialTs));
        ui->editLengthPenalty->setText(QString::number(conf.lengthPenalty));
        ui->editEntropyThreshold->setText(QString::number(conf.entropyThold));
        ui->editLogProbThreshold->setText(QString::number(conf.logprobThold));

        ui->editMinNewSampleSize->setText(
            QString::number(conf.minNewSampleSize));
        ui->editMinAudioBufferSize->setText(
            QString::number(conf.minAudioBufferSize));
        ui->editMuteAmplitudeDur->setText(QString::number(conf.muteAmplitudeDurationMs));
        ui->editMuteAmplitudeThreshold->setText(
            QString::number(conf.muteAmplitudeThreshold));
        ui->editKeepLastAudioBuffer->setText(
            QString::number(conf.keepLastAudioBufferMs));
        ui->editSleepTimeoutMs->setText(QString::number(conf.sleepTimeoutMs));
        ui->editWakeupThreshold->setText(QString::number(conf.wakeupThreshold));
        ui->editMaxSameContentCount->setText(
            QString::number(conf.maxSameContentCount));

    }
}