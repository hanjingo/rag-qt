#include <QtMath>
#include <QPainter>
#include <QAction>

#include "TextPageWidget.h"
#include "ui_TextPageWidget.h"
#include "GrpcClient.h"

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
    _init();
    _initConnections();
}

TextPageWidget::~TextPageWidget()
{
    delete ui;
}

void TextPageWidget::_slotBtnStartClicked()
{
    const bool checked = ui->btnStart->isChecked();
    if(checked)
    {
        qDebug() << "Start query.";
        QString content = ui->editInput->text();
        GrpcClient::GetGrpcClientInst()->query(content);
    } else
    {
        qDebug() << "Stop query.";
    }
}

void TextPageWidget::_init()
{
    // Keep visual pressed state after click until toggled again.
    ui->btnStart->setCheckable(true);
    ui->btnStart->setChecked(false);

    // edit line
    ui->editInput->setPlaceholderText("Input...");
}

void TextPageWidget::_initConnections()
{
    connect(ui->btnStart,
            &QPushButton::clicked,
            this,
            &TextPageWidget::_slotBtnStartClicked);

    connect(GrpcClient::GetGrpcClientInst(),
            &GrpcClient::SignalQueryResp,
            this,
            &TextPageWidget::_slotQueryResp);
}

void TextPageWidget::_slotQueryResp(const QString &resp)
{
    qDebug() << "Query Respn with " << resp;
}