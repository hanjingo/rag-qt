#include "System.h"

#include <QLocale>

System::System(QObject *parent)
    : QObject(parent)
{
}

System::~System()
{
}

QString System::LocalLang()
{
    return QLocale::system().name();
}

QString System::Arch()
{
    auto arch = QSysInfo::currentCpuArchitecture();
    if(arch == "x86")
        return "x86";
    else if(arch == "x86_64")
        return "x64";
    else if(arch == "arm64")
        return "arm64";
    else
        return arch;
}