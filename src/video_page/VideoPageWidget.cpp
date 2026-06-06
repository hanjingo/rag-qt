#include <QtMath>
#include <QPainter>
#include <QAction>

#include "VideoPageWidget.h"
#include "ui_VideoPageWidget.h"

VideoPageWidget *VideoPageWidget::m_stVideoPageWidgetInst = nullptr;
VideoPageWidget *VideoPageWidget::GetVideoPageWidgetInst()
{
    if(nullptr == m_stVideoPageWidgetInst)
    {
        m_stVideoPageWidgetInst = new VideoPageWidget();
    }

    return m_stVideoPageWidgetInst;
}

VideoPageWidget::VideoPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoPageWidget)
{
    ui->setupUi(this);
}

VideoPageWidget::~VideoPageWidget()
{
    delete ui;
}
