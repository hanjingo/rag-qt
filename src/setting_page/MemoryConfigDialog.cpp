#include "MemoryConfigDialog.h"
#include "ui_MemoryConfigDialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include <hj/algo/uuid.hpp>

#include <libqt/io/file.h>

#include "Global.h"

MemoryConfigDialog::MemoryConfigDialog(const Config::MemoryConfig &conf,
                                       QWidget                    *parent)
    : QDialog(parent)
    , ui(new Ui::MemoryConfigDialog)
    , m_conf(conf)
{
    ui->setupUi(this);

    _initUI();
    _retranslate();
    _initConnections();
}

MemoryConfigDialog::~MemoryConfigDialog()
{
}

Config::MemoryConfig MemoryConfigDialog::GetConfig()
{
    m_conf.id             = ui->editId->text();
    m_conf.indexFilePath  = ui->editIndexFilePath->text();
    m_conf.metaFilePath   = ui->editMetaFilePath->text();
    m_conf.originFilePath = ui->editOriginFilePath->text();
    m_conf.dimension      = ui->editDimension->text().toInt();

    m_conf.chunkSize         = ui->editChunkSize->text().toLongLong();
    m_conf.overlap           = ui->editOverlap->text().toInt();
    m_conf.respectSentences  = ui->ckRespectSentences->isChecked();
    m_conf.respectParagraphs = ui->ckRespectParagraphs->isChecked();
    m_conf.encoding          = ui->comboEncoding->currentText();
    return m_conf;
}

void MemoryConfigDialog::_retranslate()
{
}

void MemoryConfigDialog::_initUI()
{
    ui->editId->setText(m_conf.id);
    ui->editIndexFilePath->setText(m_conf.indexFilePath);
    ui->editMetaFilePath->setText(m_conf.metaFilePath);
    ui->editOriginFilePath->setText(m_conf.originFilePath);
    ui->editDimension->setText(QString::number(m_conf.dimension));

    // chunk params
    ui->editChunkSize->setText(QString::number(m_conf.chunkSize));
    ui->editOverlap->setText(QString::number(m_conf.overlap));
    ui->ckRespectSentences->setChecked(m_conf.respectSentences);
    ui->ckRespectParagraphs->setChecked(m_conf.respectParagraphs);
    ui->comboEncoding->setCurrentText(m_conf.encoding);
}

void MemoryConfigDialog::_initConnections()
{
    connect(ui->btnIndexFilePath,
            &QPushButton::clicked,
            this,
            &MemoryConfigDialog::_slotBtnIndexFilePathClicked);
    connect(ui->btnMetaFilePath,
            &QPushButton::clicked,
            this,
            &MemoryConfigDialog::_slotBtnMetaFilePathClicked);
    connect(ui->btnOriginFilePath,
            &QPushButton::clicked,
            this,
            &MemoryConfigDialog::_slotBtnOriginFilePathClicked);

    connect(ui->btnGenerate,
            &QPushButton::clicked,
            this,
            &MemoryConfigDialog::_slotBtnGenerateClicked);
    connect(ui->btnCancel,
            &QPushButton::clicked,
            this,
            &MemoryConfigDialog::_slotBtnCancelClicked);
}

void MemoryConfigDialog::_slotBtnIndexFilePathClicked()
{
    qDebug() << "Index File Path button clicked.";
    // choose index file path
    QString filePath =
        QFileDialog::getOpenFileName(this,
                                     tr("Select Index File"),
                                     "",
                                     tr("Index Files (*.faiss)"));
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

    ui->editIndexFilePath->setText(filePath);
}

void MemoryConfigDialog::_slotBtnMetaFilePathClicked()
{
    qDebug() << "Meta File Path button clicked.";
    // choose meta file path
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Select Meta File"),
                                                    "",
                                                    tr("Meta Files (*.json)"));
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

    ui->editMetaFilePath->setText(filePath);
}

void MemoryConfigDialog::_slotBtnOriginFilePathClicked()
{
    qDebug() << "Origin File Path button clicked.";
    // choose origin file path
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select Directory"),
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dirPath.isEmpty())
        return;

    QFileInfo fileInfo(dirPath);
    if(!fileInfo.exists() || !fileInfo.isDir())
    {
        QMessageBox::warning(this,
                             tr("Directory Not Found"),
                             tr("The selected directory does not exist."));
        return;
    }

    ui->editOriginFilePath->setText(dirPath);
}

void MemoryConfigDialog::_slotBtnGenerateClicked()
{
    qDebug() << "Generate button clicked.";
    auto conf = GetConfig();
    emit signalGenerateMemory(conf);
}

void MemoryConfigDialog::_slotBtnCancelClicked()
{
    qDebug() << "Cancel button clicked.";
}

void MemoryConfigDialog::slotEmbeddingProgressUpdate(
    const Config::MemoryConfig &conf,
    const int64_t               finishedChunkNum,
    const int64_t               totalChunkNum)
{
    if(conf.id != m_conf.id)
        return;

    ui->progressBuildIndex->setRange(0, totalChunkNum);
    ui->progressBuildIndex->setValue(finishedChunkNum);
}