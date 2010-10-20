/*
* ====================================================
*  Name        : viewfutedataform.cpp
*  Part of     : fute/listdemo
*  Description : Settings for listdemo, treedemo, griddemo
*  Version     : %version: 1 %
*
*  Copyright (c) 2009 Nokia.  All rights reserved.
*  This material, including documentation and any related computer
*  programs, is protected by copyright controlled by Nokia.  All
*  rights are reserved.  Copying, including reproducing, storing,
*  adapting or translating, any or all of this material requires the
*  prior written consent of Nokia.  This material also contains
*  confidential information which may not be disclosed to others
*  without the prior written consent of Nokia.
* ====================================================
*/

#include "viewfutedataform.h"
#include <hbdataformmodel.h>
#include <hbabstractviewitem.h>
#include <hblistviewitem.h>
#include <hblistview.h>
#include <hbtreeview.h>
#include <hbgridview.h>
#include <hbscrollbar.h>
#include <hbframebackground.h>
#include <hbaction.h>
#include <hbmenu.h>
#include <hblineedit.h>
#include <hbinputeditorinterface.h>
#include <hblistwidgetitem.h>
#include <hbabstractitemview.h>
#include <hbindexfeedback.h>

#include <QStandardItem>
#include <QGraphicsScene>
#include <QAbstractItemView>

Q_DECLARE_METATYPE(QList<int>)

// Settings
const QStringList TextStyles = (QStringList() << "Primary" << "Secondary");
const QStringList CustomWidgets = (QStringList() << "Empty" << "Zoom slider" << "Volume Slider" << "Progress bar"  << "Button" << "Text Editor" );
const QStringList OnOff = (QStringList() << "On" << "Off");
const QStringList LaunchInPopups = (QStringList() << "Launch into popup" << "Launch into view") ;
const QStringList SelectionModes = (QStringList() << "Single selection" << "Multiple selection" << "No selection");
const QStringList ScrollModes = (QStringList() << "Pan" << "Pan with follow on");
const QStringList GraphicsSizes = (QStringList() << "Small icon" << "Medium icon" << "Large icon" << "Thumbnail" << "Wide thumbnail");
const QStringList LandscapeModes = (QStringList() << "Normal" << "Stretched");
const QStringList OrientationTypes  = (QStringList() << "Portrait" << "Landscape");
const QStringList ScrollHints  = (QStringList() << "Just visible" << "At top" << "At bottom" << "At center" );
const QStringList IndexFeedbackPolicies = (QStringList() << "None" << "One Char" << "Three Char" << "String" << "detatch" );
const QStringList IconLoadPolicies = (QStringList() << "Synchronous" << "Asynchronous when scrolling" << "Asynchronous" );

// Add item
const QStringList LeftColumnWidgets = (QStringList() << "Empty" << "Icon");
const QStringList RightColumnWidgets = (QStringList() << "Empty" << "Icon");
const QStringList ItemBackgrounds = (QStringList() << "None" << "Red" << "Green" << "Blue" << "Bitmap" << "Frame");
const QStringList ItemTypes = (QStringList() << "Normal" << "Separator");
const QStringList AnimationList = (QStringList() << "Appear" << "Disappear" << "TouchDown");

const QStringList KConvenienceIcons = (QStringList() 
    << ":/demo/list1"
);

ViewFuteDataForm::ViewFuteDataForm( HbAbstractItemView &view,
                                   QGraphicsItem *parent):
  HbDataForm(parent),
  activity(None),
  view(&view),
  item(0),
  mIndexFeedback(0)
{
    settingsFormModel = new HbDataFormModel();
    initialise();
}

ViewFuteDataForm::~ViewFuteDataForm()
{
    delete settingsFormModel;
}

void ViewFuteDataForm::populateSettings()
{
    activity = Settings;
    settingsFormModel->clear();

    counter += populateCustomSettingsItem(-1);

    //populateLaunchInPopup();
    //counter += populateCustomSettingsItem(LaunchInPopup);

    populateSelectionMode();
    counter += populateCustomSettingsItem(SelectionMode);

    populateScrollHint();
    counter += populateCustomSettingsItem(ScrollHint);

    populateRecycling();
    counter += populateCustomSettingsItem(Recycling);

    populateArrangeMode();
    counter += populateCustomSettingsItem(ArrangeMode);

    populateUniformItem();
    counter += populateCustomSettingsItem(UniformItem);

    populateFrame();
    counter += populateCustomSettingsItem(Frame);

    populateRichText();
    counter += populateCustomSettingsItem(RichText);

    populateScrollingStyle();
    counter += populateCustomSettingsItem(ScrollingStyle);

    populateInteractiveScrollBar();
    counter += populateCustomSettingsItem(InteractiveScrollBar);

    populateClampingStyle();
    counter += populateCustomSettingsItem(ClampingStyle);

    populateFrictionEnabled();
    counter += populateCustomSettingsItem(FrictionEnabled);

    populateGraphicsSize();
    counter += populateCustomSettingsItem(GraphicsSize);

    populateStretchingStyle();
    counter += populateCustomSettingsItem(StretchingStyle);

    populateMaximumRowCount();
    counter += populateCustomSettingsItem(MaximumRowCount);

    populateMinimumRowCount();
    counter += populateCustomSettingsItem(MinimumRowCount);

    //populateMirroring();
    //counter += populateCustomSettingsItem(Mirroring);

    populateIndexFeedbackPolicy();
    counter += populateCustomSettingsItem(IndexFeedbackPolicy);
/*
    populateEnabledAnimations();
    counter += populateCustomSettingsItem(Animations);
*/
    populatePixmapCacheEnabled();
    counter += populateCustomSettingsItem(PixmapCache);

    populateIconLoadPolicy();
    counter += populateCustomSettingsItem(IconLoadPolicy);

    setModel(settingsFormModel);
}

