#include "ChatBox.h"
#include "ui_ChatBox.h"

ChatBox::ChatBox(QWidget *parent)
    : ui(new Ui::ChatBox)
{
}

ChatBox::~ChatBox()
{
    delete ui;
}

QWidget *ChatBox::Init(QWidget *parent)
{
    auto wgt = new QWidget(parent);
    wgt->setStyleSheet("background-color: transparent;");
    ui->setupUi(wgt);
    return wgt;
}