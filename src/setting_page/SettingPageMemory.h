#ifndef SETTINGPAGEMEMORY_H
#define SETTINGPAGEMEMORY_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QButtonGroup>
#include <QMutex>
#include <QPointer>

#include "Config.h"
#include "MemoryConfigDialog.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
class QButtonGroup;
QT_END_NAMESPACE

namespace Ui
{
class SettingPageMemory;
}

class SettingPageMemory : public QWidget
{
    Q_OBJECT

  public:
    static QPointer<SettingPageMemory> instance()
    {
        static QPointer<SettingPageMemory> inst = new SettingPageMemory();
        return inst;
    }

    QVector<Bus::MemoryInfo> GetBusMemoryInfos();

  signals:
    void signalEmbeddingProgressUpdate(const Config::MemoryConfig &conf,
                                       const int64_t finishedChunkNum,
                                       const int64_t totalChunkNum);

  protected:
    explicit SettingPageMemory(QWidget *parent = nullptr);
    ~SettingPageMemory();

    void _initUI();
    void _initConnections();
    void _retranslate();

    void _clearMemorys();
    void _addMemorys(const QVector<Config::MemoryConfig> &configs,
                     const QString                       &tag = "Staged");
    void _delMemorys(const QVector<QString> &hashs);
    void _setMemorys(const QVector<Config::MemoryConfig> &configs,
                     const QString                       &tag = "Staged");

  private slots:
    void _slotEditFilterTextChanged(const QString &content);
    void _slotMemCtlBtnGroupClicked(int id);
    void _slotGenerateMemory(const Config::MemoryConfig &conf);
    void _slotEmbeddingResp(const int         errorCode,
                            const int64_t     taskId,
                            const int64_t     chunkId,
                            const QByteArray &vectorIndexs);
    void _slotEmbeddingStopResp(const int errorCode, const int64_t taskId);
    void _slotMemoryConfigUpdate();

  private:
    void _filteMemTable(const QString &filterText);
    QVector<Config::MemoryConfig>
         _getMemoryInfos(const QVector<int> &rows = {});
    void _refreshMemTable(bool clearFirst = false);
    void _filterMemTable(const QString &filterText);

    void _saveMemoryConfigs();
    void _importMemoryConfigs();

    bool _setChunkProcessState(const int64_t taskId,
                               const int64_t chunkId,
                               const bool    isProcessed);
    bool _isChunkProcessed(const int64_t taskId, const int64_t chunkId = -1);
    QVector<int64_t> _getChunkIdsForTask(const int64_t taskId);
    bool             _isHadTask(const int64_t taskId);
    bool _setTaskConfig(const int64_t taskId, const Config::MemoryConfig &conf);
    Config::MemoryConfig _getTaskConfig(const int64_t taskId);
    void                 _removeTaskRecord(const int64_t taskId);

    MemoryConfigDialog *
    _createMemoryConfigDialog(const Config::MemoryConfig &conf);

  private:
    Ui::SettingPageMemory *ui;

    QButtonGroup *m_pMemCtlBtnGroup;

    QStandardItemModel *m_pMemListModel;

    // key: taskId, value: chunkIds
    QMutex                              m_mu;
    QMap<int64_t, QMap<int64_t, bool>>  m_mapTaskChunkIds;
    QMap<int64_t, Config::MemoryConfig> m_mapTaskConfigs;
};

#endif // SETTINGPAGEMEMORY_H
