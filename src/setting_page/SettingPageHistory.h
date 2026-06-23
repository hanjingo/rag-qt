#ifndef SETTINGPAGEHISTORY_H
#define SETTINGPAGEHISTORY_H

#include <QMap>
#include <QWidget>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "Bus.h"

namespace Ui
{
class SettingPageHistory;
}

class SettingPageHistory : public QWidget
{
    Q_OBJECT

  public:
    static SettingPageHistory *GetSettingPageHistoryInst();
    explicit SettingPageHistory(QWidget *parent = nullptr);
    ~SettingPageHistory();

  signals:

  private slots:
    void _slotNewSessionResp(const int errorCode, const Bus::Session &session);
    void _slotGetSessionResp(const int                    errorCode,
                             const QVector<Bus::Session> &sessions);

    void _slotGetMessageInfoResp(const int                        errorCode,
                                 const QVector<Bus::MessageInfo> &messages);

    void _slotTbviewCurrentChanged(const QModelIndex &curr,
                                   const QModelIndex &prev);

  private:
    void _initUI();
    void _initConnections();
    void _retranslate();
    void _addSessions(const QVector<Bus::Session> &sessions);
    void _refreshHistoryTable(bool clearFirst = false);
    void _refreshChatBrowser(bool clearFirst = true);

  private:
    void _addQueryRecord(const QString &query, const QString &timestamp);
    void _addAnswerRecord(const QString &answer, const QString &timestamp);

  private:
    Ui::SettingPageHistory    *ui;
    static SettingPageHistory *m_stSettingPageHistoryInst;

    QStandardItemModel *m_pHistoryModel;
};

#endif // SETTINGPAGEHISTORY_H
