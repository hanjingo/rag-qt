#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QMessageBox>

#include "Error.h"

#include "SettingPageVersion.h"
#include "ui_SettingPageVersion.h"

SettingPageVersion::SettingPageVersion(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPageVersion)
{
    ui->setupUi(this);

    _initConnections();
    _initUI();
}

SettingPageVersion::~SettingPageVersion()
{
    delete ui;
}

void SettingPageVersion::_initUI()
{
    _retranslate();
}

void SettingPageVersion::_retranslate()
{
}

void SettingPageVersion::_initConnections()
{
    connect(GrpcClient::Instance(),
            &GrpcClient::SignalLoginResp,
            this,
            &SettingPageVersion::_slotLoginResp);
}

void SettingPageVersion::_slotLoginResp(const int      errorCode,
                                        const int64_t  user_id,
                                        const QString &auth,
                                        const int32_t  privilege,
                                        const QString &account,
                                        const QString &lastLoginTime,
                                        const bool     isForceUpdate)
{
    qDebug() << "SettingPageVersion::_slotLoginResp enter";
    if(errorCode != ErrorCode::OK)
        return;
}