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

#ifndef CPCERTVIEW_H
#define CPCERTVIEW_H

#include <cpbasesettingview.h>

class HbAbstractViewItem;
class CpCertDataContainer;
class CCTCertInfo;
class HbListView;
class HbAction;
class HbDialog;
class HbMessageBox;
class CpCertView : public CpBaseSettingView
	{
	Q_OBJECT
	
	public:
		explicit CpCertView(const QModelIndex& index, QGraphicsItem *parent = 0);
		virtual ~CpCertView();
	
	public:
		enum TCertificateViews
			{
			EAuthorityView=0,
			ETrustedView,
			EPersonalView,
			EDeviceView			
			};
		
	public slots:
		void displayPreviousView();
		
	private slots:
		void openCertificate();
		void openCertFromList(const QModelIndex& modelIndex);
		void showTrustSettings();
		void saveTrustSettings();
		void indicateLongPress(HbAbstractViewItem*, QPointF);
			
		void deleteCertificate();
		TInt refreshListL();
		void refreshView( TInt aCount );
		
		void moveSelectedCert();
		void deleteList();  
		void selectAll();
		void moveCert();
		void viewDone();
		void handleMultipleDelete(HbAction* action);
		void handleMoveDialog(HbAction* action);
		void handleDeleteDialog(HbAction* action);
		void handleMoveCertDialog(HbAction* action);
	private:
		const CCTCertInfo* certAt(TInt index) const;
		QString certLabel(TInt index) const;
		void deleteCertsL( RArray<TInt>& indexList );
		void setDetails(CpCertView::TCertificateViews currentView);
		void moveCertList( RArray<TInt>& indexList );

	private:
		CpCertView::TCertificateViews mCertView;
		TInt mPos;
		CpCertDataContainer* mCertDataContainer;
		TBool mSelectAll;
		RArray<TInt> mSelectionIndex;
		RArray<TInt> mIndexList;
		
		HbDialog* mPopup;
		HbView* mPrevView;
		HbView* mCurrentView;
		CpBaseSettingView* mRefreshedView;
		HbListView* mListView;	
		HbListView* mSelectAllView;	
		HbView* mOriginalView;
		HbMessageBox* mNote;
		HbMenu* mContextMenu;
	};

#endif /* CPCERTVIEW_H */