void ViewFuteDataForm::setInputData(    Activity    action,
                                        int         item, 
                                        QVariant    data)
{
    if (    item < 0
        ||  action == None
        ||  (   (   action == Settings
                &&  item >= SettingsIndexLast) 
            ||  (   action == AddItem
            &&  item >= AddItemIndexLast))) {
        return;
    }

    if (action == Settings) {
        settingsData[item] = data;
        data.isValid();
    }
    else if (action == AddItem) {
        addItemData[item] = data;
    }
}


int ViewFuteDataForm::populateCustomSettingsItem(int previousItem)
{
    Q_UNUSED(previousItem);
    return 0;
}

void ViewFuteDataForm::initialise()
{
    settingsFormModel->clear();
    counter=0;
    for (int i=0; i< SettingsIndexLast; i++) {
        settingsIndexes[i] = -1;
    }
    for (int i=0; i< AddItemIndexLast; i++) {
        addItemIndexes[i] = -1;
    }
    for (int i=0; i< SettingsIndexLast; i++) {
        settingsData[i] = QVariant();
    }
    for (int i=0; i< AddItemIndexLast; i++) {
        addItemData[i] = QVariant();
    }
}

int ViewFuteDataForm::nextIndex() const
{
    return counter;
}

ViewFuteDataForm::Activity ViewFuteDataForm::action() const
{
    return activity;
}


HbDataFormModel *ViewFuteDataForm::dataModel() const
{
    return settingsFormModel;
}

void ViewFuteDataForm::populateGraphicsSize()
{
    HbListViewItem *prototype = qobject_cast<HbListViewItem *>(view->itemPrototypes().first());
    if ( prototype ) {
        settingsIndexes[GraphicsSize] = counter++;
        
        HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(HbDataFormModelItem::RadioButtonListItem, 
            QString("Graphics size:"));

        item->setContentWidgetData("items", GraphicsSizes);

        HbListViewItem::GraphicsSize graphicsSize = prototype->graphicsSize();
        if (graphicsSize == HbListViewItem::SmallIcon) {
            item->setContentWidgetData("selected", 0);
        } else if (graphicsSize == HbListViewItem::MediumIcon) {
            item->setContentWidgetData("selected", 1);
        } else if (graphicsSize == HbListViewItem::LargeIcon) {
            item->setContentWidgetData("selected", 2);
        } else if (graphicsSize == HbListViewItem::Thumbnail) {
            item->setContentWidgetData("selected", 3);
        } else {
            item->setContentWidgetData("selected", 4);
        }
    }
}

void ViewFuteDataForm::populateStretchingStyle()
{
    HbListViewItem *prototype = qobject_cast<HbListViewItem *>(view->itemPrototypes().first());
    if ( prototype ) {
        settingsIndexes[StretchingStyle] = counter++;       

        HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(HbDataFormModelItem::RadioButtonListItem, 
            QString("Landscape mode:"));

        item->setContentWidgetData("items", LandscapeModes);
        if (prototype->stretchingStyle() == HbListViewItem::StretchLandscape) {
            item->setContentWidgetData("selected", 1);
        } else {
            item->setContentWidgetData("selected", 0);
        }
    }
}

void ViewFuteDataForm::populateMaximumRowCount()
{
    HbListViewItem *prototype = qobject_cast<HbListViewItem *>(view->itemPrototypes().first());
    if ( prototype ) {
        settingsIndexes[MaximumRowCount] = counter++;

        HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(HbDataFormModelItem::SliderItem, 
            QString("Max secondary text rows:"));  

        int maximum = 0;
        int minimum = 0;
        prototype->getSecondaryTextRowCount(minimum, maximum);

        item->setContentWidgetData("value", maximum);
        item->setContentWidgetData("maximum", 10);
        item->setContentWidgetData("minimum", 1);
        item->setContentWidgetData("tickPosition", Hb::SliderTicksBelow);
        item->setContentWidgetData("majorTickInterval",1);

        // does not work
        /*QStringList values;
        values << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10"; 
        item->setContentWidgetData("majorTickLabels",values);*/
    }
}

void ViewFuteDataForm::populateMinimumRowCount()
{
    HbListViewItem *prototype = qobject_cast<HbListViewItem *>(view->itemPrototypes().first());
    if ( prototype ) {
        settingsIndexes[MinimumRowCount] = counter++;

        HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(HbDataFormModelItem::SliderItem, 
            QString("Min secondary text rows:"));  

        int maximum = 0;
        int minimum = 0;
        prototype->getSecondaryTextRowCount(minimum, maximum);

        item->setContentWidgetData("value", minimum);
        item->setContentWidgetData("maximum", 10);
        item->setContentWidgetData("minimum", 1);
        item->setContentWidgetData("tickPosition", Hb::SliderTicksBelow);
        item->setContentWidgetData("majorTickInterval",1);
    }
}

void ViewFuteDataForm::populateFrame()
{
    settingsIndexes[Frame] = counter++;
    HbAbstractViewItem *prototype = qobject_cast<HbAbstractViewItem *>(view->itemPrototypes().first());
    const HbFrameBackground &frameBackground = prototype->defaultFrame();

    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::ToggleValueItem, QString("Default frame:"));
    if (frameBackground.isNull()) {
        item->setContentWidgetData("text", "On");
        item->setContentWidgetData("additionalText", "Off");
    } else {
        item->setContentWidgetData("text", "Off");
        item->setContentWidgetData("additionalText", "On");
    }
}

void ViewFuteDataForm::populateRecycling()
{
    settingsIndexes[Recycling] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::ToggleValueItem, QString("Item recycling:"));
    if (view->itemRecycling()) {
        item->setContentWidgetData("text", "On");
        item->setContentWidgetData("additionalText", "Off");
    } else {
        item->setContentWidgetData("text", "Off");
        item->setContentWidgetData("additionalText", "On");
    }
}

