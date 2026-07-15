#ifndef MemoryConfigDialog_H
#define MemoryConfigDialog_H

#include <QDialog>
#include <QVector>
#include <QButtonGroup>

#include "ui_MemoryConfigDialog.h"
#include "Config.h"

namespace Ui
{
class MemoryConfigDialog;
}

class MemoryConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit MemoryConfigDialog(const Config::MemoryConfig &conf,
                                QWidget                    *parent = nullptr);
    ~MemoryConfigDialog();

    Config::MemoryConfig GetConfig();

  signals:
    void SignalGenerateMemory(const Config::MemoryConfig &conf);

  public slots:
    void SlotEmbeddingProgressUpdate(const Config::MemoryConfig &conf,
                                     const int64_t finishedChunkNum,
                                     const int64_t totalChunkNum);

  private slots:
    void _slotBtnIndexFilePathClicked();
    void _slotBtnMetaFilePathClicked();
    void _slotBtnOriginFilePathClicked();
    void _slotBtnGenerateClicked();
    void _slotBtnCancelClicked();

  private:
    void _retranslate();
    void _initUI();
    void _initConnections();

  private:
    Ui::MemoryConfigDialog *ui;
    Config::MemoryConfig    m_conf;
};

#endif // MemoryConfigDialog_H
