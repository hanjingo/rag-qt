#ifndef HISTORYSETTINGDIALOG_H
#define HISTORYSETTINGDIALOG_H

#include <QDialog>

#include "GrpcClient.h"

namespace Ui
{
class HistorySettingDialog;
}

class HistorySettingDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit HistorySettingDialog(QWidget *parent = nullptr);
    ~HistorySettingDialog();

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::HistorySettingDialog *ui;
};

#endif // HISTORYSETTINGDIALOG_H
