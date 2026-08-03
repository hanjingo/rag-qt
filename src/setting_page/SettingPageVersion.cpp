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
}

void SettingPageVersion::_initUI()
{
    _retranslate();
}

void SettingPageVersion::_retranslate()
{
    // TODO
}

void SettingPageVersion::_initConnections()
{
    connect(GrpcClient::instance(),
            &GrpcClient::signalLoginResp,
            this,
            &SettingPageVersion::_slotLoginResp);
}

void SettingPageVersion::_slotLoginResp(const int      errorCode,
                                        const int64_t  user_id,
                                        const QString &auth,
                                        const QString &account,
                                        const QString &lastLoginTime,
                                        const bool     isForceUpdate)
{
    qDebug() << "SettingPageVersion::_slotLoginResp enter";
    if(errorCode != ErrorCode::OK)
        return;
}