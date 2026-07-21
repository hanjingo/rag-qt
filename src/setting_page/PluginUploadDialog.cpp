#include "PluginUploadDialog.h"
#include "ui_PluginUploadDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVersionNumber>

#include <hj/algo/uuid.hpp>

#include <libqt/crypto/sha.h>
#include <libqt/encoding/zipper.h>

#include "Bus.h"
#include "StyleMgr.h"
#include "PluginMgr.h"
#include "Global.h"

PluginUploadDialog::PluginUploadDialog(const Bus::Plugin &conf, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PluginUploadDialog)
    , m_conf(conf)
{
    ui->setupUi(this);

    _initConnections();
    _initUI();
    _retranslate();
}

PluginUploadDialog::~PluginUploadDialog()
{
    delete ui;
}

Bus::Plugin PluginUploadDialog::GetConfig()
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

QString PluginUploadDialog::GetPackedFilePath()
{
    return ui->editPackAddr->text();
}

void PluginUploadDialog::_retranslate()
{
}

void PluginUploadDialog::_initUI()
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

void PluginUploadDialog::_initConnections()
{
    connect(ui->btnPackAddr,
            &QPushButton::clicked,
            this,
            &PluginUploadDialog::_slotBtnPackAddrClicked);

    connect(ui->btnPack,
            &QPushButton::clicked,
            this,
            &PluginUploadDialog::_slotBtnPackClicked);

    connect(ui->btnDllPath,
            &QPushButton::clicked,
            this,
            &PluginUploadDialog::_slotBtnDllAddrClicked);

    connect(ui->editDllPath,
            &QLineEdit::textChanged,
            this,
            &PluginUploadDialog::_slotDllPathChanged);

    connect(ui->btnIconPath,
            &QPushButton::clicked,
            this,
            &PluginUploadDialog::_slotBtnIconAddrClicked);
}

void PluginUploadDialog::_slotBtnPackAddrClicked()
{
    qDebug() << "Pack Addr button clicked.";
    // choose plugin file path
    QString exts = "*.zip";
    QString packedFile =
        QFileDialog::getOpenFileName(this,
                                     tr("Select Plugin File"),
                                     "",
                                     tr("Plugin Files (%1)").arg(exts));
    if(packedFile.isEmpty())
        return;

    Bus::Plugin conf;
    if(!_parsePackedFile(conf, packedFile))
    {
        QMessageBox::warning(this,
                             tr("Invalid Plugin Package"),
                             tr("The selected plugin package is invalid."));
        return;
    }

    ui->editPackAddr->setText(packedFile);
    ui->editName->setText(conf.name);
    QVersionNumber ver = QVersionNumber::fromString(conf.version);
    ui->editVersionMajor->setText(QString::number(ver.majorVersion()));
    ui->editVersionMinor->setText(QString::number(ver.minorVersion()));
    ui->editVersionPatch->setText(QString::number(ver.microVersion()));
    ui->editPublisher->setText(conf.publisher);
    ui->comboPlatform->setCurrentIndex(conf.platform);
    ui->editHash->setText("sha256:" + conf.hash);
    ui->editDesc->setText(conf.desc);
}

void PluginUploadDialog::_slotBtnPackClicked()
{
    qDebug() << "Pack button clicked.";
    // check if the plugin file exists
    auto      dllPath = ui->editDllPath->text();
    QFileInfo dll(dllPath);
    if(!dll.exists() || !dll.isFile())
    {
        QMessageBox::warning(this,
                             tr("File Not Found"),
                             tr("The selected DLL file does not exist."));
        return;
    }
    _parseDllFile();

    auto      iconPath = ui->editIconPath->text();
    QFileInfo icon(iconPath);
    if(!icon.exists() || !icon.isFile())
    {
        QMessageBox::warning(this,
                             tr("File Not Found"),
                             tr("The selected icon file does not exist."));
        return;
    }

    auto packPath = ui->editPackAddr->text();
    if(packPath.isEmpty())
    {
        // example: chatbox-windows-v0.0.3.zip
        packPath = QFileInfo(dllPath).absolutePath() + "/" + ui->editName->text()
                   + "-" + ui->comboPlatform->currentText() + "-v"
                   + ui->editVersionMajor->text() + "."
                   + ui->editVersionMinor->text() + "."
                   + ui->editVersionPatch->text() + ".zip";
        qDebug() << "Pack path is empty, using default: " << packPath;
        ui->editPackAddr->setText(packPath);
    }
    Zipper zipper;
    if(!zipper.zip(packPath, {dllPath, iconPath}))
    {
        QMessageBox::critical(this,
                              tr("Pack Failed"),
                              tr("Failed to create plugin package."));
        return;
    }

    auto hash = QString(SHA256::hashFile(packPath).toHex());
    ui->editHash->setText("sha256:" + hash);
}

