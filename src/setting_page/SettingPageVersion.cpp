#include <QtMath>
#include <QPainter>
#include <QAction>

#include "SettingPageVersion.h"
#include "ui_SettingPageVersion.h"

SettingPageVersion *SettingPageVersion::m_stSettingPageVersionInst = nullptr;

SettingPageVersion *SettingPageVersion::GetSettingPageVersionInst()
{
    if(nullptr == m_stSettingPageVersionInst)
    {
        m_stSettingPageVersionInst = new SettingPageVersion();
    }

    return m_stSettingPageVersionInst;
}

SettingPageVersion::SettingPageVersion(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageVersion)
{
    ui->setupUi(this);
}

SettingPageVersion::~SettingPageVersion()
{
    delete ui;
}
