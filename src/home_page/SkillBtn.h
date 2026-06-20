#ifndef SKILLBTN_H
#define SKILLBTN_H

#include <QToolButton>
#include <QPixmap>
#include <QString>
#include <QDateTime>

#include "Downloader.h"

class SkillBtn : public QToolButton
{
    Q_OBJECT
  public:
    enum class State
    {
        Unknown,
        WaitDownload,
        Downloading,
        Downloaded,
        Installing,
        Installed
    };

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

    int  DownloadProgress() const { return m_progress; }
    bool IsDownloaded() const { return m_progress >= 100; }

    void Resize(int w, int h);

    void  SetState(State state);
    State GetState() const { return m_state; }

    void SetUrl(const QString &url) { m_url = url; }
    QUrl Url() const { return m_url; }

    void Download(const QUrl &url, const QString &savePath);

  signals:
    void SignalStateChanged(SkillBtn *btn, SkillBtn::State state);

  public:
    void SlotProgressChanged(int progress);
    void SlotProgressFinished(bool success);

  protected:
    void paintEvent(QPaintEvent *e);

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

    int         m_progress; // -1: not downloading, 0~100: downloading progress
    State       m_state;
    QString     m_url;
    Downloader *m_downloader = nullptr;
};

#endif // SKILLBTN_H
