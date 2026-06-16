#ifndef NEWSESSIONDIALOG_H
#define NEWSESSIONDIALOG_H

#include <QDialog>

namespace Ui
{
class NewSessionDialog;
}

class NewSessionDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit NewSessionDialog(QWidget *parent = nullptr);
    ~NewSessionDialog();

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::NewSessionDialog *ui;
};

#endif // NEWSESSIONDIALOG_H
