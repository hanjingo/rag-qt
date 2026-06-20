#include "Downloader.h"
#include <QDebug>
#include <QDir>
#include <QFile>

Downloader::Downloader(QObject *parent)
    : QObject(parent)
    , _mgr(new QNetworkAccessManager(this))
    , _reply(nullptr)
    , _file(nullptr)
{
}

Downloader::~Downloader()
{
    _mgr->deleteLater();
}

void Downloader::Download(const QUrl &url, const QString &saveFilePath)
{
    QDir dir = QFileInfo(saveFilePath).absoluteDir();
    if(!dir.exists())
    {
        qDebug() << "Directory does not exist, creating: "
                 << dir.absolutePath();
        if(!dir.mkpath("."))
        {
            qDebug() << "Failed to create directory: " << dir.absolutePath();
            return;
        }
    }

    _file = new QFile(saveFilePath);
    if(!_file->open(QIODevice::WriteOnly))
    {
        qDebug() << "Unable to open file:" << saveFilePath << " for writing";
        return;
    }

    QNetworkRequest request(url);
    _reply = _mgr->get(request);

    connect(_reply,
            &QNetworkReply::readyRead,
            this,
            &Downloader::_slotReadyRead);
    connect(_reply,
            &QNetworkReply::downloadProgress,
            this,
            &Downloader::_slotDownloadProgress);
    connect(_reply, &QNetworkReply::finished, this, &Downloader::_slotFinished);
}

void Downloader::_slotReadyRead()
{
    if(_file)
    {
        _file->write(_reply->readAll());
    }
}

void Downloader::_slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if(bytesTotal > 0)
    {
        double progress = (double) bytesReceived / bytesTotal * 100.0;
        emit   SignalDownloadProgress(static_cast<int>(progress));
    }
}

void Downloader::_slotFinished()
{
    QString fname = _file ? _file->fileName() : QString();
    _file->flush();
    _file->close();
    _file->deleteLater();
    _file = nullptr;

    disconnect(_reply,
               &QNetworkReply::readyRead,
               this,
               &Downloader::_slotReadyRead);
    disconnect(_reply,
               &QNetworkReply::downloadProgress,
               this,
               &Downloader::_slotDownloadProgress);
    disconnect(_reply,
               &QNetworkReply::finished,
               this,
               &Downloader::_slotFinished);
    _reply->deleteLater();

    if(_reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "Download finished successfully.";
        emit SignalDownloadFinished(true, fname);
    } else
    {
        qDebug() << "Download error:" << _reply->errorString();
        emit SignalDownloadFinished(false, fname);
    }
}
