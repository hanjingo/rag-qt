#ifndef HomePageWidget_H
#define HomePageWidget_H

#include <QMap>
#include <QWidget>

namespace Ui
{
class HomePageWidget;
}

class HomePageWidget : public QWidget
{
    Q_OBJECT

  public:
    static HomePageWidget *GetMainHomePageInst();
    explicit HomePageWidget(QWidget *parent = nullptr);
    ~HomePageWidget();

  signals:

  private slots:

  private:
    Ui::HomePageWidget    *ui;
    static HomePageWidget *m_stMainHomePageInst;
};

#endif // HomePageWidget_H
