#ifndef NEWSESSIONDIALOG_H
#define NEWSESSIONDIALOG_H

#include <QDialog>
#include <QVector>

#include "GrpcClient.h"

namespace Ui
{
class NewSessionDialog;
}

class NewSessionDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit NewSessionDialog(const QVector<QString> &models,
                              QWidget                *parent = nullptr);
    ~NewSessionDialog();

    void GetConfig(Bus::Session &sess,
                   QString      &model,
                   bool         &isLocal,
                   bool         &isRemote);

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::NewSessionDialog *ui;
    QVector<QString>      m_models;
};

#endif // NEWSESSIONDIALOG_H
