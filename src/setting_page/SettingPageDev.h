#ifndef SettingPageDev_H
#define SettingPageDev_H

#include <QWidget>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
class QButtonGroup;
QT_END_NAMESPACE

namespace Ui
{
class SettingPageDev;
}

class SettingPageDev : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageDev *Instance()
    {
        static SettingPageDev inst;
        return &inst;
    }

  protected:
    explicit SettingPageDev(QWidget *parent = nullptr);
    ~SettingPageDev();

    void _initUI();
    void _initConnections();
    void _retranslate();

    void _refreshPluginTable(bool clearFirst);
    void _addPluginRecords(const QVector<Bus::Plugin> &plugins,
                           const QString              &tag = "");
    void _delPluginRecords(const QVector<QString> &hashs);
    void _clearPluginRecords();
    void _filtePluginTable(const QString &filterText);

  private slots:
    void _slotCatalogChanged(int index);
    void _slotGetPluginInfoResp(const int                   errorCode,
                                const QVector<Bus::Plugin> &plugins);

    void _slotEditFilterPluginTextChanged(const QString &content);

  private:
    Ui::SettingPageDev *ui;

    QStandardItemModel *m_pPluginListModel;
};

#endif // SettingPageDev_H
