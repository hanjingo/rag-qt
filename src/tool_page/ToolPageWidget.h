#ifndef TOOLPAGEWIDGET_H
#define TOOLPAGEWIDGET_H

#include <QMap>
#include <QWidget>

namespace Ui
{
class ToolPageWidget;
}

class ToolPageWidget : public QWidget
{
    Q_OBJECT

  public:
    static ToolPageWidget *GetToolPageWidgetInst();
    explicit ToolPageWidget(QWidget *parent = nullptr);
    ~ToolPageWidget();

  signals:

  private slots:

  private:
    Ui::ToolPageWidget    *ui;
    static ToolPageWidget *m_stToolPageWidgetInst;
};

#endif // TOOLPAGEWIDGET_H
