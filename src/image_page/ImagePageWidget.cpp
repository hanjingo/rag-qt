#include <QtMath>
#include <QPainter>
#include <QAction>

#include "ImagePageWidget.h"
#include "ui_ImagePageWidget.h"

ImagePageWidget *ImagePageWidget::m_stImagePageWidgetInst = nullptr;

ImagePageWidget *ImagePageWidget::GetImagePageWidgetInst()
{
    if(nullptr == m_stImagePageWidgetInst)
    {
        m_stImagePageWidgetInst = new ImagePageWidget();
    }

    return m_stImagePageWidgetInst;
}

ImagePageWidget::ImagePageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ImagePageWidget)
{
    ui->setupUi(this);
}

ImagePageWidget::~ImagePageWidget()
{
    delete ui;
}
