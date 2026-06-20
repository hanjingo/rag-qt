#include <QApplication>

#include "LoginWidget.h"
#include "FrameworkWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // login first
    FrameworkWidget::Instance()->hide();
    LoginWidget::Instance()->show();

    return a.exec();
}
