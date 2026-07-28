#include "PluginConfigDialog.h"
#include "ui_PluginConfigDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVersionNumber>

#include <libqt/crypto/sha.h>
#include <libqt/encoding/zipper.h>

#include "Bus.h"
#include "StyleMgr.h"
#include "PluginMgr.h"
#include "Global.h"

PluginConfigDialog::PluginConfigDialog(const Bus::Plugin &conf, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PluginConfigDialog)
    , m_conf(conf)
{
    ui->setupUi(this);

    _initConnections();
    _initUI();
    _retranslate();
}

PluginConfigDialog::~PluginConfigDialog()
{
    delete ui;
}

Bus::Plugin PluginConfigDialog::GetConfig()
{
    m_conf.filePath  = ui->editDllPath->text();
    m_conf.name      = ui->editName->text();
    m_conf.version   = QString("%1.%2.%3")
                           .arg(ui->editVersionMajor->text())
                           .arg(ui->editVersionMinor->text())
                           .arg(ui->editVersionPatch->text());
    m_conf.publisher = ui->editPublisher->text();
    m_conf.platform  = ui->comboPlatform->currentIndex();
    m_conf.hash      = ui->editHash->text();
    m_conf.desc      = ui->editDesc->text();
    return m_conf;
}

void PluginConfigDialog::_retranslate()
{
}

void PluginConfigDialog::_initUI()
{
    //ui->editPackAddr->setStyleSheet(StyleMgr::ParseFile(":/styles/line_edit"));
    ui->editPackAddr->setPlaceholderText(tr("Packed File Path"));

    ui->editDllPath->setText(m_conf.filePath);
    ui->editName->setText(m_conf.name);
    QVersionNumber version = QVersionNumber::fromString(m_conf.version);
    ui->editVersionMajor->setText(QString::number(version.majorVersion()));
    ui->editVersionMinor->setText(QString::number(version.minorVersion()));
    ui->editVersionPatch->setText(QString::number(version.microVersion()));

    ui->editPublisher->setText(m_conf.publisher);
    ui->comboPlatform->setCurrentIndex(m_conf.platform);
    ui->editHash->setText(m_conf.hash);
    ui->editDesc->setText(m_conf.desc);
}

void PluginConfigDialog::_initConnections()
{
    connect(ui->btnPackAddr,
            &QPushButton::clicked,
            this,
            &PluginConfigDialog::_slotBtnPackAddrClicked);

    connect(ui->btnDllPath,
            &QPushButton::clicked,
            this,
            &PluginConfigDialog::_slotBtnDllAddrClicked);

    connect(ui->btnIconPath,
            &QPushButton::clicked,
            this,
            &PluginConfigDialog::_slotBtnIconAddrClicked);

    connect(ui->btnUnPack,
            &QPushButton::clicked,
            this,
            &PluginConfigDialog::_slotBtnUnPackClicked);
}

void PluginConfigDialog::_slotBtnUnPackClicked()
{
    qDebug() << "Extract Pack button clicked.";
    // check if the plugin file exists
    auto      packPath = ui->editPackAddr->text();
    QFileInfo pack(packPath);
    if(!pack.exists() || !pack.isFile())
    {
        QMessageBox::warning(this,
                             tr("File Not Found"),
                             tr("The selected plugin package does not exist."));
        return;
    }

    QString destDir = Config::getPluginFilePath();
    Zipper  zipper;
    if(!zipper.unZip(destDir, packPath))
    {
        QMessageBox::critical(this,
                              tr("Extract Failed"),
                              tr("Failed to extract plugin package."));
        return;
    }

    QString name, version, icon, publisher, desc;
    int     platform = -1;
    auto    result   = PluginMgr::Instance()->Search(
        destDir,
        [&](const QJsonObject &metaData) -> bool {
            if(metaData.contains("Name") && metaData.contains("Version")
               && metaData.contains("Icon") && metaData.contains("Author")
               && metaData.contains("Description")
               && metaData.contains("Platform"))
            {
                name      = metaData.value("Name").toString();
                version   = metaData.value("Version").toString();
                icon      = metaData.value("Icon").toString();
                publisher = metaData.value("Author").toString();
                desc      = metaData.value("Description").toString();
                platform  = metaData.value("Platform").toInt();
                return true;
            }
            return false;
        });
    if(result.isEmpty())
    {
        QMessageBox::critical(
            this,
            tr("Extract Failed"),
            tr("Failed to find plugin metadata after extraction."));
        return;
    } else
    {
        auto dll = result.first();
        ui->editDllPath->setText(dll);
        ui->editIconPath->setText(QFileInfo(dll).absolutePath() + "/" + icon);
        ui->editName->setText(name);
        QVersionNumber ver = QVersionNumber::fromString(version);
        ui->editVersionMajor->setText(QString::number(ver.majorVersion()));
        ui->editVersionMinor->setText(QString::number(ver.minorVersion()));
        ui->editVersionPatch->setText(QString::number(ver.microVersion()));
        ui->editPublisher->setText(publisher);
        ui->comboPlatform->setCurrentIndex(platform);
        ui->editDesc->setText(desc);
    }
}

void PluginConfigDialog::_slotBtnPackAddrClicked()
{
    qDebug() << "Pack Addr button clicked.";
    // choose plugin file path
    QString exts = "*.zip";
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

void PluginConfigDialog::_slotBtnDllAddrClicked()
{
    qDebug() << "DLL Addr button clicked.";
    // choose DLL file path
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
                                     tr("Select DLL File"),
                                     "",
                                     tr("DLL Files (%1)").arg(exts));
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

    ui->editDllPath->setText(filePath);
}

void PluginConfigDialog::_slotBtnIconAddrClicked()
{
    qDebug() << "Icon Addr button clicked.";
    // choose icon file path
    QString exts = "*.png *.jpg *.jpeg *.bmp *.ico";
    QString filePath =
        QFileDialog::getOpenFileName(this,
                                     tr("Select Icon File"),
                                     "",
                                     tr("Image Files (%1)").arg(exts));
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

    ui->editIconPath->setText(filePath);
}