void ViewFuteDataForm::populateUniformItem()
{
    HbListView *listView = qobject_cast<HbListView *>(view);
    if (listView) {
        settingsIndexes[UniformItem] = counter++;
        HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
            HbDataFormModelItem::ToggleValueItem, QString("Uniform Items:"));
        if (view->uniformItemSizes()) {
            item->setContentWidgetData("text", "On");
            item->setContentWidgetData("additionalText", "Off");
        } else {
            item->setContentWidgetData("text", "Off");
            item->setContentWidgetData("additionalText", "On");
        }
    }
}

void ViewFuteDataForm::populateRichText()
{
    HbListViewItem *prototype = qobject_cast<HbListViewItem *>(view->itemPrototypes().first());
    if (prototype) {
        settingsIndexes[RichText] = counter++;
        HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
            HbDataFormModelItem::ToggleValueItem, QString("Rich text support:"));
        if (prototype->textFormat() == Qt::RichText) {
            item->setContentWidgetData("text", "On");
            item->setContentWidgetData("additionalText", "Off");
        } else {
            item->setContentWidgetData("text", "Off");
            item->setContentWidgetData("additionalText", "On");
        }
    }
}

void ViewFuteDataForm::populateSelectionMode()
{
    settingsIndexes[SelectionMode] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::RadioButtonListItem, QString("Selection mode:"));
    item->setContentWidgetData("items", SelectionModes);
    if (view->selectionMode() == HbAbstractItemView::SingleSelection) {
        item->setContentWidgetData("selected", 0);
    } else if (view->selectionMode() == HbAbstractItemView::MultiSelection) {
        item->setContentWidgetData("selected", 1);
    } else {
        item->setContentWidgetData("selected", 2);
    }
}


void ViewFuteDataForm::populateLaunchInPopup()
{
    settingsIndexes[LaunchInPopup] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::RadioButtonListItem, QString("Launch target of next model:"));
    item->setContentWidgetData("items", LaunchInPopups);
    if (    settingsData[LaunchInPopup].isValid() 
        &&  settingsData[LaunchInPopup].canConvert(QVariant::Bool)
        &&  settingsData[LaunchInPopup].toBool()) {
        item->setContentWidgetData("selected", 0);
    } else {
        item->setContentWidgetData("selected", 1);
    }
}

void ViewFuteDataForm::populateArrangeMode()
{
    HbListView *listView = qobject_cast<HbListView *>(view);
    if (listView) {
        settingsIndexes[ArrangeMode] = counter++;
        HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
            HbDataFormModelItem::ToggleValueItem, QString("Arrange mode:"));
        if (listView->arrangeMode()) {
            item->setContentWidgetData("text", "On");
            item->setContentWidgetData("additionalText", "Off");
        } else {
            item->setContentWidgetData("text", "Off");
            item->setContentWidgetData("additionalText", "On");
        }
    }
}

void ViewFuteDataForm::populateScrollingStyle()
{
    settingsIndexes[ScrollingStyle] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::RadioButtonListItem, QString("Scroll mode:"));
    item->setContentWidgetData("items", ScrollModes);
    if (view->scrollingStyle() == HbScrollArea::Pan) {
        item->setContentWidgetData("selected", 0);
    } else {
        item->setContentWidgetData("selected", 1);
    }
}

void ViewFuteDataForm::populateClampingStyle()
{
    settingsIndexes[ClampingStyle] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
         HbDataFormModelItem::ToggleValueItem, QString("List bounce clamping:"));
    if (view->clampingStyle() == HbScrollArea::BounceBackClamping) {
        item->setContentWidgetData("text", "On");
        item->setContentWidgetData("additionalText", "Off");
    } else {
        item->setContentWidgetData("text", "Off");
        item->setContentWidgetData("additionalText", "On");
    }
}

void ViewFuteDataForm::populateInteractiveScrollBar()
{
    settingsIndexes[InteractiveScrollBar] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
         HbDataFormModelItem::ToggleValueItem, QString("Interactive scrollbar:"));
    if (view->verticalScrollBar()->isInteractive() == true
        || view->horizontalScrollBar()->isInteractive() == true) {
        item->setContentWidgetData("text", "On");
        item->setContentWidgetData("additionalText", "Off");
    } else {
        item->setContentWidgetData("text", "Off");
        item->setContentWidgetData("additionalText", "On");
    }
}

void ViewFuteDataForm::populateFrictionEnabled()
{
    settingsIndexes[FrictionEnabled] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::ToggleValueItem, QString("List scroll inertia:"));
    if (view->frictionEnabled()) {
        item->setContentWidgetData("text", "On");
        item->setContentWidgetData("additionalText", "Off");
    } else {
        item->setContentWidgetData("text", "Off");
        item->setContentWidgetData("additionalText", "On");
    }
}

void ViewFuteDataForm::populateOrientation()
{
    settingsIndexes[Orientation] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::RadioButtonListItem, QString("Orientation:"));
    item->setContentWidgetData("items", OrientationTypes);
    if (    settingsData[Orientation].isValid() 
        &&  settingsData[Orientation].canConvert(QVariant::Int) 
        &&  (    settingsData[Orientation].toInt() == 0
            ||  settingsData[Orientation].toInt() == 1)) {
            item->setContentWidgetData("selected", settingsData[Orientation].toInt());
    } 
    else {
        item->setContentWidgetData("selected", 0);
    }
}

