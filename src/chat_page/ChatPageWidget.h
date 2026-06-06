#ifndef CHATPAGEWIDGET_H
#define CHATPAGEWIDGET_H

#include <QMap>
#include <QWidget>

namespace Ui
{
class ChatPageWidget;
}

class ChatPageWidget : public QWidget
{
    Q_OBJECT

  public:
    static ChatPageWidget *GetChatPageWidgetInst();
    explicit ChatPageWidget(QWidget *parent = nullptr);
    ~ChatPageWidget();

  signals:

  private slots:

  private:
    Ui::ChatPageWidget    *ui;
    static ChatPageWidget *m_stChatPageWidgetInst;
};

#endif // CHATPAGEWIDGET_H
