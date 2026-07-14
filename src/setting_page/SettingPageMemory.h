#ifndef SETTINGPAGEMEMORY_H
#define SETTINGPAGEMEMORY_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QButtonGroup>

#include "Config.h"

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
    static SettingPageMemory *Instance()
    {
        static SettingPageMemory inst;
        return &inst;
    }

  protected:
    explicit SettingPageMemory(QWidget *parent = nullptr);
    ~SettingPageMemory();

    void _initUI();
    void _initConnections();
    void _retranslate();

    void _addMemorys(const QVector<Config::MemoryConfig> &configs,
                     const QString                       &tag = "Staged");
    void _delMemorys(const QVector<QString> &hashs);
    void _setMemorys(const QVector<Config::MemoryConfig> &configs,
                     const QString                       &tag = "Staged");

  private slots:
    void _slotMemCtlBtnGroupClicked(int id);
    void _slotGenerateMemory(const Config::MemoryConfig &conf);
    void _slotEmbeddingResp(const int      errorCode,
                            const int64_t  taskId,
                            const int64_t  chunkId,
                            const QString &vectorIndexs);
    void _slotEmbeddingStopResp(const int errorCode, const int64_t taskId);

  private:
    QVector<Config::MemoryConfig>
         _getMemoryInfos(const QVector<int> &rows = {});
    void _refreshMemTable(bool clearFirst = false);
    void _filterMemTable(const QString &filterText);

    void _saveMemoryConfigs();
    void _importMemoryConfigs();

  private:
    Ui::SettingPageMemory *ui;

    QButtonGroup *m_pMemCtlBtnGroup;

    QStandardItemModel *m_pMemListModel;

    // key: taskId, value: chunkIds
    QMap<int64_t, QSet<int64_t>> m_mapTaskChunkIds;
};

#endif // SETTINGPAGEMEMORY_H
