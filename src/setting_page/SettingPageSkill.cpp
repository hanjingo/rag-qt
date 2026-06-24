#include "Bus.h"
#include "GrpcClient.h"

#include "Error.h"
#include "StyleMgr.h"

#include "SettingPageSkill.h"
#include "ui_SettingPageSkill.h"

SettingPageSkill *SettingPageSkill::m_stSettingPageSkillInst = nullptr;

SettingPageSkill *SettingPageSkill::Instance()
{
    if(nullptr == m_stSettingPageSkillInst)
    {
        m_stSettingPageSkillInst = new SettingPageSkill();
    }

    return m_stSettingPageSkillInst;
}

SettingPageSkill::SettingPageSkill(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageSkill)
{
    ui->setupUi(this);

    _initUI();
    _initConnections();
    _retranslate();
}

SettingPageSkill::~SettingPageSkill()
{
    delete ui;
}

void SettingPageSkill::_initConnections()
{
}

void SettingPageSkill::_initUI()
{
}

void SettingPageSkill::_retranslate()
{
}