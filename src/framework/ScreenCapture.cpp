#include "screencapture.h"
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QPainter>
#include <QClipboard>
#include <QKeyEvent>
#include <QTimer>
#include <QDebug>

ScreenCapture::ScreenCapture(QWidget *parent)
    : QWidget(nullptr) // not a child of parent to cover the whole screen
    , m_parentWidget(parent)
    , isDrawing(false)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint
                   | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_DeleteOnClose); // auto release object when close

    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    QScreen *screen = QGuiApplication::primaryScreen();
    if(screen)
        this->setGeometry(screen->geometry());
}

ScreenCapture::~ScreenCapture()
{
    if(m_parentWidget.isNull())
        return;

    m_parentWidget->showNormal();
    m_parentWidget->raise();
    m_parentWidget->activateWindow();
}

void ScreenCapture::start(int delayMs)
{
    if(!m_parentWidget.isNull())
        m_parentWidget->hide();

    QTimer::singleShot(delayMs, this, [this]() {
        QScreen *screen = QGuiApplication::primaryScreen();
        if(screen)
        {
            this->fullScreenPixmap =
                screen->grabWindow(0,
                                   0,
                                   0,
                                   screen->geometry().width(),
                                   screen->geometry().height());
        }

        this->show();
        this->raise();
        this->activateWindow();
    });
}

void ScreenCapture::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        originPos  = event->pos();
        currentPos = originPos;
        isDrawing  = true;
        update();
    }
}

void ScreenCapture::mouseMoveEvent(QMouseEvent *event)
{
    if(isDrawing)
    {
        currentPos = event->pos();
        update();
    }
}

void ScreenCapture::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && isDrawing)
    {
        isDrawing  = false;
        QRect rect = QRect(originPos, currentPos).normalized();
        if(rect.width() > 5 && rect.height() > 5)
        {
            qreal ratio = this->devicePixelRatioF();
            QRect targetRect(rect.x() * ratio,
                             rect.y() * ratio,
                             rect.width() * ratio,
                             rect.height() * ratio);

            QPixmap croppedPixmap = fullScreenPixmap.copy(targetRect);
            QApplication::clipboard()->setPixmap(croppedPixmap);

            emit SignalImageCaptured(croppedPixmap);
        }
        this->close();
    }
}

void ScreenCapture::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.drawPixmap(rect(), fullScreenPixmap);
    painter.fillRect(rect(), QColor(0, 0, 0, 120));

    if(isDrawing)
    {
        QRect selectionRect = QRect(originPos, currentPos).normalized();

        qreal ratio = this->devicePixelRatioF();
        painter.drawPixmap(selectionRect,
                           fullScreenPixmap,
                           QRect(selectionRect.x() * ratio,
                                 selectionRect.y() * ratio,
                                 selectionRect.width() * ratio,
                                 selectionRect.height() * ratio));

        painter.setPen(QPen(QColor(0, 120, 215), 2, Qt::SolidLine));
        painter.drawRect(selectionRect);

        QString sizeStr = QString("%1 x %2")
                              .arg(selectionRect.width())
                              .arg(selectionRect.height());
        painter.setPen(Qt::white);
        painter.drawText(selectionRect.x() + 5, selectionRect.y() - 5, sizeStr);
    }
}

void ScreenCapture::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        this->close();
    }
}
