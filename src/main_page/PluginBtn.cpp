#include <QFile>
#include <QDir>
#include <QDebug>
#include <QToolButton>
#include <QPainter>
#include <QIcon>
#include <QStyleOptionToolButton>

#include "PluginBtn.h"
#include "StyleMgr.h"

PluginBtn::PluginBtn(QWidget *parent)
    : QToolButton(parent)
    , m_name("")
    , m_desc("")
    , m_publisher("")
    , m_version("")
    , m_timestamp(QDateTime())
    , m_hash("")
    , m_downloadTimes(0)
    , m_progress(-1)
    , m_state(State::Unknown)
{
    // init button
    _init();
}

PluginBtn::~PluginBtn()
{
}

void PluginBtn::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QString curState = underMouse() ? "hover" : "normal";
    if(property("state").toString() != curState)
    {
        setProperty("state", curState);
        setProperty("theme", "glass");
        style()->unpolish(this);
        style()->polish(this);
    }

    setStyleSheet(StyleMgr::ParseFile(":/styles/plugin_btn"));

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QRect fillRect = rect().adjusted(2, 2, -2, -2);

    int radius  = 16;
    int padding = 14;
    if(rect().width() > 0)
    {
        // TODO:
    }

    QColor baseBgColor =
        underMouse() ? QColor(255, 255, 255, 255) : QColor(255, 255, 255, 235);
    QColor baseBorderColor = QColor(255, 255, 255, 240);
    QColor mainTextColor   = QColor(17, 17, 17);

    QColor bgGradientStart = baseBgColor;
    QColor bgGradientEnd =
        underMouse() ? QColor(255, 255, 255, 220) : QColor(255, 255, 255, 195);
    QColor borderColorTop = baseBorderColor;
    QColor borderColorBottom =
        underMouse() ? QColor(0, 0, 0, 40) : QColor(0, 0, 0, 30);

    QColor titleColor = mainTextColor;
    QColor descColor  = QColor(mainTextColor.red() + 33,
                               mainTextColor.green() + 33,
                               mainTextColor.blue() + 33);
    QColor infoColor  = QColor(mainTextColor.red() + 83,
                               mainTextColor.green() + 83,
                               mainTextColor.blue() + 83);

    painter.save();
    QColor shadowColor(10, 20, 40, 30);
    for(int i = 1; i <= 4; ++i)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadowColor);
        painter.drawRoundedRect(fillRect.adjusted(-i / 2, i, i / 2, i),
                                radius,
                                radius); //[cite: 19]
    }
    painter.restore();

    QLinearGradient bgGradient(fillRect.topLeft(),
                               fillRect.bottomLeft()); //[cite: 19]
    bgGradient.setColorAt(0.0, bgGradientStart);       //[cite: 19]
    bgGradient.setColorAt(1.0, bgGradientEnd);         //[cite: 19]

    painter.setPen(Qt::NoPen);
    painter.setBrush(bgGradient);
    painter.drawRoundedRect(fillRect, radius, radius); //[cite: 19]

    QPen borderPen;
    borderPen.setWidthF(1.2);
    QLinearGradient borderGradient(fillRect.topLeft(),
                                   fillRect.bottomLeft()); //[cite: 19]
    borderGradient.setColorAt(0.0, borderColorTop);        //[cite: 19]
    borderGradient.setColorAt(1.0, borderColorBottom);     //[cite: 19]

    borderPen.setBrush(borderGradient);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(fillRect, radius, radius); //[cite: 19]

    QFont nameFont = font();
    nameFont.setBold(true);
    nameFont.setPixelSize(15);
    painter.setFont(nameFont);
    painter.setPen(titleColor);
    QRect nameRect(fillRect.left() + padding,
                   fillRect.top() + padding,
                   fillRect.width() - padding * 2 - 35,
                   22);
    painter.drawText(nameRect,
                     Qt::AlignLeft | Qt::AlignVCenter,
                     m_name.isEmpty() ? "-" : m_name);

    QFont infoFont = font();
    infoFont.setPixelSize(11);
    painter.setFont(infoFont);
    painter.setPen(infoColor);
    QString botText = QString("V%1 · %2")
                          .arg(m_version.isEmpty() ? "0.0" : m_version)
                          .arg(m_publisher.isEmpty() ? "admin" : m_publisher);
    QRect   botRect(fillRect.left() + padding,
                    fillRect.bottom() - padding - 15,
                    fillRect.width() - padding * 2,
                    18);
    painter.drawText(botRect, Qt::AlignLeft | Qt::AlignVCenter, botText);

    QFont descFont = font();
    descFont.setPixelSize(12);
    painter.setFont(descFont);
    painter.setPen(descColor);

    int   descTop    = nameRect.bottom() + 6;
    int   descBottom = botRect.top() - 6;
    QRect descRect(fillRect.left() + padding,
                   descTop,
                   fillRect.width() - padding * 2,
                   descBottom - descTop);

    QTextOption descOpt;
    descOpt.setWrapMode(QTextOption::WordWrap);
    descOpt.setAlignment(Qt::AlignLeft | Qt::AlignTop);

    painter.drawText(descRect, m_desc.isEmpty() ? "-" : m_desc, descOpt);

    QSize iconSize(26, 26);
    int   margin = 12;
    int   iconX  = fillRect.right() - iconSize.width() - margin;
    int   iconY  = fillRect.top() + margin;
    QRect iconRect(iconX, iconY, iconSize.width(), iconSize.height());

    QIcon overlayIcon;
    switch(m_state)
    {
        case State::Downloading:
            overlayIcon = QIcon(":/icons/downloading");
            break;
        case State::Downloaded:
            overlayIcon = QIcon(":/icons/downloaded");
            break;
        case State::Installing:
            overlayIcon = QIcon(":/icons/installing");
            break;
        case State::Installed:
            overlayIcon = QIcon(":/icons/installed");
            break;
        default:
            overlayIcon = QIcon(":/icons/download");
            break;
    }

    if(m_state == State::Downloading && m_progress >= 0)
    {
        QPen trackPen(QColor(0, 0, 0, 25), 2);
        painter.setPen(trackPen);
        painter.drawEllipse(iconRect.adjusted(-2, -2, 2, 2));

        QPen progressPen(QColor(0, 122, 255), 2);
        progressPen.setCapStyle(Qt::RoundCap);
        painter.setPen(progressPen);

        int startAngle = 90 * 16;
        int spanAngle  = -((m_progress * 360) / 100) * 16;
        painter.drawArc(iconRect.adjusted(-2, -2, 2, 2), startAngle, spanAngle);
    }

    overlayIcon.paint(&painter, iconRect, Qt::AlignCenter);
}