void PluginUploadDialog::_slotBtnDllAddrClicked()
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

void PluginUploadDialog::_slotBtnIconAddrClicked()
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

void PluginUploadDialog::_slotDllPathChanged(const QString &text)
{
    Q_UNUSED(text);
    _parseDllFile();
}

bool PluginUploadDialog::_parsePackedFile(Bus::Plugin   &conf,
                                          const QString &packedFilePath)
{
    conf.hash = QString(SHA256::hashFile(packedFilePath).toHex());

    // unpack to temp folder
    QString tmpPath = QCoreApplication::applicationDirPath() + PLUGIN_DIR + "/"
                      + QString::number(hj::uuid::gen_u64());
    Zipper  zipper;
    if(!zipper.unZip(tmpPath, packedFilePath))
    {
        QDir(tmpPath).removeRecursively();
        return false;
    }

    // search for plugin metadata in the unpacked folder
    auto result = PluginMgr::Instance()->Search(
        tmpPath,
        [&](const QJsonObject &metaData) -> bool {
            if(metaData.contains("Name") && metaData.contains("Version")
               && metaData.contains("Icon") && metaData.contains("Author")
               && metaData.contains("Description")
               && metaData.contains("Platform"))
            {
                conf.name      = metaData.value("Name").toString();
                conf.version   = metaData.value("Version").toString();
                conf.publisher = metaData.value("Author").toString();
                conf.desc      = metaData.value("Description").toString();
                conf.platform  = metaData.value("Platform").toInt();
                return true;
            }
            return false;
        });

    QDir(tmpPath).removeRecursively();
    return result.isEmpty() ? false : true;
}

void PluginUploadDialog::_parseDllFile()
{
    if(ui->editDllPath->text().isEmpty())
        return;

    auto dllPath = ui->editDllPath->text();
    auto params  = PluginMgr::Parse(dllPath);
    if(params.contains("Icon"))
    {
        auto iconPath =
            QFileInfo(dllPath).absolutePath() + "/" + params.value("Icon");
        if(ui->editIconPath->text().isEmpty())
            ui->editIconPath->setText(iconPath);
    }

    if(params.contains("Name"))
    {
        auto name = params.value("Name");
        if(ui->editName->text().isEmpty())
            ui->editName->setText(name);
    }

    if(params.contains("Version"))
    {
        auto           version = params.value("Version");
        QVersionNumber ver     = QVersionNumber::fromString(version);
        if(ui->editVersionMajor->text().isEmpty())
            ui->editVersionMajor->setText(QString::number(ver.majorVersion()));
        if(ui->editVersionMinor->text().isEmpty())
            ui->editVersionMinor->setText(QString::number(ver.minorVersion()));
        if(ui->editVersionPatch->text().isEmpty())
            ui->editVersionPatch->setText(QString::number(ver.microVersion()));
    }

    if(params.contains("Author"))
    {
        auto publisher = params.value("Author");
        if(ui->editPublisher->text().isEmpty())
            ui->editPublisher->setText(publisher);
    }

    if(params.contains("Platform"))
    {
        auto platform = params.value("Platform").toInt();
        ui->comboPlatform->setCurrentIndex(platform);
    }

    if(params.contains("Description"))
    {
        auto desc = params.value("Description");
        if(ui->editDesc->text().isEmpty())
            ui->editDesc->setText(desc);
    }
}