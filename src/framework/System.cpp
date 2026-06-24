#include "System.h"

#include <QLocale>

QString LocalLang()
{
    return QLocale::system().name();
}