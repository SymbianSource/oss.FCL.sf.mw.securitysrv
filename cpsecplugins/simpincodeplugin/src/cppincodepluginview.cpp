/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:  
 *
 */

// System includes
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hblineedit.h>
#include <seccodeeditdataformviewitem.h>
#include <seccodesettings.h>

// User includes
#include "cppincodepluginview.h"
#include <../../inc/cpsecplugins.h>

/*!
    \class CpPinCodePluginView
    \brief Pin Code Setting view class

    This class is used to create PIN code setting view
*/

// ======== LOCAL FUNCTIONS ========

/*!
   Constructor
*/
CpPinCodePluginView::CpPinCodePluginView(QGraphicsItem *parent /*= 0*/)
	: CpBaseSettingView(0,parent), mSecCodeSettings(new SecCodeSettings())
{
		RDEBUG("0", 0);
    HbDataForm *form = qobject_cast<HbDataForm*>(widget());
    if (form) {
        QList<HbAbstractViewItem *> protoTypeList = form->itemPrototypes();
        protoTypeList.append(new SecCodeEditDataFormViewItem());
        form->setItemPrototypes(protoTypeList);     
        form->setHeading(tr("PIN code"));

        mFormModel = new HbDataFormModel(this);        
        mPinCodeRequestItem = new HbDataFormModelItem(
            HbDataFormModelItem::ToggleValueItem, tr("PIN code requests"));

        bool currentPinCodeRequest = mSecCodeSettings->pinCodeRequest();
        if (currentPinCodeRequest) { 
            mPinCodeRequestItem->setContentWidgetData("text", tr("On"));
            mPinCodeRequestItem->setContentWidgetData("additionalText", tr("On"));
        } else {
            mPinCodeRequestItem->setContentWidgetData("text",tr("Off"));
            mPinCodeRequestItem->setContentWidgetData("additionalText", tr("Off"));
        }
				RDEBUG("form->addConnection", 1);
        connect(mFormModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), 
                  this, SLOT(toggleChange(QModelIndex, QModelIndex)));
        mFormModel->appendDataFormItem(mPinCodeRequestItem);

        HbDataFormModelItem *pinCodeItem = new HbDataFormModelItem(
            static_cast<HbDataFormModelItem::DataItemType>
            (SecCodeEditDataFormViewItem::SecCodeEditItem), tr("PIN code"));
        pinCodeItem->setContentWidgetData("echoMode", HbLineEdit::Password);
        pinCodeItem->setContentWidgetData("text", "1111");
        pinCodeItem->setContentWidgetData("readOnly", true);
        form->addConnection(pinCodeItem, SIGNAL(clicked()), this,
        		SLOT(changePinCode()));
        mFormModel->appendDataFormItem(pinCodeItem);

        HbDataFormModelItem *pin2CodeItem = new HbDataFormModelItem(
            static_cast<HbDataFormModelItem::DataItemType>
            (SecCodeEditDataFormViewItem::SecCodeEditItem), tr("PIN2 code"));
        pin2CodeItem->setContentWidgetData("echoMode", HbLineEdit::Password);
        pin2CodeItem->setContentWidgetData("text", "1111");
        pin2CodeItem->setContentWidgetData("readOnly", true);
        form->addConnection(pin2CodeItem, SIGNAL(clicked()), this,
        		SLOT(changePin2Code()));
        mFormModel->appendDataFormItem(pin2CodeItem);
				RDEBUG("form->setModel", 1);
        form->setModel(mFormModel);
    }
}

/*!
   Destructor
*/
CpPinCodePluginView::~CpPinCodePluginView()
{
    delete mSecCodeSettings;
}

/*!
   response for click pin code request
*/
void CpPinCodePluginView::toggleChange(QModelIndex startIn, 
        QModelIndex /*endIn*/)
{
		RDEBUG("0", 0);
    HbDataFormModelItem *item = mFormModel->itemFromIndex(startIn);
    if(item->type() == HbDataFormModelItem::ToggleValueItem) {
        if (mSecCodeSettings->changePinCodeRequest()) {
        		RDEBUG("got changePinCodeRequest", 1);
						disconnect(mFormModel, 
                    SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, 
                    SLOT(toggleChange(QModelIndex, QModelIndex)));
        		RDEBUG("disconnected", 1);
            QString text = mPinCodeRequestItem->contentWidgetData(
                    "text").toString();
            if (0 == text.compare("On")) {
                mPinCodeRequestItem->setContentWidgetData("text", 
                    tr("Off"));
                mPinCodeRequestItem->setContentWidgetData("additionalText", 
                    tr("Off"));
            } else {
                mPinCodeRequestItem->setContentWidgetData("text", 
                    tr("On"));
                mPinCodeRequestItem->setContentWidgetData("additionalText", 
                    tr("On"));
            }
         		RDEBUG("reconnect", 1);
						connect(mFormModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), 
                    this, SLOT(toggleChange(QModelIndex, QModelIndex)));
        }
      else
      	{
      	RDEBUG("value was not changed", 0);
      	}
    }
   	RDEBUG("0", 0);
}

/*!
   response for click pin code
*/
void CpPinCodePluginView::changePinCode()
{
		RDEBUG("0", 0);
    mSecCodeSettings->changePinCode();
}

/*!
   response for click pin2 code
*/
void CpPinCodePluginView::changePin2Code()
{
		RDEBUG("0", 0);
    mSecCodeSettings->changePin2Code();
}
