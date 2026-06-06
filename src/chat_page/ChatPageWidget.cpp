#include <QtMath>
#include <QPainter>
#include <QAction>

#include "ChatPageWidget.h"
#include "ui_ChatPageWidget.h"

ChatPageWidget *ChatPageWidget::m_stChatPageWidgetInst = nullptr;
ChatPageWidget *ChatPageWidget::GetChatPageWidgetInst()
{
    if(nullptr == m_stChatPageWidgetInst)
    {
        m_stChatPageWidgetInst = new ChatPageWidget();
    }

    return m_stChatPageWidgetInst;
}

ChatPageWidget::ChatPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatPageWidget)
{
    ui->setupUi(this);
}

ChatPageWidget::~ChatPageWidget()
{
    delete ui;
}
