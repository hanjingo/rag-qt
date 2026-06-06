#ifndef TEXTPAGEWIDGET_H
#define TEXTPAGEWIDGET_H

#include <QMap>
#include <QWidget>

namespace Ui
{
class TextPageWidget;
}

class TextPageWidget : public QWidget
{
    Q_OBJECT

  public:
    static TextPageWidget *GetTextPageWidgetInst();
    explicit TextPageWidget(QWidget *parent = nullptr);
    ~TextPageWidget();

  signals:

  private slots:

  private:
    Ui::TextPageWidget    *ui;
    static TextPageWidget *m_stTextPageWidgetInst;
};

#endif // TEXTPAGEWIDGET_H
