#ifndef STYLEMGR_H
#define STYLEMGR_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QPointer>
#include <QTextStream>

class StyleMgr : public QObject
{
    Q_OBJECT

  public:
    explicit StyleMgr(QObject *parent = nullptr);
    ~StyleMgr();

    static QPointer<StyleMgr> instance()
    {
        static QPointer<StyleMgr> inst = new StyleMgr();
        return inst;
    }
    static QString ParseFile(const QString &styleFile);
};

#endif // STYLEMGR_H