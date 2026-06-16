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
    void _slotGetSessionResp(const int                              errorCode,
                             const QVector<::GrpcLibrary::Session> &sessions);
    void _slotNewSessionResp(const int                     errorCode,
                             const ::GrpcLibrary::Session &session);
    void _slotModifySessionTitleResp(const int      errorCode,
                                     const int64_t  id,
                                     const QString &title);
    void _slotSessionCtlBtnGroupClicked(int id);
    void _slotEditFilterTextChanged(const QString &content);

  private:
    void _initSkillsArea();
    void _drawSkillsArea();
    void _initHistoryArea();
    void _initConnections();
    void _retranslate();

    void _addSessions(const QVector<::GrpcLibrary::Session> &sessions);
    void _delSessions(const QVector<int64_t> &sessionIds);
    void _refreshSessionTable(bool clearFirst = false);
    void _filterSessionTable(const QString &filterText);

  private:
    Ui::HomePageWidget    *ui;
    static HomePageWidget *m_stMainHomePageInst;

    QButtonGroup *m_pSkillsBtnGroup;
    QButtonGroup *m_pSessionCtlBtnGroup;

    QStandardItemModel *m_pHistoryModel;

    int              m_colNum;
    QVector<QString> m_skillsInfo;
};

#endif // HomePageWidget_H
