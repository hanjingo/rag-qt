#ifndef SETTINGPAGEMODEL_H
#define SETTINGPAGEMODEL_H

#include <QMap>
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QButtonGroup>

#include "GrpcClient.h"
#include "Bus.h"

namespace Ui
{
class SettingPageModel;
}

class SettingPageModel : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageModel *Instance();
    explicit SettingPageModel(QWidget *parent = nullptr);
    ~SettingPageModel();

  public:
    QVector<Bus::ModelInfo> GetModelInfos();

  signals:

  protected:
    void changeEvent(QEvent *event) override;

  private slots:
    void _slotModelCtlBtnGroupClicked(int id);
    void _slotTbviewModelSelectionChanged(const QItemSelection &selected,
                                          const QItemSelection &deselected);

    void _slotGetModelInfoResp(const int                            errorCode,
                               const QVector<::GrpcLibrary::Model> &modelInfos);
    void _slotNewModelInfoResp(const int               errorCode,
                               const QVector<QString> &hashs);

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();

    void _addModels(const QVector<::GrpcLibrary::Model> &models);
    void _delModels(const QVector<int64_t> &modelIds);
    void _refreshModelTable(bool clearFirst = false);
    void _filterModelTable(const QString &filterText);

  private:
    Ui::SettingPageModel    *ui;
    static SettingPageModel *m_stSettingPageModelInst;

    QButtonGroup *m_pModelCtlBtnGroup;

    QStandardItemModel *m_pLLMListModel;
};

#endif // SETTINGPAGEMODEL_H
