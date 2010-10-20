#ifndef BROWNEVENVIEWITEM_H
#define BROWNEVENVIEWITEM_H

#include <hblistviewitem.h>

class QGraphicsItem;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QModelIndex;

class BrownEvenViewItem : public HbListViewItem
{
    public:
        explicit BrownEvenViewItem(QGraphicsItem* parent=0);
        virtual ~BrownEvenViewItem();

        enum { Type = Hb::ItemType_Last + 1002 };
        virtual int type() const;

        virtual HbAbstractViewItem* createItem();
        virtual bool canSetModelIndex(const QModelIndex &index) const;
        virtual void paint( QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget);
};

#endif // BROWNEVENVIEWITEM_H
