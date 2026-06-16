#include "NewSessionDialog.h"
#include "ui_NewSessionDialog.h"

NewSessionDialog::NewSessionDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewSessionDialog)
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
#ifdef DEBUG
    ui->comboSkill->addItem("Skill1");
    ui->comboSkill->addItem("Skill2");
    ui->comboSkill->addItem("Skill3");
    ui->comboSkill->addItem("Skill4");
#endif

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