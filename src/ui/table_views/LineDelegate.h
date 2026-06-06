#ifndef LINEDELEGATE_H
#define LINEDELEGATE_H

#include <QPainter>
#include <QTableView>
#include <QStyledItemDelegate>

class LineDelegate : public QStyledItemDelegate
{
  public:
    explicit LineDelegate(QTableView *tableView = nullptr);

  private:
    void SetPenColor(QColor);

  protected:
    void
    paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;

  private:
    QTableView *m_pTabelView;
    QPen        m_qPen;
};

#endif // LINEDELEGATE_H