void ViewFuteDataForm::populateMirroring()
{
    settingsIndexes[Mirroring] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::ToggleValueItem, QString("Layout mirrored:"));
    if (    settingsData[Mirroring].isValid() 
        &&  settingsData[Mirroring].canConvert(QVariant::Int)) {
        if (settingsData[Mirroring].toInt() == Qt::LeftToRight) {
            item->setContentWidgetData("text", "Off");
            item->setContentWidgetData("additionalText", "On");
        }
        else if (settingsData[Mirroring].toInt() == Qt::RightToLeft) {
            item->setContentWidgetData("text", "On");
            item->setContentWidgetData("additionalText", "Off");
        }
    } else {
        item->setContentWidgetData("text", "Off");
        item->setContentWidgetData("additionalText", "On");
    }
}

void ViewFuteDataForm::populateScrollHint()
{
    settingsIndexes[ScrollHint] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::RadioButtonListItem, QString("Scroll-to-target stop position:"));
    item->setContentWidgetData("items", ScrollHints);
    int initialValue = 0;
    if (    settingsData[ScrollHint].isValid() 
        &&  settingsData[ScrollHint].canConvert(QVariant::Int)
        &&  settingsData[ScrollHint].toInt() >= 0
        &&  settingsData[ScrollHint].toInt() <= QAbstractItemView::PositionAtCenter) {
        initialValue = settingsData[ScrollHint].toInt();
    }

    item->setContentWidgetData("selected", initialValue);
}

void ViewFuteDataForm::populateIndexFeedbackPolicy()
{
    if (mIndexFeedback) {
        settingsIndexes[IndexFeedbackPolicy] = counter++;
        HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
            HbDataFormModelItem::RadioButtonListItem, QString("Index Feedback Policy:"));
        item->setContentWidgetData("items", IndexFeedbackPolicies);
        int initialValue = 0;

        if (! mIndexFeedback->itemView() ) {
            initialValue = 4;
        } else {
            initialValue = (int) mIndexFeedback->indexFeedbackPolicy();
        }

        item->setContentWidgetData("selected", initialValue);
    }
}

void ViewFuteDataForm::resolveSettingsResults()
{
    if (!view) {
        return;
    }

    HbListView *listView = qobject_cast<HbListView *>(view);

    HbAbstractViewItem *prototype = view->itemPrototypes().first();
    HbListViewItem *prototypeListView = qobject_cast<HbListViewItem *>(prototype);
    if (prototypeListView) {
        prototypeListView->setGraphicsSize(getGraphicsSize());
        prototypeListView->setStretchingStyle(getStretchingStyle());
        int minBefore = 1;
        int maxBefore = 1;
        prototypeListView->getSecondaryTextRowCount(minBefore, maxBefore);
        if (    minBefore != 1
            ||  maxBefore != 1
            ||  getMinimumRowCount() != 1
            ||  getMaximumRowCount() != 1 ) {
            prototypeListView->setSecondaryTextRowCount(getMinimumRowCount(), getMaximumRowCount());
        }
    }

    HbFrameBackground frame;
    if (getFrame()){
        frame.setFrameGraphicsName(QString());
    } else {
        frame.setFrameGraphicsName(QString(""));
    }
    prototype->setDefaultFrame(frame);

    view->setItemRecycling(getRecycling());
    view->setUniformItemSizes(getUniformItem());

    if (prototypeListView) {
        prototypeListView->setTextFormat(getRichText());
    }
    view->setSelectionMode(getSelectionMode());
    view->setScrollingStyle(getScrollingStyle());
    view->setClampingStyle(getClampingStyle());
    view->setFrictionEnabled(getFrictionEnabled());

    bool interactiveScrollBar = getInteractiveScrollBar();
    if (view->scrollDirections() & Qt::Horizontal)
    {
        view->horizontalScrollBar()->setInteractive(interactiveScrollBar);
    }
    if (view->scrollDirections() & Qt::Vertical)
    {
        view->verticalScrollBar()->setInteractive(interactiveScrollBar);
    }

    // must be after interactive scrollbar setting
    if (listView) {
        listView->setArrangeMode(getArrangeMode());
    }

    if (mIndexFeedback) {
        int indexFeedbackMode = getIndexFeedbackPolicy();
        if (indexFeedbackMode == 4) {
            mIndexFeedback->setItemView(0);
        } else {
            mIndexFeedback->setItemView(view);
            mIndexFeedback->setIndexFeedbackPolicy((HbIndexFeedback::IndexFeedbackPolicy) indexFeedbackMode);
        }
    }

    view->setItemPixmapCacheEnabled(getPixmapCacheEnaled());
    view->setIconLoadPolicy(getIconLoadPolicy());
/*
    if (listView) {
        listView->setEnabledAnimations(getEnabledAnimations());
    }
*/
}

HbListViewItem::GraphicsSize ViewFuteDataForm::getGraphicsSize()
{
    HbListViewItem::GraphicsSize graphicsSize(HbListViewItem::SmallIcon);

    if (settingsIndexes[GraphicsSize] != -1) {
        if (static_cast<HbDataFormViewItem*>(itemByIndex(
                settingsFormModel->index(settingsIndexes[GraphicsSize],0)))->dataItemContentWidget()->property("selected").toInt() == 1) {
            graphicsSize = HbListViewItem::MediumIcon;
        } else if (static_cast<HbDataFormViewItem*>(itemByIndex(
                settingsFormModel->index(settingsIndexes[GraphicsSize],0)))->dataItemContentWidget()->property("selected").toInt() == 2) {
            graphicsSize = HbListViewItem::LargeIcon;
        } else if (static_cast<HbDataFormViewItem*>(itemByIndex(
                settingsFormModel->index(settingsIndexes[GraphicsSize],0)))->dataItemContentWidget()->property("selected").toInt() == 3) {
            graphicsSize = HbListViewItem::Thumbnail;
        } else if (static_cast<HbDataFormViewItem*>(itemByIndex(
                settingsFormModel->index(settingsIndexes[GraphicsSize],0)))->dataItemContentWidget()->property("selected").toInt() == 4) {
            graphicsSize = HbListViewItem::WideThumbnail;
        }
    } 

    return graphicsSize;
}

