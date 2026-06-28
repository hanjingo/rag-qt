#include <QApplication>
#include <QThread>

#include "SplashScreen.h"
#include "LoginWidget.h"
#include "FrameworkWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FrameworkWidget::Instance()->hide();
    LoginWidget::Instance()->hide();

    // loading splash screen
    SplashScreen splash;
    splash.show();
    splash.setProgress(0);
    a.processEvents();

    // init proc manager
    QString output;
    FrameworkWidget::Instance()->InitCore();
    for(int i = 0; i <= 50 && splash.getProgress() < 50; i++)
    {
        splash.setProgress(i);
        output = FrameworkWidget::Instance()->ReadAllStandardOutput();
        splash.appendLog(output);
        if(output.contains("init core service finish"))
        {
            splash.setProgress(50);
            break;
        }
        a.processEvents();
        QThread::msleep(300);
    }

    // init network
    FrameworkWidget::Instance()->InitNetwork();
    for(int i = splash.getProgress(); i <= 100 && splash.getProgress() < 100;
        i++)
    {
        splash.setProgress(i);
        output = FrameworkWidget::Instance()->ReadAllStandardOutput();
        splash.appendLog(output);
        if(FrameworkWidget::Instance()->IsConnectedToCoreService())
        {
            splash.setProgress(100);
            break;
        }
        a.processEvents();
        QThread::msleep(100);
    }

    splash.close();

    // start login
    FrameworkWidget::Instance()->hide();
    LoginWidget::Instance()->show();

    return a.exec();
}
