#include <QApplication>
#include <QThread>

#include "ProcManager.h"
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
    ProcManager::Instance()->init();
    for(int i = 0; i <= 100 && splash.getProgress() < 100; i++)
    {
        splash.setProgress(i);
        output = ProcManager::Instance()->readAllStandardOutput();
        splash.appendLog(output);
        if(output.contains("init llm model finish"))
        {
            splash.setProgress(100);
            break;
        }
        a.processEvents();
        QThread::msleep(300);
    }
    splash.close();

    // start login
    FrameworkWidget::Instance()->hide();
    LoginWidget::Instance()->show();

    return a.exec();
}
