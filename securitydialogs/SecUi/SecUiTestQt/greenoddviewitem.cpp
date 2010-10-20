#include "greenoddviewitem.h"

#include "QPainter"
#include "QStyleOptionGraphicsItem"
#include "QWidget"
#include "QDebug"

GreenOddViewItem::GreenOddViewItem(QGraphicsItem* parent) : 
    HbListViewItem(parent)
{
}

GreenOddViewItem::~GreenOddViewItem()
{
}


HbAbstractViewItem*  GreenOddViewItem::createItem()
{
    return new GreenOddViewItem(*this);
}

bool GreenOddViewItem::canSetModelIndex(const QModelIndex &index) const
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

        //qDebug() << "GreenOddViewItem::canSetModelIndex: value" << firstValue;
        int value = firstValue.toInt();
        //int value = index.row();
        if (    value > 0
            &&  value % 2) {
            return true;
        }
    }
    return false;
}

void GreenOddViewItem::paint(   QPainter *painter,
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
        painter->fillRect( drawRect, Qt::darkGreen);
    }
}

int GreenOddViewItem::type() const
{
    return GreenOddViewItem::Type;
}



