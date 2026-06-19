#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <QObject>
#include <QtPlugin>
#include <QWidget>

#include "Bus.h"
#include "PluginInterface.h"

namespace Ui
{
class Example;
}

class Example : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "rag-qt.PluginInterface" FILE "exampleplugin.json")
    Q_INTERFACES(PluginInterface)

  public:
    explicit Example(QWidget *parent = nullptr);
    ~Example();

    QString  Id() override { return "example-v0.0.1"; }
    QString  Name() override { return "Example"; }
    QString  Icon() override { return "ExampleIcon.png"; }
    QString  Version() override { return "0.0.1"; }
    QWidget *Init(Bus *parent = nullptr) override;

  private slots:
    void _slotPing();

  private:
    Ui::Example *ui;
    Bus         *m_pBus;
};

#endif // EXAMPLE_H