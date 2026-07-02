#include "TimedQueue.h"

TimedQueue::TimedQueue(int intervalMs, int maxQueueSize, QObject *parent)
    : QObject(parent)
    , m_maxQueueSize(maxQueueSize)
    , m_interval(intervalMs)
{
    m_timer.setInterval(m_interval);
    connect(&m_timer, &QTimer::timeout, this, &TimedQueue::consume);
}

void TimedQueue::enqueue(std::function<void()> task)
{
    int dropped = 0;
    while(m_queue.size() >= m_maxQueueSize)
    {
        m_queue.dequeue();
        dropped++;
    }
    if(dropped > 0)
        emit signalQueueOverflow(dropped);

    m_queue.enqueue(task);
}

void TimedQueue::consume()
{
    if(m_queue.isEmpty())
        return;

    auto task = m_queue.dequeue();
    task();
}

void TimedQueue::setInterval(int ms)
{
    m_interval = ms;
    m_timer.setInterval(ms);
}

void TimedQueue::start(int ms)
{
    if(!m_timer.isActive())
    {
        setInterval(ms);
        m_timer.start();
    }
}

void TimedQueue::stop()
{
    m_timer.stop();
}

void TimedQueue::clear()
{
    m_queue.clear();
    m_timer.stop();
}