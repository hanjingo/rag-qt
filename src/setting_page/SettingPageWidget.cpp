#include <QtMath>
#include <QPainter>
#include <QAction>

#include "SettingPageWidget.h"
#include "ui_SettingPageWidget.h"

SettingPageWidget *SettingPageWidget::m_stMainSettingPageInst = nullptr;

SettingPageWidget *SettingPageWidget::GetMainSettingPageInst()
{
    if(nullptr == m_stMainSettingPageInst)
    {
        m_stMainSettingPageInst = new SettingPageWidget();
    }

    return m_stMainSettingPageInst;
}

SettingPageWidget::SettingPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageWidget)
{
    ui->setupUi(this);
}

SettingPageWidget::~SettingPageWidget()
{
    delete ui;
}
