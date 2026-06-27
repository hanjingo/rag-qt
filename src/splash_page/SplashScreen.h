// SplashScreen.h
#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QTextEdit>

class SplashScreen : public QSplashScreen
{
    Q_OBJECT

  public:
    SplashScreen();
    ~SplashScreen();

    int getProgress() const
    {
        if(progressBar)
            return progressBar->value();

        return 0;
    }
    void setProgress(int value);
    void appendLog(const QString &text);

  private:
    void    _initUI();
    QPixmap _applyRoundedCorners(const QPixmap &pixmap, int radius);
    QPixmap _applyShadow(const QPixmap &pixmap);

  private:
    QProgressBar *progressBar = nullptr;
    QTextEdit    *logOutput   = nullptr;
};

#endif // SPLASHSCREEN_H