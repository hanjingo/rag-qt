#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QPointer>
#include <QApplication>
#include <QClipboard>

class ScreenCapture : public QWidget
{
    Q_OBJECT

  public:
    explicit ScreenCapture(QWidget *parent);
    ~ScreenCapture() override;

    void start(int delayMs = 200);

  signals:
    void SignalImageCaptured(const QPixmap &pixmap);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

  private:
    QPointer<QWidget> m_parentWidget;
    QPixmap           fullScreenPixmap;
    QPoint            originPos;
    QPoint            currentPos;
    bool              isDrawing;
};

#endif // SCREENCAPTURE_H
