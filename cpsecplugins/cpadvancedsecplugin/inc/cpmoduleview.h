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

#ifndef CPMODULEVIEW_H
#define CPMODULEVIEW_H

#include <cpbasesettingview.h>
#include "cpsecmodview.h"

class HbAbstractViewItem;
class CpItemDataHelper;
class CSecModUIModel;
class CpSecModView;

class CpModuleView : public CpBaseSettingView
{
    Q_OBJECT
public:
    explicit CpModuleView( 	CpSecModView::TSecModViews currentView, 
							CSecModUIModel& secModUIModel,
							QGraphicsItem *parent = 0 );
    virtual ~CpModuleView();
    
private slots:
    void handleAccessView( const QModelIndex& modelIndex );
	void handleAccessView();
	void handleSigView();
	void handleSigViewCommand();
	void indicateLongPress(HbAbstractViewItem *item,QPointF coords);
   
private:
    void showAccessView();
    void showSignatureView(TBool showBlockedNote);

private:
    enum TSecModAccessCodeCommandIds
        {
        EIndexCodeLabel = 0,
        EIndexCodeRequest,
        EIndexCodeStatus 
        };
    
    enum TSecModSignatureCommandIds
        {
        ESecModUICmdOpen = 0,
        ESecModUICmdModuleInfo,
        ESecModUICmdDelKeyStore,
        ESecModUICmdChange,
        ESecModUICmdUnblock,
        ESecModUICmdClose,    
        ESecModUICmdAppTest    
        };

private:
    CpSecModView::TSecModViews mCurrentView;
    CSecModUIModel& mSecModUIModel;
    TInt mPos;
};

#endif /* CPMODULEVIEW_H */
