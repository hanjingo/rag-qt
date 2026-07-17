#include "File.h"

#include <QCryptographicHash>
#include <QDirIterator>

bool File::isFile(const QString &filePath)
{
    QFileInfo info(filePath);
    return info.isFile();
}

bool File::isFileExist(const QString &filePath)
{
    QFile file(filePath);
    return file.exists();
}

qint64 File::fileSize(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file: " << filePath;
        return -1;
    }

    return file.size();
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

void File::walk(const QString                        &filePath,
                std::function<bool(QFileInfo &, int)> cb,
                bool                                  recursive)
{
    if(!cb)
        return;

    QFileInfo fileInfo(filePath);
    if(!fileInfo.exists())
        return;

    if(fileInfo.isFile())
    {
        cb(fileInfo, 0);
        return;
    }

    if(fileInfo.isDir())
    {
        QStringList                 nameFilters;
        QDirIterator::IteratorFlags flags = recursive
                                                ? QDirIterator::Subdirectories
                                                : QDirIterator::NoIteratorFlags;

        QDirIterator it(filePath,
                        nameFilters,
                        QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot
                            | QDir::NoSymLinks | QDir::Readable,
                        flags);

        int baseDepth = filePath.count('/');
        while(it.hasNext())
        {
            it.next();
            QFileInfo currentInfo = it.fileInfo();
            int       depth       = 0;
            if(recursive)
                depth = currentInfo.absoluteFilePath().count('/') - baseDepth;

            if(!cb(currentInfo, depth))
                break;
        }
    }
}

qint64 File::findSentenceBoundary(const QString &text, qint64 start, qint64 end)
{
    QVector<QChar> sentenceEnds = {'.', '!', '?', ';'};
    for(int i = end - 1; i > start; --i)
    {
        QChar ch = text[i];
        if(!sentenceEnds.contains(ch))
            continue;

        if(i + 1 < text.length())
        {
            if(text[i + 1].isSpace())
                return i + 2;

            if(text[i + 1] == '"' || text[i + 1] == '\'' || text[i + 1] == ')'
               || text[i + 1] == ']')
            {
                if(i + 2 < text.length() && text[i + 2].isSpace())
                    return i + 3;
            }
        }

        if(i + 1 == text.length())
            return i + 1;
    }
    return end;
}

qint64
File::findParagraphBoundary(const QString &text, qint64 start, qint64 end)
{
    for(int i = end - 1; i > start; --i)
    {
        if(i + 3 < text.length() && text[i] == '\r' && text[i + 1] == '\n'
           && text[i + 2] == '\r' && text[i + 3] == '\n')
            return i + 4;

        if(i + 1 < text.length() && text[i] == '\n' && text[i + 1] == '\n')
            return i + 2;

        if(i + 2 < text.length() && text[i] == '\n' && text[i + 1] == '\r'
           && text[i + 2] == '\n')
            return i + 3;
    }
    return end;
}

bool File::writeJsonFile(const QString &filePath, const QJsonDocument &doc)
{
    QFile saveFile(filePath);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    saveFile.write(doc.toJson());
    return true;
}

bool File::writeJsonFile(const QString &filePath, const QJsonObject &jsonObj)
{
    QJsonDocument doc(jsonObj);
    QFile         saveFile(filePath);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    saveFile.write(doc.toJson());
    return true;
}

bool File::writeJsonFile(const QString &filePath, const QJsonArray &jsonArr)
{
    QJsonDocument doc(jsonArr);
    QFile         saveFile(filePath);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    saveFile.write(doc.toJson());
    return true;
}

bool File::readJsonFile(const QString &filePath, QJsonDocument &doc)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray data = file.readAll();
    doc             = QJsonDocument::fromJson(data);
    return !doc.isNull();
}

bool File::readJsonFile(const QString &filePath, QJsonObject &jsonObj)
{
    QJsonDocument doc;
    if(!readJsonFile(filePath, doc))
        return false;

    if(!doc.isObject())
        return false;

    jsonObj = doc.object();
    return true;
}

bool File::readJsonFile(const QString &filePath, QJsonArray &jsonArr)
{
    QJsonDocument doc;
    if(!readJsonFile(filePath, doc))
        return false;

    if(!doc.isArray())
        return false;

    jsonArr = doc.array();
    return true;
}