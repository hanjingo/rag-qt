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
{
    // init button
    _init();
}

SkillBtn::~SkillBtn()
{
    disconnect(this,
               SIGNAL(SignalUpdateProgress(int)),
               this,
               SLOT(SlotUpdateProgress(int)));
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
    if(m_progress < 0)
        overlayIcon = QIcon(":/icons/download");
    else if(m_progress >= 0 && m_progress < 100)
        overlayIcon = QIcon(":/icons/downloading");
    else
        overlayIcon = QIcon(":/icons/downloaded");
    QSize iconSize(30, 30);

    // calculate the centered coordinates to overlap the icon center with the button center
    int   x = (width() - iconSize.width()) / 2;
    int   y = (height() - iconSize.height()) / 2;
    QRect iconRect(x, y, iconSize.width(), iconSize.height());

    // draw the overlay icon
    overlayIcon.paint(&painter, iconRect, Qt::AlignCenter);
}

void SkillBtn::Resize(int w, int h)
{
    setFixedSize(w, h);
}

void SkillBtn::_init()
{
    connect(this,
            SIGNAL(SignalUpdateProgress(int)),
            this,
            SLOT(SlotUpdateProgress(int)));

    emit SignalUpdateProgress(-1);
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

void SkillBtn::SlotUpdateProgress(int progress)
{
    qDebug() << "Update skill button progress:" << progress;
    m_progress = progress;
    // refresh the button display to show the new progress state
    update();
}