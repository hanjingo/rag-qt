#include "Zipper.h"
#include <QDebug>

Zipper::Zipper(const QString &zipFilePath, QObject *parent)
    : QObject(parent)
    , _reader(zipFilePath)
{
}

Zipper::~Zipper()
{
    _reader.close();
}

bool Zipper::UnZip(const QString &destDir)
{
    if(!_reader.isReadable())
    {
        emit SignalUnZipFinished(false, destDir);
        return false;
    }

    QDir dir(destDir);
    if(!dir.exists())
        dir.mkpath(destDir);

    if(!_reader.extractAll(destDir))
    {
        emit SignalUnZipFinished(false, destDir);
        return false;
    }

    emit SignalUnZipFinished(true, destDir);
    return true;
}