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
    explicit NewSessionDialog(const QVector<::GrpcLibrary::Skill> &skills,
                              QWidget *parent = nullptr);
    ~NewSessionDialog();

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::NewSessionDialog         *ui;
    QVector<::GrpcLibrary::Skill> m_skills;
};

#endif // NEWSESSIONDIALOG_H
