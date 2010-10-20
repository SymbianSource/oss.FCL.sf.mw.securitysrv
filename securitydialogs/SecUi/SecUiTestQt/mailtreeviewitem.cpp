#include "mailtreeviewitem.h"

#include <hbpushbutton.h>
#include <hbabstractitemview.h>
#include <hbtextitem.h>
#include <hbstyle.h>
#include <hblabel.h>
#include <hbanchorlayout.h>
#include <hbframedrawer.h>
#include <hbframeitem.h>
#include <hbicon.h>
#include <hbstyle_p.h>

#include <QPainter>
#include <QDebug>

/*!
    \class NmMessageListViewItem
    \brief list view item for message list view
*/

/*!
    Constructor
*/
MailTreeViewItem::MailTreeViewItem(QGraphicsItem *parent): 
    HbTreeViewItem(parent),
    mSender(0),
    mSubject(0),
    mTime(0),
    mDividerTitle(0),
    mNewMsgIcon(0),
    mFrom(0)
{
}

MailTreeViewItem::~MailTreeViewItem()
{
}


int MailTreeViewItem::type() const
{
    return MailTreeViewItem::Type;
}

HbAbstractViewItem *MailTreeViewItem::createItem()
{
    return new MailTreeViewItem(*this);
}

/*!
    boolean value indicating model index availability
*/
bool MailTreeViewItem::canSetModelIndex(const QModelIndex &index) const
{
    Q_UNUSED(index);
    // This item class can handle all items in message list
    return true;
}

/*!
    update child items
*/
void MailTreeViewItem::updateChildItems()
{
    // to create expand icon, if it doesn't exist
    HbTreeViewItem::updateChildItems();

    // Shared data with ModelFactory.
    int messageRole = Qt::UserRole+1;
    int dateRole = Qt::UserRole+2;

    mLayout = 0;
    setLayout(0);

    // Create layout
    mLayout = new HbAnchorLayout();
    setLayout(mLayout); // mLayout ownership is passed to QGraphicsWidget

    QVariant dateData = modelIndex().data(dateRole);
    QVariant messageData = modelIndex().data(messageRole);

    // Create fonts
    HbFontSpec primaryFont = HbFontSpec(HbFontSpec::Primary);
    HbFontSpec secondaryFont = HbFontSpec(HbFontSpec::Secondary);

    // Check whether item is message item or title divider
    // and set the layout accordingly
    if (messageData.isValid()) {
        QStringList stringList;
        if ( messageData.canConvert<QStringList>()) {
            stringList = messageData.toStringList();
        }

        if (stringList.count() < 3) {
            // to avoid crash
            return;
        }

        delete mDividerTitle;
        mDividerTitle = 0;

        if (!mTime) {
            mTime = new HbLabel();
        }
        mTime->setObjectName("ListViewItemMessageTime");
        mTime->setFontSpec(HbFontSpec(HbFontSpec::Primary));
        // Create subject label
        if (!mSubject) {
            mSubject = new HbLabel();
        }
        mSubject->setObjectName("ListViewItemMessageSubject");
        mSubject->setFontSpec(HbFontSpec(HbFontSpec::Secondary));

        if (!mFrom) {
            mFrom = new HbLabel(stringList.at(0));
        }
        mFrom->setObjectName("ListViewItemMessageSender");
        mFrom->setFontSpec(primaryFont);

        // Create default locale
        /*QLocale locale;
        mTime->setText(locale.toString(
                msgModelItem->metaData().sentTime().toLocalTime().time(),
                QLocale::ShortFormat));*/
        mTime->setPlainText(stringList.at(1));

       // mTime->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        mSubject->setPlainText(stringList.at(2));

        // Create new message icon
        if (!mNewMsgIcon) {
           HbFrameDrawer *drawer = new HbFrameDrawer(":/resources/qtg_fr_list_new_item", HbFrameDrawer::ThreePiecesVertical);
            drawer->setFillWholeRect(false);
            mNewMsgIcon = new HbFrameItem(drawer, this);
        }
        mNewMsgIcon->setObjectName("ListViewItemMessageIcon");

        static const int iconWidth = 8;

        // Set message item layout
        // Place new message icon to layout
        mLayout->setAnchor(mLayout, Hb::TopEdge, mNewMsgIcon, Hb::TopEdge, 0);
        mLayout->setAnchor(mLayout, Hb::LeftEdge, mNewMsgIcon, Hb::LeftEdge, 0);
        mLayout->setAnchor(mNewMsgIcon, Hb::RightEdge, mFrom, Hb::LeftEdge, 0);

        // Set from & subject 10 pixels from left
        mLayout->setAnchor(mLayout, Hb::TopEdge, mFrom, Hb::TopEdge, 0);
        mLayout->setAnchor(mLayout, Hb::LeftEdge, mFrom, Hb::LeftEdge, iconWidth);
        mLayout->setAnchor(mLayout, Hb::RightEdge, mSubject, Hb::RightEdge, iconWidth);

        mLayout->setAnchor(mLayout, Hb::LeftEdge, mSubject, Hb::LeftEdge, iconWidth);
        mLayout->setAnchor(mFrom, Hb::BottomEdge, mSubject, Hb::TopEdge, 0);
        // Set Time label to correct place
        mLayout->setAnchor(mTime, Hb::LeftEdge, mLayout, Hb::RightEdge, 100);
        mLayout->setAnchor(mFrom, Hb::RightEdge, mTime, Hb::LeftEdge, 0);
        // Set subject right edge alignment
        mLayout->setAnchor(mSubject, Hb::RightEdge, mLayout, Hb::RightEdge, iconWidth);

        mLayout->setPreferredHeight(50);
        setObjectName("ListViewItemMessage");
    }
    else if (dateData.isValid()) {
        QString date;
        if (dateData.canConvert<QString>()) {
            date = dateData.toString();
        }
        else {
            return;
        }

        // NOTE: Layout data will be read from xml once orbit supports it
        // Create divider icon
        delete mTime;
        mTime = 0;

        delete mSubject;
        mSubject = 0;

        delete mNewMsgIcon;
        mNewMsgIcon = 0;

        delete mFrom;
        mFrom = 0;

        // Create divider title
        if (!mDividerTitle) {
            mDividerTitle = new HbLabel();
        }
        mDividerTitle->setObjectName("ListViewItemDividerTitle");

        mDividerTitle->setPlainText(date);
        mDividerTitle->setFontSpec(primaryFont);
        mDividerTitle->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        // Add title divider text
        mLayout->setAnchor(mLayout, Hb::TopEdge, mDividerTitle, Hb::TopEdge, 1);
        mLayout->setAnchor(mLayout, Hb::LeftEdge, mDividerTitle, Hb::LeftEdge, 0);

        QGraphicsItem *graphicsItem = primitive(QLatin1String("subitem-indicator"));
        if (    graphicsItem 
            &&  graphicsItem->isWidget()) {
            HbLabel *dividerIcon = qobject_cast<HbLabel*>(static_cast<QGraphicsWidget*>(graphicsItem));
            if (dividerIcon) {
                mLayout->setAnchor(mLayout, Hb::TopEdge, dividerIcon, Hb::TopEdge, 5);
                mLayout->setAnchor(dividerIcon, Hb::RightEdge, mLayout, Hb::RightEdge, 20);
            }
        }

        mLayout->setPreferredHeight(32);
        setObjectName("ListViewItemDivider");
    }
    else {
        qDebug() <<"MailTreeViewItem: Invalid message meta data when drawing message list";
    }

    const QSizeF reso = HbDeviceProfile::current().logicalSize();
    mLayout->setPreferredWidth(reso.width()-20);
}

