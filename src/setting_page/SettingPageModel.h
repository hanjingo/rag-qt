#ifndef SETTINGPAGEMODEL_H
#define SETTINGPAGEMODEL_H

#include <QMap>
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QButtonGroup>
#include <QPointer>

#include "GrpcClient.h"
#include "Bus.h"
#include "Config.h"

namespace Ui
{
class SettingPageModel;
}

class SettingPageModel : public QWidget
{
    Q_OBJECT

  public:
    static QPointer<SettingPageModel> instance()
    {
        static QPointer<SettingPageModel> inst = new SettingPageModel();
        return inst;
    }
    explicit SettingPageModel(QWidget *parent = nullptr);
    ~SettingPageModel();

  public:
    QVector<Bus::ModelInfo> GetBusModelInfos();

  signals:

  protected:
    void changeEvent(QEvent *event) override;

  private slots:
    void _slotEditFilterTextChanged(const QString &content);
    void _slotModelCtlBtnGroupClicked(int id);
    void _slotLoginResp(const int      errorCode,
                        const int64_t  userId,
                        const QString &auth,
                        const QString &account,
                        const QString &lastLoginTime,
                        const bool     isForceUpdate);
    void _slotModelConfigUpdate();

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _clearModels();
    void _addModels(const QVector<Config::ModelConfig> &configs,
                    const QString                      &tag = "Staged");
    void _setModels(const QVector<Config::ModelConfig> &configs,
                    const QString                      &tag = "Staged");

    QVector<Bus::ModelInfo> _getBusModelInfos(const QVector<int> &rows = {});
    void                    _saveModelConfigs();
    void                    _importModelConfigs();
    void                    _refreshModelTable(bool clearFirst = false);
    void                    _filterModelTable(const QString &filterText);

  private:
    Ui::SettingPageModel *ui;
    QButtonGroup         *m_pModelCtlBtnGroup;
    QStandardItemModel   *m_pLLMListModel;
};

#endif // SETTINGPAGEMODEL_H
