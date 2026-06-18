#include "NewSessionDialog.h"
#include "ui_NewSessionDialog.h"

#include "StyleMgr.h"

NewSessionDialog::NewSessionDialog(const QVector<::GrpcLibrary::Skill> &skills,
                                   QWidget                             *parent)
    : QDialog(parent)
    , ui(new Ui::NewSessionDialog)
    , m_skills(skills)
{
    ui->setupUi(this);

    _retranslate();
    _initUI();
    _initConnections();
}

NewSessionDialog::~NewSessionDialog()
{
    delete ui;
}

void NewSessionDialog::_retranslate()
{
    ui->lblConfigTitle->setText(tr("Session Configuration"));
    ui->lblTitle->setText(tr("Title"));

    ui->lblSkill->setText(tr("Skill"));

    ui->lblLocalServer->setText(tr("Local Server"));

    ui->lblRemoteServer->setText(tr("Remote Server"));

    ui->lblRemoteIP->setText(tr("Remote IP"));
    ui->lblRemotePort->setText(tr("Remote Port"));
}

void NewSessionDialog::_initUI()
{
    ui->editTitle->setStyleSheet(StyleMgr::ParseFile(":/styles/line_edit"));

    for(const auto &skill : m_skills)
    {
        ui->comboSkill->addItem(QString::fromStdString(skill.name()));
    }

    // ui->comboSkill->setStyleSheet(StyleMgr::ParseFile(":/styles/combo_box"));

    ui->checkLocal->setStyleSheet(StyleMgr::ParseFile(":/styles/check_box"));
    ui->checkRemote->setStyleSheet(StyleMgr::ParseFile(":/styles/check_box"));
    ui->checkLocal->setChecked(true);
    ui->checkRemote->setChecked(false);

    ui->editRemoteIP->setEnabled(false);
    ui->editRemotePort->setEnabled(false);
}

void NewSessionDialog::_initConnections()
{
    connect(ui->checkRemote, &QCheckBox::toggled, this, [this](bool checked) {
        ui->editRemoteIP->setEnabled(checked);
        ui->editRemotePort->setEnabled(checked);
    });
}