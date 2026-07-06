#ifndef HomePageWidget_H
#define HomePageWidget_H

#include <QMap>
#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QResizeEvent>
#include <QStandardItemModel>

#include <functional>
#include <libqt/net/downloader.h>

#include "SkillBtn.h"
#include "GrpcClient.h"
#include "HistorySettingDialog.h"

namespace Ui
{
class HomePageWidget;
}

class HomePageWidget : public QWidget
{
    Q_OBJECT

  public:
    static HomePageWidget *GetMainHomePageInst();
    explicit HomePageWidget(QWidget *parent = nullptr);
    ~HomePageWidget();

    QVector<Bus::Skill> GetSkillInfos();

  protected:
    void changeEvent(QEvent *event) override;

  signals:

  private slots:
    void _slotSkillBtnClicked(QAbstractButton *);
    void _slotGrpcConnected(const QString &address);
    void _slotEditFilterTextChanged(const QString &content);

    void _slotSessionCtlBtnGroupClicked(int id);
    void _slotGetSessionResp(const int                    errorCode,
                             const QVector<Bus::Session> &sessions);
    void _slotNewSessionResp(const int errorCode, const Bus::Session &session);
    void _slotModifySessionTitleResp(const int      errorCode,
                                     const int64_t  id,
                                     const QString &title);
    void _slotDelSessionResp(const int errorCode, const QVector<int64_t> &ids);

    void _slotGetSkillInfoResp(const int                  errorCode,
                               const QVector<Bus::Skill> &skills);

    void _slotDownloadResp(const int      errorCode,
                           const QString &hash,
                           const QString &addr,
                           const int64_t  size_kb);

    void _slotSkillBtnStateChanged(SkillBtn *btn, SkillBtn::State state);

    void _slotPluginUnloaded(const QString &pluginId);

  private:
    void _initSkillsArea();
    void _initHistoryArea();
    void _initConnections();
    void _retranslate();

    void _addSessions(const QVector<Bus::Session> &sessions);
    void _delSessions(const QVector<int64_t> &sessionIds);
    void _refreshSessionTable(bool clearFirst = false);
    void _filterSessionTable(const QString &filterText);

    void _addSkills(const QVector<Bus::Skill> &skills);
    void _getSkills(
        QVector<Bus::Skill>              &skills,
        std::function<bool(Bus::Skill &)> filter = [](Bus::Skill &) {
            return true;
        });
    void _clearSkills();
    void _drawSkillsArea();

    void _download(SkillBtn *btn, const QUrl &url);
    // QString _unzip(SkillBtn *btn);

  private:
    Ui::HomePageWidget    *ui;
    static HomePageWidget *m_stMainHomePageInst;

    QButtonGroup *m_pSkillsBtnGroup;
    QButtonGroup *m_pSessionCtlBtnGroup;

    QStandardItemModel *m_pHistoryModel;

    int                          m_colNum;
    int                          m_maxRecord;
    HistorySettingDialog::SortBy m_sortBy;
};

#endif // HomePageWidget_H
