#include "System.h"

#include <QLocale>

System *System::m_stSystemInst = nullptr;
System *System::Instance()
{
    if(!m_stSystemInst)
        m_stSystemInst = new System();

    return m_stSystemInst;
}

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