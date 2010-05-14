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

#ifndef CPSECMODULEINFOIEW_H
#define CPSECMODULEINFOIEW_H

#include <cpbasesettingview.h>



class CpSecmoduleInfoView : public CpBaseSettingView
{
    Q_OBJECT
public:
    explicit CpSecmoduleInfoView(const QVector< QPair<QString,QString> >& securityDetails,QGraphicsItem *parent = 0);
    virtual ~CpSecmoduleInfoView();
		
public slots:

    
private:


};

#endif // CPSECMODULEINFOIEW_H
