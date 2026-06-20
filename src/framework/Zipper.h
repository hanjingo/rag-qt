#ifndef ZIPPER_H
#define ZIPPER_H

#include <QtGlobal>
#include <QtCore/private/qzipreader_p.h>

#include <QDir>
#include <QDebug>

class Zipper : public QObject
{
    Q_OBJECT
  public:
    explicit Zipper(const QString &zipFilePath, QObject *parent = nullptr);
    ~Zipper() override;

    bool UnZip(const QString &destDir);

  signals:
    void SignalUnZipFinished(bool success, const QString &destDir);

  private:
    QZipReader _reader;
};

#endif // ZIPPER_H