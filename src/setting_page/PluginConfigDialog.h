#ifndef PLUGINCONFIGDIALOG_H
#define PLUGINCONFIGDIALOG_H

#include <QDialog>
#include <QVector>
#include <QButtonGroup>
#include <QString>

#include "ui_PluginConfigDialog.h"

namespace Ui
{
class PluginConfigDialog;
}

class PluginConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit PluginConfigDialog(const QString &filepath,
                                QWidget       *parent = nullptr);
    ~PluginConfigDialog();

    void GetAddr(QString &filepath);
    void SetAddrEditable(bool editable);
    void SetAddrBtnEnable(bool enable);

  private slots:
    void _slotBtnPluginAddrClicked();

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::PluginConfigDialog *ui;

    QString m_filepath;
};

#endif // PLUGINCONFIGDIALOG_H
