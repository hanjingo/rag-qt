#include "ProcManager.h"

#include <QDebug>
#include <libqt/core/process.h>

ProcManager *ProcManager::m_stProcManagerInst = nullptr;
ProcManager *ProcManager::GetProcManagerInst()
{
    if(nullptr == m_stProcManagerInst)
        m_stProcManagerInst = new ProcManager();

    return m_stProcManagerInst;
}

ProcManager::ProcManager(QObject *parent)
    : QObject(parent)
    , m_pCore(nullptr)
{
}

ProcManager::~ProcManager()
{
    if(m_pCore)
    {
        Process::kill(m_pCore->processId());
        delete m_pCore;
        m_pCore = nullptr;
    }
}

void ProcManager::init()
{
    if(m_pCore)
    {
        _disconnectCore();
        Process::kill(m_pCore->processId());
        delete m_pCore;
        m_pCore = nullptr;
    }

    m_pCore = new QProcess(this);
    _connectCore();

#if defined(Q_OS_WIN)
    const QString program = QStringLiteral("rag-core.exe");
#else
    const QString program = QStringLiteral("./rag-core");
#endif
    const QStringList arguments = {QStringLiteral("run")};

    m_pCore->start(program, arguments);
    qDebug() << program << arguments;
}

void ProcManager::destroy()
{
    if(m_pCore)
    {
        Process::kill(m_pCore->processId());
        _disconnectCore();
        delete m_pCore;
        m_pCore = nullptr;
    }
}

void ProcManager::_connectCore()
{
    connect(m_pCore, &QProcess::started, this, &ProcManager::_slotCoreStarted);
    connect(m_pCore,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &ProcManager::_slotCoreFinished);
    connect(m_pCore,
            &QProcess::errorOccurred,
            this,
            &ProcManager::_slotCoreError);
}

void ProcManager::_disconnectCore()
{
    disconnect(m_pCore,
               &QProcess::started,
               this,
               &ProcManager::_slotCoreStarted);
    disconnect(m_pCore,
               QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
               this,
               &ProcManager::_slotCoreFinished);
    disconnect(m_pCore,
               &QProcess::errorOccurred,
               this,
               &ProcManager::_slotCoreError);
}

void ProcManager::_slotCoreStarted()
{
    qDebug() << "Core started.";
}

void ProcManager::_slotCoreFinished(int                  exitCode,
                                    QProcess::ExitStatus exitStatus)
{
    qDebug() << "Core finished. Exit code:" << exitCode
             << "Exit status:" << exitStatus;
}

void ProcManager::_slotCoreError(QProcess::ProcessError error)
{
    qDebug() << "Core error:" << error;
}