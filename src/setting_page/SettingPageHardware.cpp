#include <QtMath>
#include <QPainter>
#include <QAction>

#include "SettingPageHardware.h"
#include "ui_SettingPageHardware.h"

SettingPageHardware *SettingPageHardware::m_stInstance = nullptr;
SettingPageHardware *SettingPageHardware::Instance()
{
    if(nullptr == m_stInstance)
        m_stInstance = new SettingPageHardware();

    return m_stInstance;
}

SettingPageHardware::SettingPageHardware(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageHardware)
{
    ui->setupUi(this);
}

SettingPageHardware::~SettingPageHardware()
{
    delete ui;
}
