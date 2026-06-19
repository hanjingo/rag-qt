#include "Example.h"
#include "ui_Example.h"

Example::Example(QWidget *parent)
    : ui(new Ui::Example)
{
}

Example::~Example()
{
    delete ui;
}

QWidget *Example::Init(Bus *parent)
{
    m_pBus = parent;

    // init connect
    connect(m_pBus, &Bus::SignalPing, this, &Example::_slotPing);

    // create UI
    auto wgt = new QWidget(nullptr);
    wgt->setStyleSheet("background-color: transparent;");
    ui->setupUi(wgt);
    return wgt;
}

void Example::_slotPing()
{
    qDebug() << "Example received Ping signal from Bus.";
    emit m_pBus->SignalPong();
}