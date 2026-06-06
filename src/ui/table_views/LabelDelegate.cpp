#include "LabelDelegate.h"

LabelDelegate::LabelDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *LabelDelegate::createEditor(QWidget                    *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex          &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    DataLabel *plineEdit = new DataLabel(parent);
    plineEdit->setAlignment(Qt::AlignHCenter);
    plineEdit->installEventFilter(const_cast<LabelDelegate *>(this));
    return plineEdit;
}

void LabelDelegate::setEditorData(QWidget           *editor,
                                  const QModelIndex &index) const
{
    QString str       = index.model()->data(index).toString();
    QLabel *pLineEdit = static_cast<QLabel *>(editor);
    pLineEdit->setText(str);
}

void LabelDelegate::setModelData(QWidget            *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex  &index) const
{
    QLabel *pLineEdit = static_cast<QLabel *>(editor);
    QString str       = pLineEdit->text();
    model->setData(index, str, Qt::EditRole);
}

void LabelDelegate::updateEditorGeometry(QWidget                    *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
