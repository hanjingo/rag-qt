#include "NewSessionDialog.h"
#include "ui_NewSessionDialog.h"

#include "StyleMgr.h"

NewSessionDialog::NewSessionDialog(const QVector<QString> &models,
                                   QWidget                *parent)
    : QDialog(parent)
    , ui(new Ui::NewSessionDialog)
    , m_models{models}
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

    ui->lblModel->setText(tr("Model"));

    ui->lblLocalServer->setText(tr("Local Server"));

    ui->lblRemoteServer->setText(tr("Remote Server"));

    ui->lblRemoteIP->setText(tr("Remote IP"));
    ui->lblRemotePort->setText(tr("Remote Port"));
}

void NewSessionDialog::_initUI()
{
    ui->editTitle->setStyleSheet(StyleMgr::ParseFile(":/styles/line_edit"));

    // ui->comboModel->setStyleSheet(StyleMgr::ParseFile(":/styles/combo_box"));
    for(const auto &model : m_models)
        ui->comboModel->addItem(model);

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

void NewSessionDialog::GetConfig(Bus::Session &sess,
                                 QString      &model,
                                 QString      &prompt,
                                 bool         &isLocal,
                                 bool         &isRemote)
{
    sess.title     = ui->editTitle->text();
    sess.timestamp = QDateTime::currentDateTime().toString("%Y-%m-%d %H:%M:%S");

    model = ui->comboModel->currentText();

    prompt = ui->editPrompt->toPlainText();

    isLocal  = ui->checkLocal->isChecked();
    isRemote = ui->checkRemote->isChecked();
}