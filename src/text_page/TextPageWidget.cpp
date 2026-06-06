#include <QtMath>
#include <QPainter>
#include <QAction>

#include "TextPageWidget.h"
#include "ui_TextPageWidget.h"

TextPageWidget *TextPageWidget::m_stTextPageWidgetInst = nullptr;

TextPageWidget *TextPageWidget::GetTextPageWidgetInst()
{
    if(nullptr == m_stTextPageWidgetInst)
    {
        m_stTextPageWidgetInst = new TextPageWidget();
    }

    return m_stTextPageWidgetInst;
}

TextPageWidget::TextPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TextPageWidget)
{
    ui->setupUi(this);
}

TextPageWidget::~TextPageWidget()
{
    delete ui;
}
