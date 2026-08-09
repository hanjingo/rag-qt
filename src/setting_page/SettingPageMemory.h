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
                                       const int finishedChunkNum,
                                       const int totalChunkNum);

  protected:
    explicit SettingPageMemory(QWidget *parent = nullptr);
    ~SettingPageMemory();

    void changeEvent(QEvent *event) override;

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
    void _slotMemoryConfigUpdate();

    void _slotEmbeddingProgress(const int64_t     taskId,
                                const int64_t     chunkId,
                                const QString    &memoryId,
                                const int         totalChunkNum,
                                const int         finishedChunkNum,
                                const QByteArray &vectorIndexs);

  private:
    void _filteMemTable(const QString &filterText);
    QVector<Config::MemoryConfig>
         _getMemoryInfos(const QVector<int> &rows = {});
    void _refreshMemTable(bool clearFirst = false);
    void _filterMemTable(const QString &filterText);

    void _saveMemoryConfigs();
    void _importMemoryConfigs();

    void _removeTaskRecord(const int64_t taskId);

    MemoryConfigDialog *
    _createMemoryConfigDialog(const Config::MemoryConfig &conf);

  private:
    Ui::SettingPageMemory *ui;

    QButtonGroup *m_pMemCtlBtnGroup;

    QStandardItemModel *m_pMemListModel;

    int64_t m_currTaskId = -1;
};

#endif // SETTINGPAGEMEMORY_H