HbListViewItem::StretchingStyle ViewFuteDataForm::getStretchingStyle()
{
    HbListViewItem::StretchingStyle stretchingStyle(HbListViewItem::NoStretching);

    if (settingsIndexes[StretchingStyle] != -1) {
        if (static_cast<HbDataFormViewItem*>(itemByIndex(
                settingsFormModel->index(settingsIndexes[StretchingStyle],0)))->dataItemContentWidget()->property("selected").toInt() == 1) {
            stretchingStyle = HbListViewItem::StretchLandscape;
        }
    } 
     
    return stretchingStyle;
}

int ViewFuteDataForm::getMaximumRowCount()
{
    int maximumRowCount = 0;
    if (settingsIndexes[MaximumRowCount] != -1) {
        maximumRowCount = static_cast<HbDataFormViewItem*>(itemByIndex(
                settingsFormModel->index(settingsIndexes[MaximumRowCount],0)))->dataItemContentWidget()->property("value").toInt();
    } 

    return maximumRowCount;
}

int ViewFuteDataForm::getMinimumRowCount()
{
    int minimumRowCount = 0;
    if (settingsIndexes[MinimumRowCount] != -1) {
        minimumRowCount = static_cast<HbDataFormViewItem*>(itemByIndex(
                settingsFormModel->index(settingsIndexes[MinimumRowCount],0)))->dataItemContentWidget()->property("value").toInt();
    } 

    return minimumRowCount;
}

bool ViewFuteDataForm::getFrame()
{
    if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[Frame],0)))->
        dataItemContentWidget()->property("text").toString() == "On") {
        return true;
    }
    return false;
}

bool ViewFuteDataForm::getRecycling()
{
    if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[Recycling],0)))->
            dataItemContentWidget()->property("text").toString() == "On") {
        return true;
    }
    return false;
}

bool ViewFuteDataForm::getUniformItem()
{
    if (settingsIndexes[UniformItem] != -1) {
        if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[UniformItem],0)))->
                dataItemContentWidget()->property("text").toString() == "On") {
            return true;
        }
    }
    return false;
}

Qt::TextFormat ViewFuteDataForm::getRichText()
{
    if (settingsIndexes[RichText] != -1) {
        if (static_cast<HbDataFormViewItem*>(itemByIndex(
            settingsFormModel->index(settingsIndexes[RichText],0)))->
                dataItemContentWidget()->property("text").toString() == "Off") {
            return Qt::PlainText;
        } 
        return Qt::RichText;
    }
    return Qt::PlainText;
}

HbAbstractItemView::SelectionMode ViewFuteDataForm::getSelectionMode()
{
    int value = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[SelectionMode],0)))->
        dataItemContentWidget()->property("selected").toInt();
    if (value == 0) {
        return HbListView::SingleSelection;
    } else if (value == 1) {
        return HbListView::MultiSelection;
    } else {
        return HbListView::NoSelection;
    }
}

bool ViewFuteDataForm::getLaunchInPopup()
{
    int value = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[LaunchInPopup],0)))->
        dataItemContentWidget()->property("selected").toInt();
    if (value == 0) {
        return true;
    } else {
        return false;
    }
}


bool ViewFuteDataForm::getArrangeMode()
{
    if (settingsIndexes[ArrangeMode] != -1) {
        if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[ArrangeMode],0)))->
                dataItemContentWidget()->property("text").toString() == "On") {
            return true;
        } 
    }
    return false;
}

HbScrollArea::ScrollingStyle ViewFuteDataForm::getScrollingStyle()
{
    int value = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[ScrollingStyle],0)))->
        dataItemContentWidget()->property("selected").toInt();
    if (value == 0) {
        return HbScrollArea::Pan;
    } else {
        return HbScrollArea::PanWithFollowOn;
    }
}

HbScrollArea::ClampingStyle ViewFuteDataForm::getClampingStyle()
{
    if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[ClampingStyle],0)))->
            dataItemContentWidget()->property("text").toString() == "On") {
        return HbScrollArea::BounceBackClamping;
    } 
    return HbScrollArea::StrictClamping;
}

bool ViewFuteDataForm::getFrictionEnabled()
{
    if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[FrictionEnabled],0)))->
            dataItemContentWidget()->property("text").toString() == "On") {
        return true;
    } 
    return false;
}

Qt::LayoutDirection ViewFuteDataForm::getMirroring()
{
    int value = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[Mirroring],0)))->
        dataItemContentWidget()->property("selected").toInt();
    if (value == 1) {
        return Qt::RightToLeft;
    } 
    return Qt::LeftToRight;
}

Qt::Orientation ViewFuteDataForm::getOrientation()
{
    int value = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[Orientation],0)))->
        dataItemContentWidget()->property("selected").toInt();
    if (value == 0) {
        return Qt::Vertical;
    } else {
        return Qt::Horizontal;
    } 
}

HbAbstractItemView::ScrollHint ViewFuteDataForm::getScrollHint()
{
    int value = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[ScrollHint],0)))->
        dataItemContentWidget()->property("selected").toInt();
    return (HbAbstractItemView::ScrollHint)value;
}

int ViewFuteDataForm::getIndexFeedbackPolicy()
{
    int value = 0;
    if (mIndexFeedback) {
        value = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[IndexFeedbackPolicy],0)))->
            dataItemContentWidget()->property("selected").toInt();
        return value;
    }
    return value;
}

bool ViewFuteDataForm::getInteractiveScrollBar()
{
    if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[InteractiveScrollBar],0)))->
            dataItemContentWidget()->property("text").toString() == "On") {
        return true;
    }
    return false;
}

