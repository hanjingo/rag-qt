#include <QTime>
#include <QTimer>
#include <QDateTime>
#include <QScrollBar>
#include <QHeaderView>
#include <QApplication>
#include <QPropertyAnimation>

#include "AllRadiusTableView.h"
#include "LineDelegate.h"

AllRadiusTableView::AllRadiusTableView(QWidget *parent)
    : QTableView(parent)
{
    verticalHeader()->setDefaultSectionSize(66);
    //horizontalHeader()->setFixedHeight(71);
    horizontalHeader()->setHighlightSections(false);
    horizontalHeader()->setDisabled(true);
    setAutoScroll(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    verticalHeader()->setVisible(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setShowGrid(false);
    setItemDelegate(new LineDelegate(this));
    m_pScrollBarV = this->verticalScrollBar();

    m_iPress_y     = 0;
    m_iMove_y      = -1;
    m_iRelease_y   = 0;
    m_pAnimation   = new QPropertyAnimation();
    m_iScrollV_max = m_pScrollBarV->maximum();
    m_iScrollV_min = m_pScrollBarV->minimum();
}

void AllRadiusTableView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex idx = indexAt(QPoint(event->x(), event->y()));
    if(false == idx.isValid())
    {
        ClearSelection();
        emit SignalTabViewSelection(false);
        emit SignalTabViewIndexSelection(false, -1);
        return;
    }
    m_iScrollV_min  = m_pScrollBarV->minimum();
    m_iScrollV_max  = m_pScrollBarV->maximum();
    bool bIsChecked = false;
    if(event->button() == Qt::LeftButton)
    {
        int before = this->currentIndex().row();
        if(before == -1)
        {
            m_Index = this->currentIndex();
        }

        QTableView::mousePressEvent(event);
        emit clicked(currentIndex());
        m_iPressRow = currentIndex().row();
        int iNum    = m_SelectedRowList.indexOf(currentIndex().row());
        if(iNum >= 0)
        {
            bIsChecked = false;
            m_SelectedRowList.takeAt(iNum);
            if(m_SelectedRowList.count() < 1)
            {
                ClearSelection();
            }
        } else
        {
            if(-1 == m_iPressRow)
            {
                bIsChecked = false;
            } else
            {
                bIsChecked = true;
                if(this->selectionMode() == QAbstractItemView::SingleSelection)
                {
                    m_SelectedRowList.clear();
                }
                m_SelectedRowList.append(m_iPressRow);
            }
        }

        emit SignalTabViewSelection(bIsChecked);
        emit SignalTabViewIndexSelection(bIsChecked, m_iPressRow);

        m_PressTime = QDateTime::currentDateTime();
        m_iMove_y   = event->pos().y();
        m_iPress_y  = m_iMove_y;
        m_pAnimation->stop();
    }
    if(event->button() == Qt::RightButton)
    {
        if(0 < m_SelectedRowList.count())
        {
            ClearSelection();
            emit SignalTabViewSelection(bIsChecked);
            emit SignalTabViewIndexSelection(bIsChecked, m_iPressRow);
        }
    }
}

void AllRadiusTableView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(m_pAnimation->targetObject() != m_pScrollBarV)
        {
            m_pAnimation->setTargetObject(m_pScrollBarV);
            m_pAnimation->setPropertyName("value");
        }
        m_iMove_y    = -1;
        m_iRelease_y = event->pos().y();
        int endValue = 0;
        int pageStep;

        if(m_iRelease_y - m_iPress_y != 0
           && qAbs(m_iRelease_y - m_iPress_y) > 45)
        {
            QItemSelectionModel *pSelectionModel = this->selectionModel();
            QModelIndexList      selectedIndexList =
                pSelectionModel->selectedIndexes();
            QList<int> ListSelectedRow;
            foreach(QModelIndex index, selectedIndexList)
            {
                int iRow = index.row();
                if(true == ListSelectedRow.contains(iRow))
                {
                    continue;
                }
                ListSelectedRow.append(iRow);
            }

            int iRow = m_iPressRow;
            int iNum = m_SelectedRowList.indexOf(iRow);
            if(iNum >= 0)
            {
                m_SelectedRowList.takeAt(iNum);
                emit SignalTabViewSelection(false);
                emit SignalTabViewIndexSelection(false,
                                                 this->currentIndex().row());
            }

            for(int i = 0; i < ListSelectedRow.count(); i++)
            {
                selectRow(ListSelectedRow.at(i));
            }
            for(int i = 0; i < m_SelectedRowList.count(); i++)
            {
                selectRow(m_SelectedRowList.at(i));
            }

            int limit    = 440;
            int mseconds = m_PressTime.msecsTo(QDateTime::currentDateTime());
            pageStep     = m_pScrollBarV->pageStep();
            if(mseconds > limit)
            {
                mseconds = mseconds + (mseconds - limit) * 20;
            }

            if(m_iRelease_y - m_iPress_y > 0)
            {
                endValue =
                    m_pScrollBarV->value() - pageStep * (200.0 / mseconds);
                if(m_iScrollV_min > endValue)
                {
                    endValue = m_iScrollV_min;
                }
            } else if(m_iRelease_y - m_iPress_y < 0)
            {
                endValue =
                    m_pScrollBarV->value() + pageStep * (200.0 / mseconds);
                if(endValue > m_iScrollV_max)
                {
                    endValue = m_iScrollV_max;
                }
            }
            if(mseconds > limit)
            {
                mseconds = 0;
            }
            m_pAnimation->setDuration(mseconds + 1550);
            m_pAnimation->setEndValue(endValue);
            m_pAnimation->setEasingCurve(QEasingCurve::OutQuad);
            m_pAnimation->start();
        }
    }
}

