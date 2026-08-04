#include <QtMath>
#include <QPainter>
#include <QAction>
#include <QMessageBox>

#include "StyleMgr.h"

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
    _retranslate();
}

SettingPageVersion::~SettingPageVersion()
{
}

void SettingPageVersion::_initUI()
{
    // Keep the page/container transparent and only show the login panel card.
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("QWidget#LoginPage{background: transparent;}"
                  "QWidget#wgtLogin{background: transparent;}");

    ui->lblLogo->setPixmap(QPixmap(":/icons/logo"));
    ui->lblTitle->setStyleSheet(StyleMgr::ParseFile(":/styles/title_label"));

    ui->lblEmailTitle->setStyleSheet(
        StyleMgr::ParseFile(":/styles/note_label"));
    ui->lblEmail->setStyleSheet(StyleMgr::ParseFile(":/styles/note_label"));

    ui->lblVersionTitle->setStyleSheet(
        StyleMgr::ParseFile(":/styles/note_label"));
    ui->lblVersion->setStyleSheet(StyleMgr::ParseFile(":/styles/note_label"));
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