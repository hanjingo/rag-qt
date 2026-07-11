#include "File.h"

#include <QCryptographicHash>

bool File::isFileExist(const QString &filePath)
{
    QFile file(filePath);
    return file.exists();
}

qint64 File::fileSizeKB(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file: " << filePath;
        return -1;
    }

    return file.size() / 1024;
}

QString File::md5(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file: " << filePath;
        return "";
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    if(!hash.addData(&file))
    {
        qWarning() << "Failed to compute MD5 for file: " << filePath;
        return "";
    }

    return hash.result().toHex();
}