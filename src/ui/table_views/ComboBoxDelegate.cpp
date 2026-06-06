#include "ComboBoxDelegate.h"

ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

ComboBoxDelegate::ComboBoxDelegate(QStringList strList, QObject *parent)
    : QItemDelegate(parent)
{
    foreach(QString str, strList)
    {
        m_strListItem.append(str);
    }
}

QWidget *ComboBoxDelegate::createEditor(QWidget                    *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex          &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    ChoiceComboBox *pComboBox = new ChoiceComboBox(parent);
    pComboBox->setStyleSheet(
        "QComboBox{border-top-left-radius: 0px;border-top-right-radius: "
        "0px;border-bottom-left-radius: 0px;font: 16px "
        "微软雅黑"
        ";padding-left:100px;color:#333333 ;background-color: rgb(209, 216, 223);}\
                              QComboBox QAbstractItemView{outline: 0px solid gray;padding-left: 5px; border-top-left-radius: 0px;border-top-right-radius: 0px;border-bottom-left-radius: 10px;border-bottom-right-radius: 10px;font: 16px "
        "微软雅黑"
        ";background-color: rgb(255, 255, 255);selection-background-color: rgb(163, 200, 93);}\
                              QComboBox QAbstractItemView::item:selected{background-color: rgb(163, 200, 93);color: rgb(255, 255, 255);}\
                              QComboBox::drop-down{width: 35px;border-left: 1px solid #ffffff;}\
                              QComboBox::down-arrow{image: url(:/SVG/comBox_arrow.svg);height: 17px;width: 18px;}\
                              QComboBox QScrollBar:vertical{padding-top: 15px;padding-bottom: 15px;min-width: 26px;min-height: 100px;border-radius: 3px;}\
                              QComboBox QScrollBar::handle:vertical{border-radius: 3px;background-color: rgb(139, 139, 139);}\
                              QComboBox QScrollBar::add-page:vertical{border-image: url();}\
                              QComboBox QScrollBar::sub-page:vertical{border-image: url();}\
                              QComboBox QScrollBar::add-line:vertical{border-image: url(:/SVG/Arrow_down.svg);}\
                              QComboBox QScrollBar::sub-line:vertical{border-image: url(:/SVG/Arrow_up.svg);}");
    pComboBox->addItems(m_strListItem);
    pComboBox->setFrame(false);
    pComboBox->installEventFilter(const_cast<ComboBoxDelegate *>(this));
    return pComboBox;
}

void ComboBoxDelegate::setEditorData(QWidget           *editor,
                                     const QModelIndex &index) const
{
    QString    str       = index.model()->data(index).toString();
    QComboBox *pComboBox = static_cast<QComboBox *>(editor);
    int        iIndex    = pComboBox->findText(str);
    if(-1 == iIndex)
    {
        iIndex = 0;
    }
    pComboBox->setCurrentIndex(iIndex);
}

void ComboBoxDelegate::setModelData(QWidget            *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex  &index) const
{
    QComboBox *pComboBox = static_cast<QComboBox *>(editor);
    QString    str       = pComboBox->currentText();
    model->setData(index, str, Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget                    *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::SetComboBoxItem(QStringList strListItem)
{
    m_strListItem = strListItem;
}

int ComboBoxDelegate::GetComboBoxIndex(QString strText)
{
    int iReturn = 0;
    iReturn     = m_strListItem.indexOf(strText);
    return iReturn;
}

QString ComboBoxDelegate::GetComboBoxText(int iIndex)
{
    QString strReturn = QString();
    strReturn         = m_strListItem.at(iIndex);
    return strReturn;
}
