#ifndef SKILLBTN_H
#define SKILLBTN_H

#include <QToolButton>
#include <QPixmap>
#include <QString>
#include <QDateTime>

class SkillBtn : public QToolButton
{
    Q_OBJECT
  public:
    explicit SkillBtn(QWidget *parent = nullptr);
    ~SkillBtn();

    int64_t Id() const { return m_id; }
    void    SetId(int64_t id)
    {
        m_id = id;
        _refreshText();
    }

    QString Name() const { return m_name; }
    void    SetName(const QString &name)
    {
        m_name = name;
        _refreshText();
    }

    QString Desc() const { return m_desc; }
    void    SetDesc(const QString &desc)
    {
        m_desc = desc;
        _refreshText();
    }

    QString Publisher() const { return m_publisher; }
    void    SetPublisher(const QString &publisher)
    {
        m_publisher = publisher;
        _refreshText();
    }

    QString Version() const { return m_version; }
    void    SetVersion(const QString &version)
    {
        m_version = version;
        _refreshText();
    }

    QDateTime Timestamp() const { return m_timestamp; }
    void      SetTimestamp(const QString &timestamp)
    {
        m_timestamp = QDateTime::fromString(timestamp, Qt::ISODate);
        _refreshText();
    }

    void    SetHash(const QString &hash) { m_hash = hash; }
    QString Hash() const { return m_hash; }

    int  DownloadTimes() const { return m_downloadTimes; }
    void SetDownloadTimes(int downloadTimes)
    {
        m_downloadTimes = downloadTimes;
        _refreshText();
    }

    void Resize(int w, int h);

  protected:
    void paintEvent(QPaintEvent *e);

  signals:
    void SignalUpdateProgress(int progress);

  protected slots:
    void SlotUpdateProgress(int progress);

  private:
    void _init();
    void _refreshText();

  private:
    int64_t   m_id;
    QString   m_name;
    QString   m_desc;
    QString   m_publisher;
    QString   m_version;
    QDateTime m_timestamp;
    QString   m_hash;
    int       m_downloadTimes;

    int m_progress; // -1: not downloading, 0~100: downloading progress
};

#endif // SKILLBTN_H
