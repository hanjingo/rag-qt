#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QStandardItemModel>
#include <QListView>

#include "SettingPageHardware.h"
#include "ui_SettingPageHardware.h"

#include "StyleMgr.h"

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
}

void SettingPageHardware::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if(event->type() == QEvent::LanguageChange)
    {
        qDebug() << "SettingPageHardware language change event received.";
        ui->retranslateUi(this);
        _retranslate();
    }
}

void SettingPageHardware::_initUI()
{
    // init left side bar
    ui->listWidgetCatalog->setViewMode(QListView::ListMode);

    QListWidgetItem *itemAudioTranslator =
        new QListWidgetItem(tr("Audio Translator"));
    itemAudioTranslator->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    ui->listWidgetCatalog->insertItem(0, itemAudioTranslator);

    // QListWidgetItem *itemGPU = new QListWidgetItem(tr("GPU"));
    // itemGPU->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    // ui->listWidgetCatalog->insertItem(1, itemGPU);

    // init bottom button
    ui->btnSave->setStyleSheet(StyleMgr::ParseFile(":/styles/push_button"));

    // not allowed to edit the model path
    ui->editVADModelPath->setDisabled(true);

    // init asr param config page
    ui->spinNoSpeechThreshold->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));
    ui->spinLogProbThreshold->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));
    ui->spinEntropyThreshold->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));
    ui->spinLengthPenalty->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));
    ui->spinMaxInitTs->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));
    ui->spinTemperatureInc->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));
    ui->spinTemperature->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));

    ui->spinVADThreshold->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));
    ui->spinVADMaxSpeechDurS->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));
    ui->spinVADSamplesOverlap->setStyleSheet(
        StyleMgr::ParseFile(":/styles/double_spin_box"));
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

    connect(ui->btnSave,
            &QPushButton::clicked,
            this,
            &SettingPageHardware::_slotBtnSaveClicked);

    connect(Config::instance(),
            &Config::signalAsrConfigUpdate,
            this,
            &SettingPageHardware::_slotAsrConfigUpdate);

    connect(ui->ckVADEnable,
            &QCheckBox::toggled,
            this,
            &SettingPageHardware::_slotVADEnabled);
}

void SettingPageHardware::_retranslate()
{
    ui->btnSave->setText(tr("Save"));

    ui->listWidgetCatalog->item(0)->setText(tr("Audio Translator"));
    ui->lblAudioTranslatorTitle->setText(tr("Audio Translator"));
    ui->lblFullParamTitle->setText(tr("Full Param"));
    ui->lblThreadsNum->setText(tr("Threads Num"));
    ui->lblMaxTextCtx->setText(tr("Max Text Context"));
    ui->lblOffsetMs->setText(tr("Offset Milliseconds"));
    ui->lblDurMs->setText(tr("Duration Milliseconds"));
    ui->ckTranslate->setText(tr("Translate"));
    ui->ckDetectLanguage->setText(tr("Detect Language"));
    ui->lblLanguage->setText(tr("Language"));
    ui->ckNoCtx->setText(tr("No Context"));
    ui->ckNoTimestamps->setText(tr("No Timestamps"));
    ui->ckSingleSegment->setText(tr("Single Segment"));
    ui->ckPrintSpecial->setText(tr("Print Special"));
    ui->ckPrintProgress->setText(tr("Print Progress"));
    ui->ckPrintRealTime->setText(tr("Print Realtime"));
    ui->ckPrintTimeStamp->setText(tr("Print Timestamps"));
    ui->lblTemperature->setText(tr("Temperature"));
    ui->lblTemperatureInc->setText(tr("Temperature Increment"));
    ui->lblMaxInitTs->setText(tr("Max Initial Timestamps"));
    ui->lblLengthPenalty->setText(tr("Length Penalty"));
    ui->lblEntropyThreshold->setText(tr("Entropy Threshold"));
    ui->lblLogProbThreshold->setText(tr("Log Probability Threshold"));
    ui->lblNoSpeechThreshold->setText(tr("No Speech Threshold"));

    ui->lblNoiseControlTitle->setText(tr("Noise Control"));
    ui->ckSuppressBlank->setText(tr("Suppress Blank"));
    ui->ckSuppressNst->setText(tr("Suppress NST"));
    ui->lblSuppressRegex->setText(tr("Suppress Regex"));
    ui->ckCarryInitPrompt->setText(tr("Carry Initial Prompt"));
    ui->lblInitPrompt->setText(tr("Initial Prompt"));

    ui->lblVADTitle->setText(tr("Voice Activity Detection"));
    ui->ckVADEnable->setText(tr("Enable"));
    ui->lblVADModelPath->setText(tr("Model Path"));
    ui->lblVADThreshold->setText(tr("Threshold"));
    ui->lblVADMinSpeechDurMs->setText(tr("Min Speech Dur Ms"));
    ui->lblVADMinSilenceDurMs->setText(tr("Min Silence Dur Ms"));
    ui->lblVADMaxSpeechDurS->setText(tr("Max Speech Dur S"));
    ui->lblVADSpeechPadMs->setText(tr("Speech Pad Ms"));
    ui->lblVADSamplesOverlap->setText(tr("Samples Overlap"));

    ui->lblBufferControlTitle->setText(tr("Buffer Control"));
    ui->lblMinNewSampleSize->setText(tr("Min New Sample Size"));
    ui->lblMinAudioBufferSize->setText(tr("Min Audio Buffer Size"));
    ui->lblMaxAudioBufferSize->setText(tr("Max Audio Buffer Size"));

    // reset config value
    auto id   = ui->comboAudioTranslator->currentText();
    auto conf = Config::instance()->getAsrParamById(id);
    if(conf.id == id)
        _set(conf);
}

