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

#ifndef CPSECURITYVIEW_H
#define CPSECURITYVIEW_H

#include <cpbasesettingview.h>

class HbAbstractViewItem;
class CpItemDataHelper;
class CpSecModView;
class CpCertView;
class CSecModUIModel;
class QModelIndex;
class CpSecmoduleInfoView;
class HbMessageBox;
class CpSecurityView : public CpBaseSettingView
{
    Q_OBJECT
public:
    explicit CpSecurityView(QGraphicsItem *parent = 0);
    virtual ~CpSecurityView();
		
public slots:
    void viewDone();
    void showWIMview();
    void displayCert(const QModelIndex& modelIndex);
    void displayPrevious();
    void showCodeView();
    void showCodeView( const QModelIndex& modelIndex);
    void indicateLongPress(HbAbstractViewItem *item,QPointF coords);
    void deleteModule();
    void moduleDetails();
    void displayPreviousFromModInfo();
    void dialogClosed(HbAction* action);
private:
    CSecModUIModel* mSecModUIModel;  
    CpSecModView* mSecModView;
	HbView* mPreView;
	CpCertView* mCertView;
	CpSecmoduleInfoView* mModuleinfoView;
	HbView* mCurrentView;
	TInt mPos;
	HbMessageBox* mNote;
	HbMenu* mContextMenu;

};

#endif // CPSECURITYVIEW_H
