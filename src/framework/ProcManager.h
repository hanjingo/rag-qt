#ifndef PROCMANAGER_H
#define PROCMANAGER_H

#include <QProcess>

class ProcManager : public QObject
{
    Q_OBJECT

  public:
    static ProcManager *GetProcManagerInst();
    explicit ProcManager(QObject *parent = nullptr);
    ~ProcManager();

    void init();
    void destroy();

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