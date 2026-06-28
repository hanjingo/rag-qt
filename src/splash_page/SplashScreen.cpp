#include "SplashScreen.h"
#include <QPainter>
#include <QFont>

SplashScreen::SplashScreen()
    : QSplashScreen()
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint);

    _initUI();
}

SplashScreen::~SplashScreen()
{
}

void SplashScreen::setProgress(int value)
{
    if(progressBar)
    {
        progressBar->setValue(value);
    }

    this->update();
    QApplication::processEvents();
}

void SplashScreen::appendLog(const QString &text)
{
    if(logOutput)
    {
        logOutput->append(text);
        QTextCursor cursor = logOutput->textCursor();
        cursor.movePosition(QTextCursor::End);
        logOutput->setTextCursor(cursor);
    }
    QApplication::processEvents();
}

void SplashScreen::_initUI()
{
    QWidget *container = new QWidget(this);
    container->setAttribute(Qt::WA_TranslucentBackground);
    container->setStyleSheet("background: transparent;");

    QVBoxLayout *mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(50, 40, 50, 40);
    mainLayout->setSpacing(15);

    QLabel *title = new QLabel("Rag-Qt");
    title->setStyleSheet("color: white;"
                         "font-size: 48px;"
                         "font-weight: 300;"
                         "background: transparent;"
                         "letter-spacing: 3px;");
    title->setAlignment(Qt::AlignCenter);

    progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(false);
    progressBar->setStyleSheet("QProgressBar {"
                               "    border: none;"
                               "    background: rgba(255, 255, 255, 100);"
                               "    height: 2px;"
                               "    border-radius: 1px;"
                               "}"
                               "QProgressBar::chunk {"
                               "    background: white;"
                               "    border-radius: 1px;"
                               "}");
    progressBar->setFixedHeight(2);

    logOutput = new QTextEdit();
    logOutput->setReadOnly(true);
    logOutput->setStyleSheet("QTextEdit {"
                             "    background: rgba(0, 0, 0, 150);"
                             "    color: #a8d8ea;"
                             "    border: 1px solid rgba(255, 255, 255, 50);"
                             "    border-radius: 4px;"
                             "    font-family: 'Courier New', monospace;"
                             "    font-size: 12px;"
                             "    padding: 8px;"
                             "}"
                             "QTextEdit:focus {"
                             "    border: 1px solid rgba(255, 255, 255, 50);"
                             "}");
    logOutput->setFixedHeight(150);
    logOutput->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    logOutput->append("> Initializing...");

    mainLayout->addWidget(title);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(logOutput);

    container->resize(600, 300);

    QPixmap pixmap(container->size());
    pixmap.fill(Qt::transparent);
    container->render(&pixmap);

    this->setPixmap(pixmap);
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int   x              = (screenGeometry.width() - this->width()) / 2;
    int   y              = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}