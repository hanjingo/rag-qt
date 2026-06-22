#include "HistorySettingDialog.h"
#include "ui_HistorySettingDialog.h"

#include "StyleMgr.h"

HistorySettingDialog::HistorySettingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HistorySettingDialog)
{
    ui->setupUi(this);

    _retranslate();
    _initUI();
    _initConnections();
}

HistorySettingDialog::~HistorySettingDialog()
{
    delete ui;
}

void HistorySettingDialog::_retranslate()
{
    ui->lblDialogTitle->setText(tr("History Configuration"));
    ui->lblSortBy->setText(tr("Sort By"));
    ui->lblRecordNum->setText(tr("Max Record Count"));
}

void HistorySettingDialog::_initUI()
{
}

void HistorySettingDialog::_initConnections()
{
}

int HistorySettingDialog::MaxRecord() const
{
    return ui->editMaxRecord->text().toInt();
}

HistorySettingDialog::SortBy HistorySettingDialog::SortByType() const
{
    int sortByIndex = ui->comboSortBy->currentIndex();
    switch(sortByIndex)
    {
        case 0:
            return SortBy::TimeAsc;
        case 1:
            return SortBy::TimeDesc;
        default:
            return SortBy::TimeAsc;
    }
}