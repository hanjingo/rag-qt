#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QStandardItemModel>
#include <QListView>

#include "SettingPageHardware.h"
#include "ui_SettingPageHardware.h"

#include "StyleMgr.h"
#include "Config.h"
#include "Global.h"
#include "BusAdapter.h"

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

    ui->listWidgetCatalog->setCurrentRow(0);

    // init audio config
    auto confs = Config::Instance().audioTranslatorParams();
    for(auto conf : confs)
    {
        ui->comboAudioTranslator->addItem(conf.id);
    }
    ui->comboAudioTranslator->setCurrentIndex(0);

    // init bottom button
    ui->btnSave->setStyleSheet(StyleMgr::ParseFile(":/styles/push_button"));
    ui->btnAdd->setStyleSheet(StyleMgr::ParseFile(":/styles/push_button"));
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

    connect(ui->btnAdd,
            &QPushButton::clicked,
            this,
            &SettingPageHardware::_slotBtnAddClicked);
}

void SettingPageHardware::_retranslate()
{
    ui->btnAdd->setText(tr("Add"));
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

    ui->lblBufferControlTitle->setText(tr("Buffer Control"));
    ui->lblMinNewSampleSize->setText(tr("Min New Sample Size"));
    ui->lblMinAudioBufferSize->setText(tr("Min Audio Buffer Size"));
    ui->lblMaxAudioBufferSize->setText(tr("Max Audio Buffer Size"));
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
    auto confs  = Config::Instance().audioTranslatorParams();
    for(auto conf : confs)
    {
        if(id != currId)
            continue;

        ui->editThreadsNum->setText(QString::number(conf.nThreads));
        ui->editMaxTextCtx->setText(QString::number(conf.nMaxTextCtx));
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
        ui->ckSuppressNst->setChecked(conf.suppressBlank);
        ui->editSuppressRegex->setText(conf.suppressRegex);
        ui->editTemperature->setText(QString::number(conf.temperature, 'f', 1));
        ui->editTemperatureInc->setText(
            QString::number(conf.temperatureInc, 'f', 1));
        ui->editMaxInitTs->setText(QString::number(conf.maxInitialTs, 'f', 1));
        ui->editLengthPenalty->setText(
            QString::number(conf.lengthPenalty, 'f', 1));
        ui->editEntropyThreshold->setText(
            QString::number(conf.entropyThold, 'f', 1));
        ui->editLogProbThreshold->setText(
            QString::number(conf.logprobThold, 'f', 1));
        ui->editNoSpeechThreshold->setText(
            QString::number(conf.noSpeechThold, 'f', 1));

        ui->editMinNewSampleSize->setText(
            QString::number(conf.minNewSampleSize));
        ui->editMinAudioBufferSize->setText(
            QString::number(conf.minAudioBufferSize));
        ui->editMaxAudioBufferSize->setText(
            QString::number(conf.maxAudioBufferSize));
    }
}

void SettingPageHardware::_save()
{
    // save audio
    auto audioId = ui->comboAudioTranslator->currentText();
    auto confs   = Config::Instance().audioTranslatorParams();
    for(int i = 0; i < confs.size(); ++i)
    {
        if(confs[i].id != audioId)
            continue;

        confs[i].nThreads           = ui->editThreadsNum->text().toInt();
        confs[i].nMaxTextCtx        = ui->editMaxTextCtx->text().toInt();
        confs[i].translate          = ui->ckTranslate->isChecked();
        confs[i].detectLanguage     = ui->ckDetectLanguage->isChecked();
        confs[i].language           = ui->comboLanguage->currentText();
        confs[i].noCtx              = ui->ckNoCtx->isChecked();
        confs[i].noTimestamps       = ui->ckNoTimestamps->isChecked();
        confs[i].singleSegment      = ui->ckSingleSegment->isChecked();
        confs[i].printSpecial       = ui->ckPrintSpecial->isChecked();
        confs[i].printProgress      = ui->ckPrintProgress->isChecked();
        confs[i].printRealtime      = ui->ckPrintRealTime->isChecked();
        confs[i].printTimestamps    = ui->ckPrintTimeStamp->isChecked();
        confs[i].carryInitialPrompt = ui->ckCarryInitPrompt->isChecked();
        confs[i].initialPrompt      = ui->editInitPrompt->text();
        confs[i].suppressBlank      = ui->ckSuppressBlank->isChecked();
        confs[i].suppressNst        = ui->ckSuppressNst->isChecked();
        confs[i].suppressRegex      = ui->editSuppressRegex->text();
        confs[i].temperature        = ui->editTemperature->text().toFloat();
        confs[i].temperatureInc     = ui->editTemperatureInc->text().toFloat();
        confs[i].maxInitialTs       = ui->editMaxInitTs->text().toFloat();
        confs[i].lengthPenalty      = ui->editLengthPenalty->text().toFloat();
        confs[i].entropyThold  = ui->editEntropyThreshold->text().toFloat();
        confs[i].logprobThold  = ui->editLogProbThreshold->text().toFloat();
        confs[i].noSpeechThold = ui->editNoSpeechThreshold->text().toFloat();

        confs[i].minNewSampleSize = ui->editMinNewSampleSize->text().toInt();
        confs[i].minAudioBufferSize =
            ui->editMinAudioBufferSize->text().toInt();
        confs[i].maxAudioBufferSize =
            ui->editMaxAudioBufferSize->text().toInt();
    }
    Config::Instance().setAudioTranslatorParams(confs);
    Config::Instance().save(Config::getConfigFilePath());

    auto params = Config::Instance().getBusAudioParams();
    emit BusAdapter::Instance() -> SignalAudioParamUpdateNtf(params);
}

void SettingPageHardware::_slotBtnSaveClicked()
{
    qDebug()
        << "Save button clicked. Implement hardware settings save logic here.";
    _save();
}

void SettingPageHardware::_slotBtnAddClicked()
{
    qDebug()
        << "Add button clicked. Implement add audio translator logic here.";
    switch(ui->stackPage->currentIndex())
    {
        case 0: // Audio Translator
        {
            QString newId = QString("audio_translator_%1")
                                .arg(ui->comboAudioTranslator->count() + 1);
            _addAudioConfig(newId);
        }
        break;
        case 1: // GPU
            // Handle GPU add logic here if needed
            break;
        default:
            break;
    }
}

void SettingPageHardware::_addAudioConfig(const QString &id)
{
    auto confs = Config::Instance().audioTranslatorParams();
    for(auto conf : confs)
    {
        if(conf.id == id)
        {
            qDebug() << "Audio translator with id" << id
                     << "already exists. Cannot add duplicate.";
            return;
        }
    }

    Config::TranslatorParam newConf;
    newConf.id = id;
    confs.append(newConf);
    Config::Instance().setAudioTranslatorParams(confs);

    ui->comboAudioTranslator->addItem(id);
    ui->comboAudioTranslator->setCurrentText(id);
}