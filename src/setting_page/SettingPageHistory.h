#ifndef SETTINGPAGEHISTORY_H
#define SETTINGPAGEHISTORY_H

#include <QMap>
#include <QWidget>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>

#include "Bus.h"

namespace Ui
{
class SettingPageHistory;
}

class SettingPageHistory : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageHistory *Instance();
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

    void _slotGetMessageInfoResp(const int                        errorCode,
                                 const QVector<Bus::MessageInfo> &messages);

    void _slotTbviewCurrentChanged(const QModelIndex &curr,
                                   const QModelIndex &prev);

    void _slotBtnDelSessionClicked();

    void _slotBtnExportSessionClicked();

    void _slotBtnImportSessionClicked();

    void _slotBtnSearchClicked();

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _addSessions(const QVector<Bus::Session> &sessions);
    void _refreshHistoryTable(bool clearFirst = false);
    void _addMessages(const QVector<Bus::MessageInfo> &messages);
    void _refreshChatBrowser(bool clearFirst = true);

  private:
    void _addQueryRecord(const QString &query, const QString &timestamp);
    void _addAnswerRecord(const QString &answer, const QString &timestamp);

  private:
    Ui::SettingPageHistory    *ui;
    static SettingPageHistory *m_stSettingPageHistoryInst;

    QStandardItemModel *m_pHistoryModel;
    QSet<qint64>        m_setRecvedMsgIds;
};

#endif // SETTINGPAGEHISTORY_H
