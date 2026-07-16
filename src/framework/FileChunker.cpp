#include "FileChunker.h"

#include <QFile>
#include <QDebug>

qint64 FileChunker::chunkText(const QString       &text,
                              const qint64         chunkSize,
                              const ChunkCallback &cb,
                              const QString       &filePathName)
{
    if(text.isEmpty())
    {
        qWarning() << "Empty text provided";
        return 0;
    }

    m_totalChunks   = 0;
    qint64 totalLen = text.length();
    Chunk  chunk;
    chunk.startPos     = 0;
    chunk.id           = 0;
    chunk.offset       = qMin(chunkSize, totalLen);
    chunk.filePathName = filePathName;
    while(cb(chunk))
    {
        chunk.offset = qMin(chunkSize, totalLen - chunk.startPos);
        if(chunk.offset < 1)
            break;

        if(chunk.startPos >= totalLen)
            break;

        chunk.data = text.mid(chunk.startPos, chunk.offset).toUtf8();
        m_totalChunks++;
    }

    qDebug() << "Created" << m_totalChunks
             << "chunks from text, total length:" << totalLen;
    return m_totalChunks;
}

qint64 FileChunker::chunkFile(const QString       &filePath,
                              const qint64         chunkSize,
                              const ChunkCallback &cb)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file:" << filePath;
        return -1;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Encoding::Utf8);

    QString content = stream.readAll();
    file.close();
    return chunkText(content, chunkSize, cb, filePath);
}

qint64 FileChunker::chunkFiles(const QStringList   &filePaths,
                               const qint64         chunkSize,
                               const ChunkCallback &cb)
{
    qint64 totalChunks = 0;
    for(const QString &filePath : filePaths)
    {
        qint64 chunks = chunkFile(filePath, chunkSize, cb);
        if(chunks > 0)
            totalChunks += chunks;
    }

    qDebug() << "Total chunks from" << filePaths.size()
             << "files:" << totalChunks;
    return totalChunks;
}