#include <QApplication>
#include <QThread>
#include <QStyleFactory>

#include "SplashScreen.h"
#include "LoginWidget.h"
#include "MainPageWidget.h"
#include "Config.h"
#include "System.h"
#include "Crash.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // init crash reporter
    hj::crash_handler::instance()->set_dump_callback(crashCallback);
    QString absPath = QCoreApplication::applicationDirPath() + "/crash";
    hj::crash_handler::instance()->set_local_path(absPath.toStdString());

    // show login page, hide framework page
    MainPageWidget::instance()->hide();
    LoginWidget::instance()->hide();

    // init config
    Config::instance()->init();

    // loading splash screen
    SplashScreen splash;
    splash.show();
    splash.setProgress(0);
    a.processEvents();

    // init proc manager
    QString output;
    if(Config::instance()->isCoreRun())
    {
        MainPageWidget::instance()->InitCore();
        for(int i = 0; i <= 50 && splash.getProgress() < 50; i++)
        {
            splash.setProgress(i);
            output = MainPageWidget::instance()->ReadAllStandardOutput();
            splash.appendLog(output);
            if(output.contains("init core service finish"))
            {
                splash.setProgress(50);
                break;
            }
            a.processEvents();
            QThread::msleep(300);
        }
    } else
    {
        splash.setProgress(50);
    }

    // init network
    MainPageWidget::instance()->InitNetwork();
    for(int i = splash.getProgress(); i <= 100 && splash.getProgress() < 100;
        i++)
    {
        splash.setProgress(i);
        output = MainPageWidget::instance()->ReadAllStandardOutput();
        splash.appendLog(output);
        if(MainPageWidget::instance()->IsConnectedToCoreService())
        {
            splash.setProgress(100);
            break;
        }
        a.processEvents();
        QThread::msleep(100);
    }

    splash.close();

    // start login
    MainPageWidget::instance()->hide();
    LoginWidget::instance()->show();

    return a.exec();
}