HbAbstractItemView::ItemAnimations ViewFuteDataForm::getEnabledAnimations()
{
    QVariant selectedAsVariant = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[Animations],0)))->
        dataItemContentWidget()->property("selectedItems");

    QList<QVariant> selected = selectedAsVariant.value< QList<QVariant> >();

    HbAbstractItemView::ItemAnimations mask = HbAbstractItemView::None;

    HbListView *listView = qobject_cast<HbListView *>(view);
    if (listView) {
        if (selected.contains(0)) {
            mask |= HbAbstractItemView::Appear;
        }
        if (selected.contains(1)) {
            mask |= HbAbstractItemView::Disappear;
        }
        if (selected.contains(2)) {
            mask |= HbAbstractItemView::TouchDown;
        }
    } else {
        if (selected.contains(0)) {
            mask |= HbAbstractItemView::TouchDown;
        }
    }
    return mask;
}

bool ViewFuteDataForm::getPixmapCacheEnaled()
{
    if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[PixmapCache],0)))->
            dataItemContentWidget()->property("text").toString() == "On") {
        return true;
    }
    return false;
}

HbAbstractItemView::IconLoadPolicy ViewFuteDataForm::getIconLoadPolicy()
{
    int selection = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(settingsIndexes[IconLoadPolicy],0)))->
            dataItemContentWidget()->property("selected").toInt();
    if ( selection == 0) {
        return HbAbstractItemView::LoadSynchronously;
    } else if (selection == 2) {
        return HbAbstractItemView::LoadAsynchronouslyAlways;
    } else {
        return HbAbstractItemView::LoadAsynchronouslyWhenScrolling;
    }
}

void ViewFuteDataForm::populateAddItem()
{
    activity = AddItem;
    settingsFormModel->clear();

    populatePrimaryText();
    populateSecondaryText();
    populateThirdText();
    populateLeftColumn();
    populateRightColumn();
    populateType();
    populateBackground();
    populateAddItemCount();

    setModel(settingsFormModel);

    HbListView *listView = qobject_cast<HbListView *>(view);
    HbTreeView *treeView = qobject_cast<HbTreeView *>(view);
    if (listView || treeView) {
        HbAction *action = new HbAction(tr("Format"));
        connect(action, SIGNAL(triggered()), this, SLOT(formatTriggered()));

        if (addItemIndexes[PrimaryText] != -1) {
            if (qobject_cast<HbLineEdit *>(static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[PrimaryText],0)))->dataItemContentWidget())) {
                HbLineEdit *editor = qobject_cast<HbLineEdit *>(static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[PrimaryText],0)))->dataItemContentWidget());
                HbEditorInterface editInterface(editor);
                editInterface.addAction(action);
            }
        }

        if (addItemIndexes[SecondaryText] != -1) {
            if (qobject_cast<HbLineEdit *>(static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[SecondaryText],0)))->dataItemContentWidget())) {
                HbLineEdit *editor = qobject_cast<HbLineEdit *>(static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[SecondaryText],0)))->dataItemContentWidget());
                HbEditorInterface editInterface2(editor);
                editInterface2.addAction(action);
            }

        if (addItemIndexes[ThirdText] != -1) {
            if (qobject_cast<HbLineEdit *>(static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[ThirdText],0)))->dataItemContentWidget())) {
                HbLineEdit *editor = qobject_cast<HbLineEdit *>(static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[ThirdText],0)))->dataItemContentWidget());
                HbEditorInterface editInterface2(editor);
                editInterface2.addAction(action);
            }
        }
        }
    }
}

void ViewFuteDataForm::populatePrimaryText()
{
    addItemIndexes[PrimaryText] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
            HbDataFormModelItem::TextItem, QString("Primary text:"));
    if (addItemData[PrimaryText].isValid() && addItemData[PrimaryText].canConvert(QVariant::String)) {
        item->setContentWidgetData("text", addItemData[PrimaryText].toString());
    }
}

void ViewFuteDataForm::populateSecondaryText()
{
    addItemIndexes[SecondaryText] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::TextItem, QString("Secondary text:"));
    if (addItemData[SecondaryText].isValid() && addItemData[SecondaryText].canConvert(QVariant::String)) {
        item->setContentWidgetData("text", addItemData[SecondaryText].toString());
    }
}

void ViewFuteDataForm::populateThirdText()
{
    addItemIndexes[ThirdText] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::TextItem, QString("Third text:"));
    if (addItemData[ThirdText].isValid() && addItemData[ThirdText].canConvert(QVariant::String)) {
        item->setContentWidgetData("text", addItemData[ThirdText].toString());
    }
}


void ViewFuteDataForm::populateLeftColumn()
{
    addItemIndexes[LeftColumn] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::RadioButtonListItem, QString("Left column widget:"));
    item->setContentWidgetData(QString("items"), LeftColumnWidgets);
    item->setContentWidgetData("selected", 0);
}

void ViewFuteDataForm::populateRightColumn()
{
    addItemIndexes[RightColumn] = counter++;
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::RadioButtonListItem, QString("Right column widget:"));
    item->setContentWidgetData(QString("items"), RightColumnWidgets);
    item->setContentWidgetData("selected", 0);
}

void ViewFuteDataForm::populateType()
{    
    HbListView *listView = qobject_cast<HbListView *>(view);
    if (listView) {
        addItemIndexes[Type] = counter++;
        HbDataFormModelItem *item = settingsFormModel->
            appendDataFormItem(HbDataFormModelItem::RadioButtonListItem, QString("Type:"));
        item->setContentWidgetData("items", ItemTypes);
        item->setContentWidgetData("selected", 0);
    }
}

void ViewFuteDataForm::populateBackground()
{
    addItemIndexes[Background] = counter++;
    HbDataFormModelItem *item = settingsFormModel->
        appendDataFormItem(HbDataFormModelItem::RadioButtonListItem, QString("Background:"));
    item->setContentWidgetData("items", ItemBackgrounds);
    item->setContentWidgetData("selected", 0);
}

