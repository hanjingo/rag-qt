#include <QApplication>

#include "FrameworkWidget.h"

int main(int argc, char *argv[])
{
    QApplication     a(argc, argv);
    FrameworkWidget *w = FrameworkWidget::GetFrameworkWidgetInst();
    w->show();
    return a.exec();
}
