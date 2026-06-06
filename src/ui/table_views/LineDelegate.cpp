#include "LineDelegate.h"

LineDelegate::LineDelegate(QTableView *tableView)
{
    int gridHint = tableView->style()->styleHint(QStyle::SH_Table_GridLineColor,
                                                 new QStyleOptionViewItem());
    QColor gridColor = static_cast<QRgb>(gridHint);
    m_qPen           = QPen(gridColor, 0, tableView->gridStyle());

    m_pTabelView = tableView;
}

void LineDelegate::paint(QPainter                   *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex          &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    QPen oldPen = painter->pen();
    painter->setPen(m_qPen);
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    painter->setPen(oldPen);
}

void LineDelegate::SetPenColor(QColor qColor)
{
    m_qPen.setColor(qColor);
}
