/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
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


#ifndef __AUTOLOCKVIEW_H__
#define __AUTOLOCKVIEW_H__

#include <aknview.h>
#include "AutolockContainer.h"

class CAutolockView : public CAknView
	{
	public:
		/**
        * Symbian OS constructor.
        */
		void ConstructL();
		/**
        * Destructor.
        */
		~CAutolockView();
	public: // from CAknView
		/**
        * Returns view id.
        * @return An unsigned integer (view id).
        */
        TUid Id() const;
		/**
        * Handles commands.
        * @param aCommand Command to be handled.
        * @return void.
        */
		void HandleCommandL(TInt aCommand);
		/**
        * Handles statuspane changes
        * @return void.
        */
		void HandleStatusPaneSizeChange();
		/**
        * Handles screen layout changes
        */
        void ScreenDeviceChanged();
        void MakeVisible(TBool aVisibility);
        void HandleCall(TInt aCommand, TRect &aRect );
	private: // from CAknView
		/**
        * Activates the view.
        * @param aPrevViewId ID of previous view
        * @param aCustomMessageId customized message ID
        * @param aCustomMessage sutomized message payload
        */
		void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,const TDesC8& aCustomMessage);
		/**
        * Deactivates view
        */
		void DoDeactivate();
	private: // data
		CAutolockContainer* iView;
		};
#endif 
// end of file
