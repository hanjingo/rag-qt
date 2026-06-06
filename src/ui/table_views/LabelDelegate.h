#ifndef LABELDELEGATE_H
#define LABELDELEGATE_H

#include <QFile>
#include <QLabel>
#include <QItemDelegate>

class LabelDelegate : public QItemDelegate
{
  private:
    class DataLabel : public QLabel
    {
      public:
        explicit DataLabel(QWidget *parent = nullptr)
            : QLabel(parent)
        {
        }
    };

  public:
    LabelDelegate(QObject *parent = nullptr);

  private:
    QWidget *createEditor(QWidget *,
                          const QStyleOptionViewItem &,
                          const QModelIndex &) const;
    void     setEditorData(QWidget *, const QModelIndex &) const;
    void
    setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const;
    void updateEditorGeometry(QWidget *,
                              const QStyleOptionViewItem &,
                              const QModelIndex &) const;
};

#endif // LABELDELEGATE_H
