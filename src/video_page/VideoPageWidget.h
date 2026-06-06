#ifndef VIDEOPAGEWIDGET_h
#define VIDEOPAGEWIDGET_h

#include <QMap>
#include <QWidget>

namespace Ui
{
class VideoPageWidget;
}

class VideoPageWidget : public QWidget
{
    Q_OBJECT

  public:
    static VideoPageWidget *GetVideoPageWidgetInst();
    explicit VideoPageWidget(QWidget *parent = nullptr);
    ~VideoPageWidget();

  signals:

  private slots:

  private:
    Ui::VideoPageWidget    *ui;
    static VideoPageWidget *m_stVideoPageWidgetInst;
};

#endif // VIDEOPAGEWIDGET_h
