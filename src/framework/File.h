#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <QDir>
#include <QFile>

class File : public QObject
{
    Q_OBJECT

  public:
    static bool    isFileExist(const QString &filePath);
    static qint64  fileSizeKB(const QString &filePath);
    static QString md5(const QString &filePath);
};

#endif // FILE_H