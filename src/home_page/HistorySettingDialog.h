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
    enum class SortBy
    {
        TimeAsc = 0,
        TimeDesc
    };

  public:
    explicit HistorySettingDialog(QWidget *parent = nullptr);
    ~HistorySettingDialog();

    int    MaxRecord() const;
    SortBy SortByType() const;
    bool   ShowDesc() const;

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::HistorySettingDialog *ui;
};

#endif // HISTORYSETTINGDIALOG_H