void ViewFuteDataForm::populateAddItemCount()
{

    addItemIndexes[ItemCount] = counter++;
    HbDataFormModelItem *item = settingsFormModel->
        appendDataFormItem(HbDataFormModelItem::TextItem, QString("Item count:"));
    if (addItemData[ItemCount].isValid() && addItemData[ItemCount].canConvert(QVariant::Int)) {
        item->setContentWidgetData("text", addItemData[ItemCount].toInt());
    } else {
        item->setContentWidgetData("text", QString("1"));
    }
}

void ViewFuteDataForm::populateEnabledAnimations()
{    
    settingsIndexes[Animations] = counter++;

    HbListView *listView = qobject_cast<HbListView *>(view);

    QStringList animations;
    if (listView) {
        animations << "Appear" << "Disappear";
    }
    animations << "TouchDown";

    QList<QVariant> selected;
    if (listView) {
        if (view->enabledAnimations() | HbAbstractItemView::Appear) {
            selected.append(0);
        }
        if (view->enabledAnimations() | HbAbstractItemView::Disappear) {
            selected.append(1);
        }
        if (view->enabledAnimations() | HbAbstractItemView::TouchDown) {
            selected.append(2);
        }
    } else {
        if (view->enabledAnimations() | HbAbstractItemView::TouchDown) {
            selected.append(0);
        }
    }

    HbDataFormModelItem *item = settingsFormModel->
        appendDataFormItem(HbDataFormModelItem::MultiselectionItem, QString("Enabled animations:"));
    item->setContentWidgetData("items", animations);

    item->setContentWidgetData("selectedItems", selected);
}

void ViewFuteDataForm::populatePixmapCacheEnabled()
{
    settingsIndexes[PixmapCache] = counter++;
    
    HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
        HbDataFormModelItem::ToggleValueItem, QString("Pixmap cache:"));
    if (view->itemPixmapCacheEnabled()) {
        item->setContentWidgetData("text", "On");
        item->setContentWidgetData("additionalText", "Off");
    } else {
        item->setContentWidgetData("text", "Off");
        item->setContentWidgetData("additionalText", "On");
    }
}

void ViewFuteDataForm::populateIconLoadPolicy()
{
    settingsIndexes[IconLoadPolicy] = counter++;
    
    HbDataFormModelItem *item = settingsFormModel->
        appendDataFormItem(HbDataFormModelItem::RadioButtonListItem, QString("Icon load policy:"));
    item->setContentWidgetData("items", IconLoadPolicies);
    switch (view->iconLoadPolicy()) {
        case HbAbstractItemView::LoadSynchronously: {
            item->setContentWidgetData("selected", 0);
            break;
        }
        case HbAbstractItemView::LoadAsynchronouslyWhenScrolling: {
            item->setContentWidgetData("selected", 1);
            break;
        }
        case HbAbstractItemView::LoadAsynchronouslyAlways: {
            item->setContentWidgetData("selected", 2);
            break;
        }
        default: {
            break;
        }
    }    
}

QList <HbListWidgetItem *> ViewFuteDataForm::getListWidgetItems()
{
    QList <HbListWidgetItem *> items;

    if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[PrimaryText],0)))->
            dataItemContentWidget()->property("text").toString().isEmpty()
     && static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[SecondaryText],0)))->
            dataItemContentWidget()->property("text").toString().isEmpty()
     && static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[ThirdText],0)))->
            dataItemContentWidget()->property("text").toString().isEmpty()
     && static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[LeftColumn],0)))->
            dataItemContentWidget()->property("selected").toInt() == 0
     && static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[RightColumn],0)))->
            dataItemContentWidget()->property("selected").toInt() == 0) {
         return items;
    }

    QVariantList strings;
    QVariantList icons;

    QString primaryString = getPrimaryText();
    QString secondaryString = getSecondaryText();
    QString thirdString = getThirdText();


    if (!thirdString.isEmpty()) {
        if (primaryString.isEmpty()) {
            primaryString = " ";
        }
        strings.append(primaryString);

        if (!secondaryString.isEmpty()) {
            strings.append(secondaryString);
        } else {
            strings.append(QVariant());
        }
        strings.append(thirdString);
    } else if (!secondaryString.isEmpty()) {
        if (primaryString.isEmpty()) {
            primaryString = " ";
        }
        strings.append(primaryString);
        strings.append(secondaryString);
    } else if (!primaryString.isEmpty()) {
        strings.append(primaryString);
    }

    QVariant value1 = getLeftColumn();
    QVariant value2 = getRightColumn();
    if (value2.canConvert<HbIcon>()) {
        icons.append(value1);
        icons.append(value2);
    } else if (value1.canConvert<HbIcon>()) {
        icons.append(value1);
    }

    QVariant background = getBackground();

    int count = getAddItemCount();
    for (int i = 0; i < count; i++) {

        if (i != 0) {
            QString s = QString("%1_%2").arg(primaryString).arg(i);
            strings.replace(0, s);
        }
        
        HbListWidgetItem *widgetItem = new HbListWidgetItem(getType());
        
        widgetItem->setData(strings, Qt::DisplayRole);
        widgetItem->setData(strings.at(0), Hb::IndexFeedbackRole);
        widgetItem->setData(icons, Qt::DecorationRole);

        if (background.isValid()) {
            widgetItem->setBackground(background);
        }

        items.append(widgetItem);
    }


    return items;
}

