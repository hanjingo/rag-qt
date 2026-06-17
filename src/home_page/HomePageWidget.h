#ifndef HomePageWidget_H
#define HomePageWidget_H

#include <QMap>
#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QResizeEvent>
#include <QStandardItemModel>

#include "GrpcClient.h"

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

  protected:
    void changeEvent(QEvent *event) override;

  signals:

  private slots:
    void _slotSkillBtnClicked(QAbstractButton *);
    void _slotGrpcConnected(const QString &address);
    void _slotEditFilterTextChanged(const QString &content);

    void _slotGetSessionResp(const int                              errorCode,
                             const QVector<::GrpcLibrary::Session> &sessions);
    void _slotNewSessionResp(const int                     errorCode,
                             const ::GrpcLibrary::Session &session);
    void _slotModifySessionTitleResp(const int      errorCode,
                                     const int64_t  id,
                                     const QString &title);
    void _slotSessionCtlBtnGroupClicked(int id);

    void _slotGetSkillInfoResp(const int                            errorCode,
                               const QVector<::GrpcLibrary::Skill> &skills);

    void _slotDownloadResp(const int      errorCode,
                           const QString &hash,
                           const QString &addr,
                           const int64_t  size_kb);

  private:
    void _initSkillsArea();
    void _initHistoryArea();
    void _initConnections();
    void _retranslate();

    void _addSessions(const QVector<::GrpcLibrary::Session> &sessions);
    void _delSessions(const QVector<int64_t> &sessionIds);
    void _refreshSessionTable(bool clearFirst = false);
    void _filterSessionTable(const QString &filterText);

    void _addSkills(const QVector<::GrpcLibrary::Skill> &skills);
    void _clearSkills();
    void _drawSkillsArea();

  private:
    Ui::HomePageWidget    *ui;
    static HomePageWidget *m_stMainHomePageInst;

    QButtonGroup *m_pSkillsBtnGroup;
    QButtonGroup *m_pSessionCtlBtnGroup;

    QStandardItemModel *m_pHistoryModel;

    int m_colNum;
};

#endif // HomePageWidget_H
