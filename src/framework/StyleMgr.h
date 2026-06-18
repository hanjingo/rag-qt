#ifndef STYLEMGR_H
#define STYLEMGR_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QTextStream>

class StyleMgr : public QObject
{
    Q_OBJECT

  public:
    explicit StyleMgr(QObject *parent = nullptr);
    ~StyleMgr();

    static StyleMgr *Instance();
    static QString   ParseFile(const QString &styleFile);

  private:
    static StyleMgr *m_stStyleMgrInst;
};

#endif // STYLEMGR_H