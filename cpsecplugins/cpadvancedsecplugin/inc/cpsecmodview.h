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

#ifndef CPSECMODVIEW_H
#define CPSECMODVIEW_H

#include <cpbasesettingview.h>

class HbAbstractViewItem;
class CpItemDataHelper;
class CSecModUIModel;

class CpSecModView : public CpBaseSettingView
{
    Q_OBJECT
public:
    explicit CpSecModView(TInt aCurrentPos, CSecModUIModel& aSecModUIModel, QGraphicsItem *parent = 0);
    virtual ~CpSecModView();
    
private slots:
	void displayPrevious();
	void showNextView( const QModelIndex& modelIndex );
	
public:
    enum TSecModViews
    	{
    	EAccessView=0,
    	ESignatureView
    	};
    
    
private:
    CSecModUIModel& mSecModUIModel;  
    TInt mPos;
    TSecModViews mCurrentView;
    HbView* mAccessView;
    HbView* mPrevView;
 
};

#endif /* CPPLACEHOLDERENTRYITEMDATA_H */
