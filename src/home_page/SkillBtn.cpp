#include <QFile>
#include <QDebug>
#include <QToolButton>
#include <QPainter>
#include <QIcon>
#include <QStyleOptionToolButton>

#include "SkillBtn.h"

SkillBtn::SkillBtn(QWidget *parent)
    : QToolButton(parent)
    , m_id(-1)
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

SkillBtn::~SkillBtn()
{
}

void SkillBtn::paintEvent(QPaintEvent *e)
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // layer1: draw background
    opt.text = "";
    opt.icon = QIcon(); // use default icon painting
    style()->drawControl(QStyle::CE_ToolButtonLabel, &opt, &painter, this);

    // layer2: draw the text (will be covered by the overlay icon, but we want
    //      it to be there for accessibility and tooltips)
    painter.setFont(font());
    painter.setPen(palette().color(QPalette::ButtonText));
    QTextOption textOpt;
    textOpt.setAlignment(Qt::AlignCenter);
    textOpt.setWrapMode(QTextOption::WordWrap);
    QRect textRect = rect().adjusted(5, 5, -5, -5); // add some padding
    painter.drawText(textRect, text(), textOpt);

    // layer3: draw the overlay icon based on the download progress
    QIcon overlayIcon;
    switch(m_state)
    {
        case State::WaitDownload:
            overlayIcon = QIcon(":/icons/download");
            break;
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

    QSize iconSize(30, 30);
    //// calculate the centered coordinates to overlap the icon center with the button center
    //int x = (width() - iconSize.width()) / 2;
    //int y = (height() - iconSize.height()) / 2;

    // calculate the top right coordinates to overlap the icon center with the button center
    int margin = 5;
    int x      = width() - iconSize.width() - margin;
    int y      = margin;

    QRect iconRect(x, y, iconSize.width(), iconSize.height());

    // draw the overlay icon
    overlayIcon.paint(&painter, iconRect, Qt::AlignCenter);
}

void SkillBtn::Resize(int w, int h)
{
    setFixedSize(w, h);
}

void SkillBtn::SetState(State state)
{
    if(m_state == state)
        return;

    switch(state)
    {
        case State::WaitDownload: {
            qDebug() << "SkillBtn state changed to WaitDownload, hash: "
                     << Hash();
            this->setCheckable(true);
        }
        break;
        case State::Downloading: {
            qDebug() << "SkillBtn state changed to Downloading, hash: "
                     << Hash();
            this->setCheckable(false); // disable the button while downloading
        }
        break;
        case State::Downloaded: {
            qDebug() << "SkillBtn state changed to Downloaded, hash: "
                     << Hash();
            this->setCheckable(true);
        }
        break;
        case State::Installing: {
            qDebug() << "SkillBtn state changed to Installing, hash: "
                     << Hash();
            this->setCheckable(false); // disable the button while installing
        }
        break;
        case State::Installed: {
            qDebug() << "SkillBtn state changed to Installed, hash: " << Hash();
            this->setCheckable(false); // disable the button when installed
        }
        break;
        default: {
            qDebug() << "SkillBtn state changed to Unknown, hash: " << Hash();
            this->setCheckable(true);
        }
        break;
    }

    m_state = state;
    emit SignalStateChanged(this, m_state);
    update(); // trigger a repaint to reflect the new state
}

void SkillBtn::_init()
{
    SetState(State::Unknown);
}

void SkillBtn::_refreshText()
{
    const QString timestampText =
        m_timestamp.isValid() ? m_timestamp.toString(Qt::TextDate) : "-";

    setText(QString("%1\n"
                    "%2\n"
                    "V%3\n"
                    "Publisher:%4\n"
                    "%5\n"
                    "⤓: %6")
                .arg(m_name.isEmpty() ? "-" : m_name)
                .arg(m_desc.isEmpty() ? "-" : m_desc)
                .arg(m_version.isEmpty() ? "-" : m_version)
                .arg(m_publisher.isEmpty() ? "-" : m_publisher)
                .arg(timestampText)
                .arg(m_downloadTimes));
}

void SkillBtn::UpdateDownloadProgress(int progress)
{
    qDebug() << "Update skill button " << m_hash << ", progress:" << progress;
    SetState(State::Downloading);
    m_progress = progress;
    // refresh the button display to show the new progress state
    update();

    // if download is complete, emit the downloaded signal
    if(progress >= 100)
        SetState(State::Downloaded);
}