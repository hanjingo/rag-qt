#ifndef SETTINGPAGEHISTORY_H
#define SETTINGPAGEHISTORY_H

#include <QMap>
#include <QWidget>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include <QPointer>

#include "Bus.h"

namespace Ui
{
class SettingPageHistory;
}

class SettingPageHistory : public QWidget
{
    Q_OBJECT

  public:
    static QPointer<SettingPageHistory> instance()
    {
        static QPointer<SettingPageHistory> inst = new SettingPageHistory();
        return inst;
    }
    explicit SettingPageHistory(QWidget *parent = nullptr);
    ~SettingPageHistory();

  protected:
    void changeEvent(QEvent *event) override;

  signals:

  private slots:
    void _slotNewSessionResp(const int errorCode, const Bus::Session &session);
    void _slotGetSessionResp(const int                    errorCode,
                             const QVector<Bus::Session> &sessions);
    void _slotDelSessionResp(const int errorCode, const QVector<int64_t> &ids);

    void _slotGetChatMessageResp(const int                        errorCode,
                                 const QVector<Bus::ChatMessage> &messages);

    void _slotTbviewClicked(const QModelIndex &curr);

    void _slotBtnDelSessionClicked();

    void _slotBtnExportSessionClicked();

    void _slotBtnImportSessionClicked();

    void _slotBtnSearchClicked();

    void _slotBtnCatalogCtlClicked();

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _addSessions(const QVector<Bus::Session> &sessions);
    void _refreshHistoryTable(bool clearFirst = false);
    void _addMessages(const QVector<Bus::ChatMessage> &messages);
    void _refreshChatBrowser(bool clearFirst = true);

  private:
    void _addQueryRecord(const QString &query, const QString &timestamp);
    void _addAnswerRecord(const QString &answer, const QString &timestamp);

  private:
    Ui::SettingPageHistory *ui;

    QStandardItemModel *m_pHistoryModel;
    QSet<qint64>        m_setRecvedMsgIds;
};

#endif // SETTINGPAGEHISTORY_H
