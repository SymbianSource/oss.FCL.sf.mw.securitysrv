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
    HbDataForm *form = qobject_cast<HbDataForm*>(widget());
    if (form) {
        QList<HbAbstractViewItem *> protoTypeList = form->itemPrototypes();
        protoTypeList.append(new SecCodeEditDataFormViewItem());
        form->setItemPrototypes(protoTypeList);     
        form->setHeading(tr("PIN code"));

        HbDataFormModel *formModel = new HbDataFormModel();
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

        form->addConnection(mPinCodeRequestItem, SIGNAL(clicked()), this,
        		SLOT(changePinCodeRequest()));
        formModel->appendDataFormItem(mPinCodeRequestItem);

        HbDataFormModelItem *pinCodeItem = new HbDataFormModelItem(
            static_cast<HbDataFormModelItem::DataItemType>
            (SecCodeEditDataFormViewItem::SecCodeEditItem), tr("PIN code"));
        pinCodeItem->setContentWidgetData("echoMode", HbLineEdit::Password);
        pinCodeItem->setContentWidgetData("text", "1111");
        pinCodeItem->setContentWidgetData("readOnly", true);
        form->addConnection(pinCodeItem, SIGNAL(clicked()), this,
        		SLOT(changePinCode()));
        formModel->appendDataFormItem(pinCodeItem);

        HbDataFormModelItem *pin2CodeItem = new HbDataFormModelItem(
            static_cast<HbDataFormModelItem::DataItemType>
            (SecCodeEditDataFormViewItem::SecCodeEditItem), tr("PIN2 code"));
        pin2CodeItem->setContentWidgetData("echoMode", HbLineEdit::Password);
        pin2CodeItem->setContentWidgetData("text", "1111");
        pin2CodeItem->setContentWidgetData("readOnly", true);
        form->addConnection(pin2CodeItem, SIGNAL(clicked()), this,
        		SLOT(changePin2Code()));
        formModel->appendDataFormItem(pin2CodeItem);

        form->setModel(formModel);
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
   response for click pin code
*/
void CpPinCodePluginView::changePinCode()
{
    mSecCodeSettings->changePinCode();
}

/*!
   response for click pin2 code
*/
void CpPinCodePluginView::changePin2Code()
{
    mSecCodeSettings->changePin2Code();
}

/*!
   response for click pin code request
*/
void CpPinCodePluginView::changePinCodeRequest()
{
    if (mSecCodeSettings->changePinCodeRequest()) {
        QString text = mPinCodeRequestItem->contentWidgetData("text").toString();
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
    }
}