void AllRadiusTableView::mouseMoveEvent(QMouseEvent *event)
{
    int iMoveDis = event->pos().y() - m_iMove_y;
    int endValue = m_pScrollBarV->value() - iMoveDis;
    if(m_iScrollV_min > endValue)
    {
        endValue = m_iScrollV_min;
    }
    if(endValue > m_iScrollV_max)
    {
        endValue = m_iScrollV_max;
    }
    m_pScrollBarV->setValue(endValue);
    m_iMove_y = event->pos().y();
}

void AllRadiusTableView::SlotShowSelectionRow()
{
    for(int i = 0; i < m_SelectedRowList.count(); i++)
    {
        this->selectRow(m_SelectedRowList.at(i));
    }
}

void AllRadiusTableView::SlotChangeFilter()
{
    m_SelectedRowList.clear();
}

void AllRadiusTableView::InitHeaderInfo(QList<int> ShowHeaderList)
{
    for(int i = 0; i < this->horizontalHeader()->count(); i++)
    {
        if(false == ShowHeaderList.contains(i))
        {
            this->setColumnHidden(i, true);
        }
    }
}

void AllRadiusTableView::ClearSelection()
{
    m_SelectedRowList.clear();
    m_iPressRow = -1;
    this->clearSelection();
}

QList<int> &AllRadiusTableView::GetSelectedRows()
{
    return m_SelectedRowList;
}

int AllRadiusTableView::GetCurrectSelectedRow()
{
    if(m_SelectedRowList.count() > 0)
    {
        m_iPressRow = m_SelectedRowList.last();
    } else
    {
        m_iPressRow = -1;
    }
    return m_iPressRow;
}

void AllRadiusTableView::SelSameProRow(int iRow)
{
    int iNum = m_SelectedRowList.indexOf(iRow);
    if(iNum >= 0)
    {
        m_SelectedRowList.removeOne(iRow);
    } else
    {
        m_SelectedRowList.append(iRow);
    }
    this->selectRow(iRow);
}

QList<int> AllRadiusTableView::GetSelRowDesc()
{
    QList<int> lstSelRows = m_SelectedRowList;
    std::sort(lstSelRows.begin(), lstSelRows.end(), _CmpRowMax);
    return lstSelRows;
}

bool AllRadiusTableView::_CmpRowMax(int iFstRow, int iScdRow)
{
    bool blMax = false;
    if(iFstRow > iScdRow)
    {
        blMax = true;
    }
    return blMax;
}
