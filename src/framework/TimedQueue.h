#ifndef TIMED_QUEUE_H
#define TIMED_QUEUE_H

#include <QObject>
#include <QTimer>
#include <QQueue>
#include <functional>

class TimedQueue : public QObject
{
    Q_OBJECT
  public:
    explicit TimedQueue(int      intervalMs,
                        int      maxQueueSize = 1024,
                        QObject *parent       = nullptr);

    static TimedQueue &Instance()
    {
        static TimedQueue instance(100, 1024);
        return instance;
    }

    void enqueue(std::function<void()> task);
    void setInterval(int ms);
    void start(int ms);
    void stop();
    void clear();
    int  queueSize() const { return m_queue.size(); }

  signals:
    void signalQueueOverflow(int droppedCount);

  private slots:
    void consume();

  private:
    QQueue<std::function<void()>> m_queue;
    QTimer                        m_timer;
    int                           m_maxQueueSize;
    int                           m_interval;
};

#endif // TIMED_QUEUE_H