void SettingPageHardware::_slotComboAudioTranslatorCurrentChanged(int iIndex)
{
    qDebug() << "Audio translator combo box current index changed: " << iIndex;
    if(iIndex < 0 || iIndex >= ui->comboAudioTranslator->count())
        return;

    auto id   = ui->comboAudioTranslator->itemText(iIndex);
    auto conf = Config::instance()->getAsrParamById(id);
    if(conf.id == "" || conf.id != id)
        return;

    _set(conf);
}

void SettingPageHardware::_slotBtnSaveClicked()
{
    qDebug()
        << "Save button clicked. Implement hardware settings save logic here.";
    switch(ui->stackPage->currentIndex()) // audio translator
    {
        case 0: // Audio Translator
        {
            auto audioId = ui->comboAudioTranslator->currentText();
            auto confs   = Config::instance()->getAsrParams();
            for(int i = 0; i < confs.size(); ++i)
            {
                if(confs[i].id != audioId)
                    continue;

                _get(confs[i]);
            }
            Config::instance()->setAsrParams(confs);
            Config::instance()->saveAsr(
                Config::instance()->getAsrConfigFilePath());
        }
        break;
        case 1: // GPU
            // Handle GPU add logic here if needed
            break;
        default:
            break;
    }
}

void SettingPageHardware::_slotAsrConfigUpdate()
{
    // update asr config
    qDebug() << "_slotAsrConfigUpdate";
    ui->comboAudioTranslator->clear();
    auto ids = Config::instance()->getAsrIds();
    for(auto id : ids)
    {
        ui->comboAudioTranslator->addItem(id);
    }

    auto currIndex = ui->comboAudioTranslator->currentIndex();
    if(currIndex >= 0 && currIndex < ids.size())
        ui->comboAudioTranslator->setCurrentIndex(currIndex);
    else
        ui->comboAudioTranslator->setCurrentIndex(0);
}

void SettingPageHardware::_slotVADEnabled(bool isEnabled)
{
    // ui->editVADModelPath->setEnabled(isEnabled);
    ui->spinVADThreshold->setEnabled(isEnabled);
    ui->editVADMinSpeechDurMs->setEnabled(isEnabled);
    ui->editVADMinSilenceDurMs->setEnabled(isEnabled);
    ui->spinVADMaxSpeechDurS->setEnabled(isEnabled);
    ui->editVADSpeechPadMs->setEnabled(isEnabled);
    ui->spinVADSamplesOverlap->setEnabled(isEnabled);
}

void SettingPageHardware::_addAudioConfig(const QString &id)
{
    auto confs = Config::instance()->getAsrParams();
    for(auto conf : confs)
    {
        if(conf.id == id)
        {
            qDebug() << "Audio translator with id" << id
                     << "already exists. Cannot add duplicate.";
            return;
        }
    }

    Config::AsrParam newConf;
    newConf.id = id;
    confs.append(newConf);
    Config::instance()->setAsrParams(confs);

    ui->comboAudioTranslator->addItem(id);
    ui->comboAudioTranslator->setCurrentText(id);
}

