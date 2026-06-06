#include <QtMath>
#include <QPainter>
#include <QAction>

#include "SettingPageHistory.h"
#include "ui_SettingPageHistory.h"

SettingPageHistory *SettingPageHistory::m_stSettingPageHistoryInst = nullptr;

SettingPageHistory *SettingPageHistory::GetSettingPageHistoryInst()
{
    if(nullptr == m_stSettingPageHistoryInst)
    {
        m_stSettingPageHistoryInst = new SettingPageHistory();
    }

    return m_stSettingPageHistoryInst;
}

SettingPageHistory::SettingPageHistory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageHistory)
{
    ui->setupUi(this);
}

SettingPageHistory::~SettingPageHistory()
{
    delete ui;
}