void PluginBtn::Resize(int w, int h)
{
    setFixedSize(w, h);
}

void PluginBtn::SetState(State state)
{
    if(m_state == state)
        return;

    switch(state)
    {
        case State::Unknown: {
            qDebug() << "PluginBtn state changed to Unknown, hash: " << Hash();
            this->setCheckable(true);
        }
        break;
        case State::WaitDownload: {
            qDebug() << "PluginBtn state changed to WaitDownload, hash: "
                     << Hash();
            this->setCheckable(false);
        }
        break;
        case State::Downloading: {
            qDebug() << "PluginBtn state changed to Downloading, hash: "
                     << Hash();
            this->setCheckable(false); // disable the button while downloading
        }
        break;
        case State::Downloaded: {
            qDebug() << "PluginBtn state changed to Downloaded, hash: "
                     << Hash();
            this->setCheckable(false);
        }
        break;
        case State::Installing: {
            qDebug() << "PluginBtn state changed to Installing, hash: "
                     << Hash();
            this->setCheckable(false); // disable the button while installing
        }
        break;
        case State::Installed: {
            qDebug() << "PluginBtn state changed to Installed, hash: "
                     << Hash();
            this->setCheckable(false); // disable the button when installed
        }
        break;
        default:
            break;
    }

    m_state = state;
    emit signalStateChanged(this, m_state);
    update(); // trigger a repaint to reflect the new state
}

void PluginBtn::_init()
{
    this->setCheckable(true);
    SetState(State::Unknown);
}

void PluginBtn::_refreshText()
{
    // const QString timestampText =
    //     m_timestamp.isValid() ? m_timestamp.toString(Qt::TextDate) : "-";

    // setText(QString("%1\n"
    //                 "%2\n"
    //                 "V%3\n"
    //                 "Publisher:%4\n"
    //                 "%5\n"
    //                 "⤓: %6")
    //             .arg(m_name.isEmpty() ? "-" : m_name)
    //             .arg(m_desc.isEmpty() ? "-" : m_desc)
    //             .arg(m_version.isEmpty() ? "-" : m_version)
    //             .arg(m_publisher.isEmpty() ? "-" : m_publisher)
    //             .arg(timestampText)
    //             .arg(m_downloadTimes));

    update();
}

void PluginBtn::slotProgressChanged(int progress)
{
    qDebug() << "Update plugin button " << m_hash << ", progress:" << progress;
    SetState(State::Downloading);
    m_progress = progress;
    // refresh the button display to show the new progress state
    update();
}

void PluginBtn::slotProgressFinished(bool success)
{
    m_downloader->deleteLater();
    m_downloader = nullptr;
    if(success)
    {
        qDebug() << "Download finished successfully for plugin button "
                 << m_hash;
        m_progress = 100;
        SetState(State::Downloaded);
    } else
    {
        qDebug() << "Download failed for plugin button " << m_hash;
        m_progress = -1; // reset progress on failure
        SetState(State::Unknown);
    }
    update();
}

void PluginBtn::Download(const QUrl &url, const QString &saveFilePath)
{
    if(saveFilePath.isEmpty() || url.isEmpty())
    {
        qDebug()
            << "Invalid URL or save file path for downloading plugin content.";
        SetState(PluginBtn::State::Unknown);
        return;
    }

    if(m_downloader)
    {
        m_downloader->deleteLater(); // clean up any existing downloader
        m_downloader = nullptr;
    }

    m_downloader = new Downloader(this);
    m_progress   = -1;
    // m_url        = url;
    qDebug() << "Started downloading plugin content from " << url.toString()
             << " to " << saveFilePath;
    connect(m_downloader,
            &Downloader::signalDownloadProgress,
            this,
            &PluginBtn::slotProgressChanged);

    connect(m_downloader,
            &Downloader::signalDownloadFinished,
            this,
            &PluginBtn::slotProgressFinished);

    SetState(PluginBtn::State::Downloading);
    m_downloader->Download(url, saveFilePath);
}

void PluginBtn::Reset()
{
    qDebug() << "Resetting plugin button " << m_hash;
    SetState(State::Unknown);
    m_progress = -1;
    m_url      = QString();
}