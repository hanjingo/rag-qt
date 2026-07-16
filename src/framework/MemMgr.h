#ifndef MEMMGR_H
#define MEMMGR_H

#include <QObject>

class MemMgr : public QObject
{
    Q_OBJECT

  public:
    explicit MemMgr(QObject *parent = nullptr) {};
    ~MemMgr() {};

    static MemMgr *Instance()
    {
        static MemMgr instance;
        return &instance;
    }

    // QVector<ChunkFile> retrieve(const QString          &question,
    //                             const int               topK,
    //                             const QVector<QString> &memoryIds);
    // {
    //     // Implementation goes here
    //     // 1. embedding question
    //     // 2. retrieve fro
    //     return {};
    // }
};

#endif // MEMMGR_H