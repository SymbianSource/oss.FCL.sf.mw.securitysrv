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

#ifndef CPPINCODEPLUGINVIEW_H
#define CPPINCODEPLUGINVIEW_H

// System includes
#include <cpbasesettingview.h>

// Forward declarations
class SecCodeSettings;
class HbDataFormModelItem;

// Class declaration
class CpPinCodePluginView : public CpBaseSettingView
{
    Q_OBJECT
    
public:
    explicit CpPinCodePluginView(QGraphicsItem *parent = 0);
    virtual ~CpPinCodePluginView();
    
private slots:
    void changePinCode();
    void changePin2Code();
    void changePinCodeRequest();
    
private:
    SecCodeSettings *mSecCodeSettings;
    HbDataFormModelItem *mPinCodeRequestItem;
};

#endif  //CPPINCODEPLUGINVIEW_H
