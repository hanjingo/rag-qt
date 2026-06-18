#ifndef CHATBOX_H
#define CHATBOX_H

#include <QObject>
#include <QtPlugin>
#include <QWidget>

#include "PluginInterface.h"

namespace Ui
{
class ChatBox;
}

class ChatBox : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "rag-qt.PluginInterface" FILE "chatboxplugin.json")
    Q_INTERFACES(PluginInterface)

  public:
    explicit ChatBox(QWidget *parent = nullptr);
    ~ChatBox();

    QString  Id() override { return "chatbox-v0.0.1"; }
    QString  Name() override { return "chatbox"; }
    QString  Icon() override { return "ChatBoxIcon.png"; }
    QString  Version() override { return "0.0.1"; }
    QWidget *Init(QWidget *parent = nullptr) override;

  private:
    Ui::ChatBox *ui;
};

#endif