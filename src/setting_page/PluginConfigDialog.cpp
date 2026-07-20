#include "PluginConfigDialog.h"
#include "ui_PluginConfigDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVersionNumber>

#include "Bus.h"
#include "StyleMgr.h"

PluginConfigDialog::PluginConfigDialog(const Bus::Plugin &conf, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PluginConfigDialog)
    , m_conf(conf)
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

QString PluginConfigDialog::GetPackAddr()
{
    return ui->editPackAddr->text();
}

Bus::Plugin PluginConfigDialog::GetConfig()
{
    m_conf.name      = ui->editName->text();
    m_conf.publisher = ui->editPublisher->text();
    m_conf.version   = QString("%1.%2.%3")
                           .arg(ui->editVersionMajor->text())
                           .arg(ui->editVersionMinor->text())
                           .arg(ui->editVersionPatch->text());
    m_conf.platform  = ui->comboPlatform->currentIndex();
    m_conf.desc      = ui->editDesc->text();
    return m_conf;
}

void PluginConfigDialog::SetAddrEditable(bool editable)
{
    ui->editPackAddr->setEnabled(editable);
}

void PluginConfigDialog::SetAddrBtnEnable(bool enable)
{
    ui->btnPackAddr->setEnabled(enable);
}

void PluginConfigDialog::_retranslate()
{
}

void PluginConfigDialog::_initUI()
{
    //ui->editPackAddr->setStyleSheet(StyleMgr::ParseFile(":/styles/line_edit"));
    ui->editPackAddr->setPlaceholderText(tr("Plugin Addr"));

    ui->editName->setText(m_conf.name);
    ui->editPublisher->setText(m_conf.publisher);
    QVersionNumber version = QVersionNumber::fromString(m_conf.version);
    ui->editVersionMajor->setText(QString::number(version.majorVersion()));
    ui->editVersionMinor->setText(QString::number(version.minorVersion()));
    ui->editVersionPatch->setText(QString::number(version.microVersion()));

    ui->comboPlatform->setCurrentIndex(m_conf.platform);
    ui->editDesc->setText(m_conf.desc);
}

void PluginConfigDialog::_initConnections()
{
    connect(ui->btnPackAddr,
            &QPushButton::clicked,
            this,
            &PluginConfigDialog::_slotBtnPluginAddrClicked);
}

void PluginConfigDialog::_slotBtnPluginAddrClicked()
{
    qDebug() << "Plugin Addr button clicked.";
    // choose plugin file path
    QString exts;
#ifdef Q_OS_WIN
    exts = "*.dll";
#elif defined(Q_OS_LINUX)
    exts = "*.so";
#elif defined(Q_OS_MAC)
    exts = "*.dylib";
#endif

    QString filePath =
        QFileDialog::getOpenFileName(this,
                                     tr("Select Plugin File"),
                                     "",
                                     tr("Plugin Files (%1)").arg(exts));
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

    ui->editPackAddr->setText(filePath);
}

void PluginConfigDialog::SetAllEnabled(const bool enabled)
{
    ui->editPackAddr->setEnabled(enabled);
    ui->btnPackAddr->setEnabled(enabled);

    ui->ckBuildPack->setEnabled(enabled);

    ui->editDllPath->setEnabled(enabled);
    ui->btnDllPath->setEnabled(enabled);

    ui->editIconPath->setEnabled(enabled);
    ui->btnIconPath->setEnabled(enabled);

    ui->editId->setEnabled(enabled);
    ui->editName->setEnabled(enabled);
    ui->editPublisher->setEnabled(enabled);
    ui->editVersionMajor->setEnabled(enabled);
    ui->editVersionMinor->setEnabled(enabled);
    ui->editVersionPatch->setEnabled(enabled);
    ui->comboPlatform->setEnabled(enabled);

    ui->editDesc->setEnabled(enabled);
    ui->btnUpload->setEnabled(enabled);
}