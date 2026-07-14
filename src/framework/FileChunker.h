#ifndef FILECHUNKDER_H
#define FILECHUNKDER_H

#include <QString>
#include <QVector>

#include <functional>

class FileChunker
{
  public:
    struct Chunk
    {
        QByteArray data;
        qint64     startPos   = 0;
        qint64     offset     = 0;
        qint64     chunkIndex = 0;
    };

    using ChunkCallback = std::function<bool(Chunk &)>;

  public:
    FileChunker()  = default;
    ~FileChunker() = default;

    qint64 chunkText(const QString       &text,
                     const qint64         chunkSize,
                     const ChunkCallback &cb);

    qint64 chunkFile(const QString       &filePath,
                     const qint64         chunkSize,
                     const ChunkCallback &cb);

    qint64 chunkFiles(const QStringList   &filePaths,
                      const qint64         chunkSize,
                      const ChunkCallback &cb);

  private:
    qint64 m_totalChunks = 0;
};

#endif // FILECHUNKDER_H