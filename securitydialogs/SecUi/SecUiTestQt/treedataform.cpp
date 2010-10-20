/*
* ====================================================
*  Name        : treedataform.cpp
*  Part of     : fute/SecUiTestQt
*  Description : Data form for SecUiTestQt
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

#include "treedataform.h"
#include "dirviewitem.h"
#include <hbtreeview.h>
#include <hbtreeviewitem.h>
#include <hbdataformmodel.h>

const QStringList KDepths = ( QStringList() << "1" << "3" << "5" << "all" );
const QStringList ViewItems = (QStringList() << "Default" << "DirItem");

// ======== MEMBER FUNCTIONS ========

TreeDataForm::TreeDataForm( HbAbstractItemView &view,
                                   QGraphicsItem *parent):
    ViewFuteDataForm(view, parent),
    depth(1),
    indentation(-1),
    dirViewItemEnabled(false)
{
}

TreeDataForm::~TreeDataForm()
{
}

void TreeDataForm::initialise()
{
    depth = 1;
    dirViewItemEnabled = false;
    for (int i=0; i< TreeCustomLast; i++) {
        customTreeSettingsIndexes[i] = -1;
    }
    ViewFuteDataForm::initialise();
}

int TreeDataForm::populateCustomSettingsItem(int previousItem)
{
    HbTreeView *tree = qobject_cast<HbTreeView*>(view);
    HbTreeViewItem *prototype = qobject_cast<HbTreeViewItem *>(tree->itemPrototypes().first());

    if (tree) {
        if (previousItem == ViewFuteDataForm::ScrollHint) {
            customTreeSettingsIndexes[Depth] = counter;
            HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
                HbDataFormModelItem::RadioButtonListItem, QString("Depth of visible tree:"));
            item->setContentWidgetData("items", KDepths);
            if (depth == 1) {
                item->setContentWidgetData("selected", 0);
            } else if (depth == 3) {
                item->setContentWidgetData("selected", 1);
            } else if (depth == 5) {
                item->setContentWidgetData("selected", 2);
            } else {
                item->setContentWidgetData("selected", 3);
            }

            customTreeSettingsIndexes[Indentation] = counter+1;           
            item = settingsFormModel->appendDataFormItem(
                    HbDataFormModelItem::TextItem, QString("Indentation: (negative sets default)"));
            QString indentationString;
            indentationString.setNum(indentation);
            item->setContentWidgetData("text", indentationString);

            customTreeSettingsIndexes[ItemUserExpandable] = counter + 2;
            item = settingsFormModel->appendDataFormItem(
                HbDataFormModelItem::ToggleValueItem, QString("Items user expandable:"));
            if (prototype->isUserExpandable()) {
                item->setContentWidgetData("text", "On");
                item->setContentWidgetData("additionalText", "Off");
            } else {
                item->setContentWidgetData("text", "Off");
                item->setContentWidgetData("additionalText", "On");
            }
            return 3;
        } else if ( previousItem == ViewFuteDataForm::FrictionEnabled
                   && dirViewItemEnabled) {
            customTreeSettingsIndexes[ViewItemType] = counter;
            HbDataFormModelItem *item = settingsFormModel->appendDataFormItem(
                HbDataFormModelItem::RadioButtonListItem, QString("View item type:"));
            item->setContentWidgetData("items", ViewItems);
            if (qobject_cast<DirViewItem*>(tree->itemPrototypes().first())) {
                item->setContentWidgetData("selected", 1);
            } else {
                item->setContentWidgetData("selected", 0);
            }
            return 1;
        }
    }
    return 0;
}

void TreeDataForm::resolveSettingsResults()
{
    HbTreeView *tree = qobject_cast<HbTreeView*>(view);
    HbTreeViewItem *prototype = qobject_cast<HbTreeViewItem *>(tree->itemPrototypes().first());

    if (tree) {
        if (dirViewItemEnabled) {
            if ( static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(customTreeSettingsIndexes[ViewItemType],0)))->
                dataItemContentWidget()->property("selected").toInt() == 1) {
                if (!qgraphicsitem_cast<DirViewItem*>(tree->itemPrototypes().first())) {
                    DirViewItem *prototype = new DirViewItem;
                    tree->setItemPrototype(prototype);
                    tree->setLayoutName("treeviewitem_dir_button");
                }
            }
            else {
                HbTreeViewItem *prototype = new HbTreeViewItem;
                tree->setItemPrototype(prototype);
                tree->setLayoutName("default");
            }
        }
        QString indentationString = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(
            customTreeSettingsIndexes[Indentation],0)))->dataItemContentWidget()->property("text").toString();
        bool ok = false;
        int newIndentation = indentationString.toInt(&ok);
        if (ok) {
            indentation = newIndentation;
        }
        tree->setIndentation(indentation);

        if (prototype) {
            if (static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(
                customTreeSettingsIndexes[ItemUserExpandable],0)))->dataItemContentWidget()->property("text").toString() == "On") {
                prototype->setUserExpandable(true);
            } else {
                prototype->setUserExpandable(false);
            }
        }
    }
    ViewFuteDataForm::resolveSettingsResults();
}



void TreeDataForm::setIndentation(int indentation)
{
    this->indentation = indentation;
}


void TreeDataForm::setDepth(int depth)
{
    this->depth = depth;
}

void TreeDataForm::setDirViewItemEnabled(bool enable)
{
    dirViewItemEnabled = enable;
}

int TreeDataForm::getDepth() const
{
    int selected = static_cast<HbDataFormViewItem*>(itemByIndex(settingsFormModel->index(customTreeSettingsIndexes[Depth],0)))->
                    dataItemContentWidget()->property("selected").toInt();
    if (    selected >= 0
        &&  selected < KDepths.count()) {
        if (KDepths[selected] == KDepths[KDepths.count()-1]) {
            return 999;
        } else {
            return KDepths[selected].toInt();
        }
    } else {
        return depth;
    }
}

