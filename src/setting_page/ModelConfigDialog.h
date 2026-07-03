#ifndef MODELCONFIGDIALOG_H
#define MODELCONFIGDIALOG_H

#include <QDialog>
#include <QVector>
#include <QButtonGroup>

#include "ui_ModelConfigDialog.h"
#include "Config.h"

namespace Ui
{
class ModelConfigDialog;
}

class ModelConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ModelConfigDialog(const Config::ModelConfig &conf,
                               QWidget                   *parent = nullptr);
    ~ModelConfigDialog();

    Config::ModelConfig GetConfig();

  private slots:
    void _slotBtnModelAddrClicked();

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::ModelConfigDialog *ui;

    QButtonGroup       *m_pPipelineBtnGroup;
    Config::ModelConfig m_conf;
};

#endif // MODELCONFIGDIALOG_H
