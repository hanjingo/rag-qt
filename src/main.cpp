#include <QApplication>

#include "LoginWidget.h"
#include "FrameworkWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // login first
    FrameworkWidget::GetFrameworkWidgetInst()->hide();
    LoginWidget::GetLoginWgtInst()->show();

    return a.exec();
}
