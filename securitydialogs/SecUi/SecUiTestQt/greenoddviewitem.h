#ifndef GREENODDVIEWITEM_H
#define GREENODDVIEWITEM_H

#include <hblistviewitem.h>

class QGraphicsItem;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QModelIndex;

class GreenOddViewItem : public HbListViewItem
{
    public:
        explicit GreenOddViewItem(QGraphicsItem* parent=0);
        virtual ~GreenOddViewItem();

        enum { Type = Hb::ItemType_Last + 1001 };
        virtual int type() const;

        virtual HbAbstractViewItem* createItem();
        virtual bool canSetModelIndex(const QModelIndex &index) const;
        virtual void paint( QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget);
};

#endif // GREENODDVIEWITEM_H
