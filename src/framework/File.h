#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class File : public QFile
{
  public:
    File(const QString &filePath)
        : QFile(filePath)
    {
    }

    ~File() override
    {
        if(isOpen())
            close();
    }

  public:
    static bool    isFileExist(const QString &filePath);
    static qint64  fileSize(const QString &filePath);
    static qint64  fileSizeKB(const QString &filePath);
    static QString md5(const QString &filePath);

    static qint64
    findSentenceBoundary(const QString &text, qint64 start, qint64 end);
    static qint64
    findParagraphBoundary(const QString &text, qint64 start, qint64 end);

    static bool writeJsonFile(const QString       &filePath,
                              const QJsonDocument &doc);
    static bool writeJsonFile(const QString     &filePath,
                              const QJsonObject &jsonObj);
    static bool writeJsonFile(const QString    &filePath,
                              const QJsonArray &jsonArr);

    static bool readJsonFile(const QString &filePath, QJsonDocument &doc);
    static bool readJsonFile(const QString &filePath, QJsonObject &jsonObj);
    static bool readJsonFile(const QString &filePath, QJsonArray &jsonArr);
};

#endif // FILE_H