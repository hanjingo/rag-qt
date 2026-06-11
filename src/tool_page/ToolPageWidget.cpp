#include <QtMath>
#include <QPainter>
#include <QAction>

#include "ToolPageWidget.h"
#include "ui_ToolPageWidget.h"

ToolPageWidget *ToolPageWidget::m_stToolPageWidgetInst = nullptr;

ToolPageWidget *ToolPageWidget::GetToolPageWidgetInst()
{
    if(nullptr == m_stToolPageWidgetInst)
    {
        m_stToolPageWidgetInst = new ToolPageWidget();
    }

    return m_stToolPageWidgetInst;
}

ToolPageWidget::ToolPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ToolPageWidget)
{
    ui->setupUi(this);
}

ToolPageWidget::~ToolPageWidget()
{
    delete ui;
}
