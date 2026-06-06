#ifndef AUDIOPAGEWIDGET_H
#define AUDIOPAGEWIDGET_H

#include <QMap>
#include <QWidget>

namespace Ui
{
class AudioPageWidget;
}

class AudioPageWidget : public QWidget
{
    Q_OBJECT

  public:
    static AudioPageWidget *GetAudioPageWidgetInst();
    explicit AudioPageWidget(QWidget *parent = nullptr);
    ~AudioPageWidget();

  signals:

  private slots:

  private:
    Ui::AudioPageWidget    *ui;
    static AudioPageWidget *m_stAudioPageWidgetInst;
};

#endif // AUDIOPAGEWIDGET_H
