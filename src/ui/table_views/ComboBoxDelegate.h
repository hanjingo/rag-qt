#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QComboBox>
#include <QItemDelegate>

class ComboBoxDelegate : public QItemDelegate
{
  private:
    class ChoiceComboBox : public QComboBox
    {
      public:
        explicit ChoiceComboBox(QWidget *parent = nullptr)
            : QComboBox(parent)
        {
        }
    };


  public:
    ComboBoxDelegate(QObject *parent = nullptr);
    ComboBoxDelegate(QStringList, QObject *parent = nullptr);

  public:
    void    SetComboBoxItem(QStringList);
    int     GetComboBoxIndex(QString);
    QString GetComboBoxText(int);

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

  private:
    QStringList m_strListItem;
};

#endif // COMBOBOXDELEGATE_H
