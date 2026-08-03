#ifndef PROCMANAGER_H
#define PROCMANAGER_H

#include <QString>
#include <QProcess>
#include <QPointer>

class ProcManager : public QObject
{
    Q_OBJECT

  public:
    explicit ProcManager(QObject *parent = nullptr);
    ~ProcManager();

    static QPointer<ProcManager> instance()
    {
        static QPointer<ProcManager> inst = new ProcManager();
        return inst;
    }
    void    init();
    void    destroy();
    QString readAllStandardOutput();

  signals:
    void signalCoreStarted();
    void signalCoreFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void signalCoreError(QProcess::ProcessError error);

  private:
    void _connectCore();
    void _disconnectCore();

  private slots:
    void _slotCoreStarted();
    void _slotCoreFinished(int, QProcess::ExitStatus);
    void _slotCoreError(QProcess::ProcessError);

  private:
    QProcess *m_pCore = nullptr;
};

#endif // PROCMANAGER_H