QList <QStandardItem *> ViewFuteDataForm::getStandardItems()
{
    QList <QStandardItem *> items; 

    if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[PrimaryText],0)))->
            dataItemContentWidget()->property("text").toString().isEmpty()
     && static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[SecondaryText],0)))->
            dataItemContentWidget()->property("text").toString().isEmpty()
     &&  static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[ThirdText],0)))->
            dataItemContentWidget()->property("text").toString().isEmpty()
     && static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[LeftColumn],0)))->
            dataItemContentWidget()->property("selected").toInt() == 0
     && static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[RightColumn],0)))->
            dataItemContentWidget()->property("selected").toInt() == 0) {
         return items;
    }

    QString primaryString = getPrimaryText();
    if (primaryString.isEmpty()) {
        primaryString = " ";
    }

    QString secondaryString = getSecondaryText();

    QStringList displayTexts;
    displayTexts.append(primaryString);

    if (!secondaryString.isEmpty()) {
        displayTexts.append(secondaryString);
    }

    QString thirdString = getThirdText();

    if (!thirdString.isEmpty()) {
        displayTexts.append(thirdString);
    }


    QVariant left = getLeftColumn();
    QVariant right = getRightColumn();
    QVariantList decorations;
    decorations.append(left);
    decorations.append(right);

    QVariant background = getBackground();

    int count = getAddItemCount();
    for (int i = 0; i < count; i++) {

        if (i != 0) {
            QString s = QString("%1_%2").arg(primaryString).arg(i);
            displayTexts.replace(0, s);
        }

        QStandardItem *item = new QStandardItem();
        item->setData(displayTexts, Qt::DisplayRole);
        item->setData(decorations, Qt::DecorationRole);

        if (background.isValid()) {
            item->setData(background, Qt::BackgroundRole);
        }

        items.append(item);
    }
    return items;
}




QString ViewFuteDataForm::getPrimaryText()
{
    return (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[PrimaryText],0)))->
            dataItemContentWidget()->property("text").toString());
}

QString ViewFuteDataForm::getSecondaryText()
{
    return (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[SecondaryText],0)))->
            dataItemContentWidget()->property("text").toString());
}

QString ViewFuteDataForm::getThirdText()
{
    return (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[ThirdText],0)))->
            dataItemContentWidget()->property("text").toString());
}

QVariant ViewFuteDataForm::getLeftColumn()
{
    HbIcon icon(KConvenienceIcons.at(0));
    int i = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[LeftColumn],0)))->
        dataItemContentWidget()->property("selected").toInt();
    if (i == 1) {
        return icon;
    }
    return QVariant();
}

QVariant ViewFuteDataForm::getRightColumn()
{
    HbIcon icon(KConvenienceIcons.at(1));
    int i = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[RightColumn],0)))->
        dataItemContentWidget()->property("selected").toInt();
    if (i == 1) {
        return icon;
    } 
    return QVariant();
}

Hb::ModelItemType ViewFuteDataForm::getType()
{
    Hb::ModelItemType type = Hb::StandardItem;
    if (   addItemIndexes[Type] != -1
        && static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[Type],0)))->
            dataItemContentWidget()->property("selected").toInt() == 1) {
        type = Hb::SeparatorItem;
    }
    return type;
}

QVariant ViewFuteDataForm::getBackground()
{
    HbIcon icon(KConvenienceIcons.at(2));
    int i = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[Background],0)))->
        dataItemContentWidget()->property("selected").toInt();
    QVariant background;
    if (i == 1) {
        background = QColor(Qt::red);
    } else if (i == 2) {
        background = QBrush(Qt::green, Qt::SolidPattern);
    } else if (i == 3) {
        background = QBrush(Qt::blue, Qt::SolidPattern);
    } else if (i == 4) {
        background = icon;
    } else if (i == 5) {
        background = HbFrameBackground(":/demo/qsn_fr_list", HbFrameDrawer::NinePieces);
    }
    return background;
}

int ViewFuteDataForm::getAddItemCount()
{
    QString result = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(addItemIndexes[ItemCount],0)))->
                        dataItemContentWidget()->property("text").toString();
    return result.toInt();
}


void ViewFuteDataForm::formatTriggered()
{
    HbMenu *menu = new HbMenu();
    HbAction *action = menu->addAction("Bold");
    action->setData("<b>Bold text </b>");
    connect(action, SIGNAL(triggered()), this, SLOT(addFormating()));

    action = menu->addAction("Italic");
    action->setData("<i>Italic text </i>");
    connect(action, SIGNAL(triggered()), this, SLOT(addFormating()));

    action = menu->addAction("Underline");
    action->setData("<u>Underlined text </u>");
    connect(action, SIGNAL(triggered()), this, SLOT(addFormating()));

    action = menu->addAction("Link");
    action->setData("<a href=\"link\">Link text</a> ");
    connect(action, SIGNAL(triggered()), this, SLOT(addFormating()));

    action = menu->addAction("Color");
    action->setData("<font color=red>Colored text </font> ");
    connect(action, SIGNAL(triggered()), this, SLOT(addFormating()));

    action = menu->addAction("Font");
    action->setData("<h3>Medium text </h3> ");
    connect(action, SIGNAL(triggered()), this, SLOT(addFormating()));

    /*action = menu->addAction("List");
    action->setData("<ul><li><a href=\"link\">Open</a><li><a href=\"link\">Save</a><li><a href=\"link\">Delete</a><li><a href=\"link\">Move</a></ul>");
    connect(action, SIGNAL(triggered()), this, SLOT(addFormating()));*/

    menu->setPreferredPos(scene()->focusItem()->scenePos());
    menu->open();
}

void ViewFuteDataForm::addFormating()
{
    HbAction *action = qobject_cast<HbAction *>(sender());
    QGraphicsWidget* focused = focusWidget();
    if (focused) {
        HbLineEdit *item = qobject_cast<HbLineEdit*>(focused);

        if (item) {
            QString newText = item->text();
            newText.append(action->data().toString());
            item->setText(newText);
        }
    }
}

void ViewFuteDataForm::setIndexFeedback(HbIndexFeedback* indexFeedback)
{
    mIndexFeedback = indexFeedback;
}


