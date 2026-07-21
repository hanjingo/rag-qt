#ifndef PLUGINUPLOADDIALOG_H
#define PLUGINUPLOADDIALOG_H

#include <QDialog>
#include <QVector>
#include <QButtonGroup>
#include <QString>

#include <libqt/net/uploader.h>

#include "ui_PluginUploadDialog.h"

#include "GrpcClient.h"

namespace Ui
{
class PluginUploadDialog;
}

class PluginUploadDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit PluginUploadDialog(const Bus::Plugin &conf,
                                QWidget           *parent = nullptr);
    ~PluginUploadDialog();

    Bus::Plugin GetConfig();
    QString     GetPackedFilePath();

  private slots:
    void _slotBtnPackAddrClicked();
    void _slotBtnPackClicked();
    void _slotBtnDllAddrClicked();
    void _slotBtnIconAddrClicked();
    void _slotDllPathChanged(const QString &text);

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();
    bool _parsePackedFile(Bus::Plugin &conf, const QString &packedFilePath);
    void _parseDllFile();

  private:
    Ui::PluginUploadDialog *ui;

    Bus::Plugin m_conf;
};

#endif // PLUGINUPLOADDIALOG_H
