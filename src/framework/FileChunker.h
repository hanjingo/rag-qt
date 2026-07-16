#ifndef FILECHUNKER_H
#define FILECHUNKER_H

#include <QString>
#include <QVector>

#include <functional>

class FileChunker
{
  public:
    struct Chunk
    {
        int64_t    id = 0;
        QByteArray data;
        int64_t    startPos = 0;
        int64_t    offset   = 0;
        QString    filePathName;
        QString    timestamp;
    };

    using ChunkCallback = std::function<bool(Chunk &)>;

  public:
    FileChunker()  = default;
    ~FileChunker() = default;

    qint64 chunkText(const QString       &text,
                     const qint64         chunkSize,
                     const ChunkCallback &cb,
                     const QString       &filePathName = QString());

    qint64 chunkFile(const QString       &filePath,
                     const qint64         chunkSize,
                     const ChunkCallback &cb);

    qint64 chunkFiles(const QStringList   &filePaths,
                      const qint64         chunkSize,
                      const ChunkCallback &cb);

  private:
    qint64 m_totalChunks = 0;
};

#endif // FILECHUNKER_H