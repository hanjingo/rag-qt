#include <QtMath>
#include <QPainter>
#include <QAction>

#include "AudioPageWidget.h"
#include "ui_AudioPageWidget.h"

AudioPageWidget *AudioPageWidget::m_stAudioPageWidgetInst = nullptr;

AudioPageWidget *AudioPageWidget::GetAudioPageWidgetInst()
{
    if(nullptr == m_stAudioPageWidgetInst)
    {
        m_stAudioPageWidgetInst = new AudioPageWidget();
    }

    return m_stAudioPageWidgetInst;
}

AudioPageWidget::AudioPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AudioPageWidget)
{
    ui->setupUi(this);
}

AudioPageWidget::~AudioPageWidget()
{
    delete ui;
}
