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
    void _slotGetHistoryResp(const QVector<GrpcClient::History> &resp);

  private:
    void _initSkillsArea();
    void _drawSkillsArea();
    void _initHistoryArea();
    void _initConnections();
    void _retranslateTexts();

  private:
    Ui::HomePageWidget    *ui;
    static HomePageWidget *m_stMainHomePageInst;

    QVector<QString>    m_skillsInfo;
    QButtonGroup       *m_pSkillsBtnGroup;
    QStandardItemModel *m_pHistoryModel;
    int                 m_colNum;
};

#endif // HomePageWidget_H