void MailTreeViewItem::polishEvent()
{
    QGraphicsWidget::polishEvent();
}


HbWidgetBase *MailTreeViewItem::updateExpandItem()
{
    HbLabel *dividerIcon = 0;
    QGraphicsItem *graphicsItem = primitive(QLatin1String("subitem-indicator"));
    if (    graphicsItem 
        &&  graphicsItem->isWidget()) {
        dividerIcon = qobject_cast<HbLabel*>(static_cast<QGraphicsWidget*>(graphicsItem));
    }

    if (!dividerIcon) {
        dividerIcon = new HbLabel();
        HbIcon icon;
        icon.setIconName(":/resources/qtg_nmailui_minus_sign", QIcon::Normal, QIcon::On);
        icon.setIconName(":/resources/qtg_nmailui_plus_sign", QIcon::Normal, QIcon::Off);
        dividerIcon->setIcon(icon);
    }

    if (isExpanded()) {
        dividerIcon->setObjectName("ListViewItemDividerIconMinus");
    }
    else {
        dividerIcon->setObjectName("ListViewItemDividerIconPlus");
    }
    return dividerIcon;
}

void MailTreeViewItem::paint(
    QPainter *painter,
    const QStyleOptionGraphicsItem *option,
    QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (painter){
        painter->save();
        painter->setOpacity(0.15);
        QLineF line1( rect().topLeft().x(), rect().bottomRight().y(),
                     rect().bottomRight().x(), rect().bottomRight().y());
        painter->drawLine(line1);
        // Draw line before each item
        QLineF line2( rect().topLeft().x(), rect().topLeft().y(),
                     rect().bottomRight().x(), rect().topLeft().y());
        painter->drawLine(line2);
        painter->restore();
    }
}


