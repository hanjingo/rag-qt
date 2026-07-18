#include "PluginConfigDialog.h"
#include "ui_PluginConfigDialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include "Global.h"

PluginConfigDialog::PluginConfigDialog(const QString &filepath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PluginConfigDialog)
    , m_filepath(filepath)
{
    ui->setupUi(this);

    _initUI();
    _retranslate();
    _initConnections();
}

PluginConfigDialog::~PluginConfigDialog()
{
    delete ui;
}

void PluginConfigDialog::GetAddr(QString &filepath)
{
    m_filepath = ui->editAddr->text();

    filepath = m_filepath;
}

void PluginConfigDialog::_retranslate()
{
}

void PluginConfigDialog::_initUI()
{
    ui->editAddr->setText(m_filepath);
}

void PluginConfigDialog::_initConnections()
{
    connect(ui->btnPluginAddr,
            &QPushButton::clicked,
            this,
            &PluginConfigDialog::_slotBtnPluginAddrClicked);
}

void PluginConfigDialog::_slotBtnPluginAddrClicked()
{
    qDebug() << "Plugin Addr button clicked.";
    // choose plugin file path
    QString filePath =
        QFileDialog::getOpenFileName(this,
                                     tr("Select Plugin File"),
                                     "",
                                     tr("Plugin Files (*.so, *.dll, *.dylib)"));
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