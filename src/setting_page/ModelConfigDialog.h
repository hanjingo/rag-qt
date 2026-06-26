#ifndef MODELCONFIGDIALOG_H
#define MODELCONFIGDIALOG_H

#include <QDialog>
#include <QVector>
#include <QButtonGroup>

#include "ui_ModelConfigDialog.h"
#include "Bus.h"

namespace Ui
{
class ModelConfigDialog;
}

class ModelConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ModelConfigDialog(const Bus::ModelConfig &conf,
                               QWidget                *parent = nullptr);
    ~ModelConfigDialog();

    Bus::ModelConfig GetConfig();

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::ModelConfigDialog *ui;

    QButtonGroup    *m_pPipelineBtnGroup;
    Bus::ModelConfig m_conf;
};

#endif // MODELCONFIGDIALOG_H
