#include "StyleMgr.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>

StyleMgr::StyleMgr(QObject *parent)
    : QObject(parent)
{
}

StyleMgr::~StyleMgr()
{
}

QString StyleMgr::ParseFile(const QString &styleFile)
{
    QFile file(styleFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open style file:" << styleFile;
        return "";
    }

    QTextStream in(&file);
    QString     styleContent = in.readAll();
    file.close();
    return styleContent;
}