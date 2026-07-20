#ifndef PLUGINCONFIGDIALOG_H
#define PLUGINCONFIGDIALOG_H

#include <QDialog>
#include <QVector>
#include <QButtonGroup>
#include <QString>

#include "ui_PluginConfigDialog.h"

#include "GrpcClient.h"

namespace Ui
{
class PluginConfigDialog;
}

class PluginConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit PluginConfigDialog(const Bus::Plugin &conf,
                                QWidget           *parent = nullptr);
    ~PluginConfigDialog();

    QString     GetPackAddr();
    Bus::Plugin GetConfig();
    void        SetAddrEditable(bool editable);
    void        SetAddrBtnEnable(bool enable);
    void        SetAllEnabled(const bool enabled);

  private slots:
    void _slotBtnPluginAddrClicked();

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::PluginConfigDialog *ui;

    Bus::Plugin m_conf;
};

#endif // PLUGINCONFIGDIALOG_H
