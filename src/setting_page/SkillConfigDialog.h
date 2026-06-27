#ifndef SKILLCONFIGDIALOG_H
#define SKILLCONFIGDIALOG_H

#include <QDialog>
#include <QVector>
#include <QButtonGroup>
#include <QString>

#include "ui_SkillConfigDialog.h"

namespace Ui
{
class SkillConfigDialog;
}

class SkillConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit SkillConfigDialog(const QString &filepath,
                               QWidget       *parent = nullptr);
    ~SkillConfigDialog();

    void GetAddr(QString &filepath);

  private slots:
    void _slotBtnSkillAddrClicked();

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::SkillConfigDialog *ui;

    QString m_filepath;
};

#endif // SKILLCONFIGDIALOG_H
