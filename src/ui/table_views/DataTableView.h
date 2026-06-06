#ifndef DATATABLEVIEW_H
#define DATATABLEVIEW_H

#include <QDateTime>
#include <QTableView>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QStyledItemDelegate>

class DataTableView : public QTableView
{
    Q_OBJECT

  signals:
    void SignalTabViewSelection(bool);
    void SignalTabViewIndexSelection(bool, int);

  public slots:
    void SlotChangeFilter();
    void SlotShowSelectionRow();

  public:
    explicit DataTableView(QWidget *parent = nullptr);

  public:
    void        InitHeaderInfo(QList<int> ShowHeaderList);
    void        ClearSelection();
    QList<int> &GetSelectedRows();
    int         GetCurrectSelectedRow();
    void        SelSameProRow(int);
    QList<int>  GetSelRowDesc();

  protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

  private:
    static bool _CmpRowMax(int, int);

  private:
    int                 m_iMove_y;
    int                 m_iPress_y;
    int                 m_iRelease_y;
    int                 m_iPressRow;
    int                 m_iClickRow;
    int                 m_iScrollV_max;
    int                 m_iScrollV_min;
    QDateTime           m_PressTime;
    QScrollBar         *m_pScrollBarV;
    QModelIndex         m_Index;
    QList<int>          m_SelectedRowList;
    QPropertyAnimation *m_pAnimation;
};

#endif // DATATABLEVIEW_H
