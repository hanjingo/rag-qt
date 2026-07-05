#include "ProcManager.h"

#include <QDebug>
#include <libqt/core/process.h>
#include <QMessageBox>

#include "Config.h"

ProcManager *ProcManager::m_stProcManagerInst = nullptr;
ProcManager *ProcManager::Instance()
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
    if(!Config::Instance().isCoreRun())
    {
        qDebug() << "Core process is disabled in config.";
        return;
    }

    // end the current core process if it exists
    if(m_pCore)
    {
        _disconnectCore();
        Process::kill(m_pCore->processId());
        delete m_pCore;
        m_pCore = nullptr;
    }

    // scan existing core processes
    QVector<qint64> cores;
    Process::list(cores, [](const QStringList &cols) {
        return cols.size() >= 2
               && (cols[0] == "rag-core" || cols[0] == "rag-core.exe");
    });
    for(qint64 pid : cores)
        Process::kill(pid);

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

QString ProcManager::readAllStandardOutput()
{
    if(!m_pCore)
        return QString();

    return QString::fromLocal8Bit(m_pCore->readAllStandardOutput());
}

void ProcManager::_connectCore()
{
    if(!m_pCore)
        return;

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
    if(!m_pCore)
        return;

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
    emit SignalCoreStarted();
}

void ProcManager::_slotCoreFinished(int                  exitCode,
                                    QProcess::ExitStatus exitStatus)
{
    qDebug() << "Core finished. Exit code:" << exitCode
             << "Exit status:" << exitStatus;
    emit SignalCoreFinished(exitCode, exitStatus);
}

void ProcManager::_slotCoreError(QProcess::ProcessError error)
{
    qDebug() << "Core error:" << error;
    emit SignalCoreError(error);
}