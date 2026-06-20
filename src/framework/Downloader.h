#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>

class Downloader : public QObject
{
    Q_OBJECT
  public:
    explicit Downloader(QObject *parent = nullptr);
    ~Downloader() override;

    void Download(const QUrl &url, const QString &saveFilePath);

  signals:
    void SignalDownloadProgress(int progress);
    void SignalDownloadFinished(bool success, const QString &filePath);

  private slots:
    void _slotReadyRead();
    void _slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void _slotFinished();

  private:
    QNetworkAccessManager *_mgr;
    QNetworkReply         *_reply;
    QFile                 *_file;
};

#endif // DOWNLOADER_H