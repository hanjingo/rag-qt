#ifndef SKILLBTN_H
#define SKILLBTN_H

#include <QPushButton>
#include <QPixmap>
#include <QString>

class SkillBtn : public QPushButton
{
    Q_OBJECT
  public:
    explicit SkillBtn(QWidget *parent = nullptr);
    ~SkillBtn();

  protected:
    void paintEvent(QPaintEvent *e);

  signals:
    void SignalUpdateProgress(int progress);

  protected slots:
    void SlotUpdateProgress(int progress);

  private:
    void _init();

  private:
};

#endif // SKILLBTN_H