void SettingPageHardware::_set(const Config::AsrParam &conf)
{
    for(int i = 0; i < ui->comboAudioTranslator->count(); ++i)
    {
        if(ui->comboAudioTranslator->itemText(i) == conf.id)
            ui->comboAudioTranslator->setCurrentIndex(i);
    }
    ui->editThreadsNum->setText(QString::number(conf.nThreads));
    ui->editMaxTextCtx->setText(QString::number(conf.nMaxTextCtx));
    ui->editOffsetMs->setText(QString::number(conf.offsetMs));
    ui->editDurMs->setText(QString::number(conf.durationMs));
    ui->ckTranslate->setChecked(conf.translate);
    ui->ckDetectLanguage->setChecked(conf.detectLanguage);
    for(int i = 0; i < ui->comboLanguage->count(); ++i)
    {
        if(ui->comboLanguage->itemText(i) == conf.language)
            ui->comboLanguage->setCurrentIndex(i);
    }
    ui->ckNoCtx->setChecked(conf.noCtx);
    ui->ckNoTimestamps->setChecked(conf.noTimestamps);
    ui->ckSingleSegment->setChecked(conf.singleSegment);
    ui->ckPrintSpecial->setChecked(conf.printSpecial);
    ui->ckPrintProgress->setChecked(conf.printProgress);
    ui->ckPrintRealTime->setChecked(conf.printRealtime);
    ui->ckPrintTimeStamp->setChecked(conf.printTimestamps);
    ui->ckCarryInitPrompt->setChecked(conf.carryInitialPrompt);
    ui->editInitPrompt->setText(conf.initialPrompt);
    ui->ckSuppressBlank->setChecked(conf.suppressBlank);
    ui->ckSuppressNst->setChecked(conf.suppressNst);
    ui->editSuppressRegex->setText(conf.suppressRegex);
    ui->spinTemperature->setValue(conf.temperature);
    ui->spinTemperatureInc->setValue(conf.temperatureInc);
    ui->spinMaxInitTs->setValue(conf.maxInitialTs);
    ui->spinLengthPenalty->setValue(conf.lengthPenalty);
    ui->spinEntropyThreshold->setValue(conf.entropyThold);
    ui->spinLogProbThreshold->setValue(conf.logprobThold);
    ui->spinNoSpeechThreshold->setValue(conf.noSpeechThold);

    ui->ckVADEnable->setChecked(conf.vad);
    ui->editVADModelPath->setText(conf.vadModelPath);
    ui->spinVADThreshold->setValue(conf.vadParams.threshold);
    ui->editVADMinSpeechDurMs->setText(
        QString::number(conf.vadParams.minSpeechDurMs));
    ui->editVADMinSilenceDurMs->setText(
        QString::number(conf.vadParams.minSilenceDurMs));
    ui->spinVADMaxSpeechDurS->setValue(conf.vadParams.maxSpeechDurS);
    ui->editVADSpeechPadMs->setText(
        QString::number(conf.vadParams.speechPadMs));
    ui->spinVADSamplesOverlap->setValue(conf.vadParams.samplesOverlap);

    ui->editMinNewSampleSize->setText(QString::number(conf.minNewSampleSize));
    ui->editMinAudioBufferSize->setText(
        QString::number(conf.minAudioBufferSize));
    ui->editMaxAudioBufferSize->setText(
        QString::number(conf.maxAudioBufferSize));
}

void SettingPageHardware::_get(Config::AsrParam &param)
{
    param.id                 = ui->comboAudioTranslator->currentText();
    param.nThreads           = ui->editThreadsNum->text().toInt();
    param.nMaxTextCtx        = ui->editMaxTextCtx->text().toInt();
    param.offsetMs           = ui->editOffsetMs->text().toInt();
    param.durationMs         = ui->editDurMs->text().toInt();
    param.translate          = ui->ckTranslate->isChecked();
    param.detectLanguage     = ui->ckDetectLanguage->isChecked();
    param.language           = ui->comboLanguage->currentText();
    param.noCtx              = ui->ckNoCtx->isChecked();
    param.noTimestamps       = ui->ckNoTimestamps->isChecked();
    param.singleSegment      = ui->ckSingleSegment->isChecked();
    param.printSpecial       = ui->ckPrintSpecial->isChecked();
    param.printProgress      = ui->ckPrintProgress->isChecked();
    param.printRealtime      = ui->ckPrintRealTime->isChecked();
    param.printTimestamps    = ui->ckPrintTimeStamp->isChecked();
    param.carryInitialPrompt = ui->ckCarryInitPrompt->isChecked();
    param.initialPrompt      = ui->editInitPrompt->text();
    param.suppressBlank      = ui->ckSuppressBlank->isChecked();
    param.suppressNst        = ui->ckSuppressNst->isChecked();
    param.suppressRegex      = ui->editSuppressRegex->text();
    param.temperature        = ui->spinTemperature->value();
    param.temperatureInc     = ui->spinTemperatureInc->value();
    param.maxInitialTs       = ui->spinMaxInitTs->value();
    param.lengthPenalty      = ui->spinLengthPenalty->value();
    param.entropyThold       = ui->spinEntropyThreshold->value();
    param.logprobThold       = ui->spinLogProbThreshold->value();
    param.noSpeechThold      = ui->spinNoSpeechThreshold->value();

    param.vad                      = ui->ckVADEnable->isChecked();
    param.vadModelPath             = ui->editVADModelPath->text();
    param.vadParams.threshold      = ui->spinVADThreshold->value();
    param.vadParams.minSpeechDurMs = ui->editVADMinSpeechDurMs->text().toInt();
    param.vadParams.minSilenceDurMs =
        ui->editVADMinSilenceDurMs->text().toInt();
    param.vadParams.maxSpeechDurS  = ui->spinVADMaxSpeechDurS->value();
    param.vadParams.speechPadMs    = ui->editVADSpeechPadMs->text().toInt();
    param.vadParams.samplesOverlap = ui->spinVADSamplesOverlap->value();

    param.minNewSampleSize   = ui->editMinNewSampleSize->text().toInt();
    param.minAudioBufferSize = ui->editMinAudioBufferSize->text().toInt();
    param.maxAudioBufferSize = ui->editMaxAudioBufferSize->text().toInt();
}