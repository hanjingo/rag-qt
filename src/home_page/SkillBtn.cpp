#include <QFile>
#include "SkillBtn.h"

SkillBtn::SkillBtn(QWidget *parent)
    : QPushButton(parent)
{
    // init button
    _init();
}

SkillBtn::~SkillBtn()
{
}

void SkillBtn::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);
}

void SkillBtn::_init()
{
    connect(this,
            SIGNAL(SignalUpdateProgress(int)),
            this,
            SLOT(SlotUpdateProgress(int)));

    emit SignalUpdateProgress(-1);
}

void SkillBtn::SlotUpdateProgress(int progress)
{
    qDebug() << "Update skill button progress:" << progress;
    if(progress < 0)
    {
        auto pixmap = QPixmap(":/icons/download");
        setIcon(pixmap);
        return;
    }

    // TODO
}