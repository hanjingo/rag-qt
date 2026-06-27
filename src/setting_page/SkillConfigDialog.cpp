#include "SkillConfigDialog.h"
#include "ui_SkillConfigDialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include "Global.h"

SkillConfigDialog::SkillConfigDialog(const QString &filepath,
                                     QWidget       *parent)
    : QDialog(parent)
    , ui(new Ui::SkillConfigDialog)
    , m_filepath(filepath)
{
    ui->setupUi(this);

    _initUI();
    _retranslate();
    _initConnections();
}

SkillConfigDialog::~SkillConfigDialog()
{
    delete ui;
}

void SkillConfigDialog::GetAddr(QString &filepath)
{
    m_filepath = ui->editAddr->text();

    filepath = m_filepath;
}

void SkillConfigDialog::_retranslate()
{
}

void SkillConfigDialog::_initUI()
{
    ui->editAddr->setText(m_filepath);
}

void SkillConfigDialog::_initConnections()
{
    connect(ui->btnSkillAddr,
            &QPushButton::clicked,
            this,
            &SkillConfigDialog::_slotBtnSkillAddrClicked);
}

void SkillConfigDialog::_slotBtnSkillAddrClicked()
{
    qDebug() << "Plugin Addr button clicked.";
    // choose skill file path
    QString filePath =
        QFileDialog::getOpenFileName(this,
                                     tr("Select Plugin File"),
                                     "",
                                     tr("Skill Files (*.so, *.dll, *.dylib)"));
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