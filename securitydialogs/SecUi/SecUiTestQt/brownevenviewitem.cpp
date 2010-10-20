#include "brownevenviewitem.h"

#include "QPainter"
#include "QStyleOptionGraphicsItem"
#include "QWidget"
#include "QDebug"

BrownEvenViewItem::BrownEvenViewItem(QGraphicsItem* parent) : 
    HbListViewItem(parent)
{
}

BrownEvenViewItem::~BrownEvenViewItem()
{
}


HbAbstractViewItem*  BrownEvenViewItem::createItem()
{
    return new BrownEvenViewItem(*this);
}

bool BrownEvenViewItem::canSetModelIndex(const QModelIndex &index) const
{
    int itemType(index.data(Hb::ItemTypeRole).toInt()); 
    if (itemType == Hb::StandardItem) {
        QVariant displayRole = index.data(Qt::DisplayRole);
        QString firstValue;
        QStringList stringList;
        if (displayRole.isValid()) {
            if (displayRole.canConvert<QString>()) {
                firstValue = displayRole.toString();
            } else if (     displayRole.canConvert<QStringList>()
                        &&  displayRole.toStringList().count()) {
                firstValue = displayRole.toStringList().at(0);
            }
        }

        //qDebug() << "BrownEvenViewItem::canSetModelIndex: value" << firstValue;
        //int value = index.row();
        int value = firstValue.toInt();
        if (    value > 0
            &&  value % 2 == 0) {
            return true;
        }
    }
    return false;
}

void BrownEvenViewItem::paint(   QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                QWidget *widget)
{
    HbAbstractViewItem::paint( painter, option, widget );
    if (painter){
        QRectF drawRect = boundingRect();
        drawRect.adjust(1,1,-1,-1);
        //QColor brown(141,92,7);
        //QColor brown(106,57,10);
        QColor brown(87,45,11);
        painter->fillRect( drawRect, brown);
    }
}

int BrownEvenViewItem::type() const
{
    return BrownEvenViewItem::Type;
}


