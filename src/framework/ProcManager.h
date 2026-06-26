#ifndef PROCMANAGER_H
#define PROCMANAGER_H

#include <QProcess>

class ProcManager : public QObject
{
    Q_OBJECT

  public:
    explicit ProcManager(QObject *parent = nullptr);
    ~ProcManager();

    static ProcManager *Instance();
    void                init();
    void                destroy();

  signals:
    void SignalCoreStarted();
    void SignalCoreFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void SignalCoreError(QProcess::ProcessError error);

  private:
    void _connectCore();
    void _disconnectCore();

  private slots:
    void _slotCoreStarted();
    void _slotCoreFinished(int, QProcess::ExitStatus);
    void _slotCoreError(QProcess::ProcessError);

  private:
    static ProcManager *m_stProcManagerInst;
    QProcess           *m_pCore = nullptr;
};

#endif // PROCMANAGER_H