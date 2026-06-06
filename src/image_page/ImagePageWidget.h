#ifndef IMAGEPAGEWIDGET_H
#define IMAGEPAGEWIDGET_H

#include <QMap>
#include <QWidget>

namespace Ui
{
class ImagePageWidget;
}

class ImagePageWidget : public QWidget
{
    Q_OBJECT

  public:
    static ImagePageWidget *GetImagePageWidgetInst();
    explicit ImagePageWidget(QWidget *parent = nullptr);
    ~ImagePageWidget();

  signals:

  private slots:

  private:
    Ui::ImagePageWidget    *ui;
    static ImagePageWidget *m_stImagePageWidgetInst;
};

#endif // IMAGEPAGEWIDGET_H
