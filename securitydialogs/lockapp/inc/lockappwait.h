/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Async-Sync utility class
 *
*/


#ifndef __LOCKAPPWAIT_H
#define __LOCKAPPWAIT_H

//  INCLUDES

#include <e32base.h>
#include <eikappui.h>
#include <coecntrl.h>

//  CLASS DEFINITIONS
class CWait : public CActive
	{
public:
	/**
	 * Creates instance of the CWait class.
	 *
	 * @return Returns the instance just created.
	 */
	static CWait* NewL( );

	/**
	 * Destructor.
	 */
	~CWait( );

	/**
	 * Starts waiting for aReqStatus.
	 */
	TInt WaitForRequestL( );

	/**
	 * Sets active request type.
	 */
	void SetRequestType(TInt aRequestType );
	
	/**
	 * Gets active request type.
	 */
	TInt GetRequestType( );

private:
	
	/**
	 * C++ default constructor.
	 */
	CWait( );
	
	/**
	 * Symbian OS constructor.
	 */
	void ConstructL( );

private: // from CActive

	/** @see CActive::RunL() */
	void RunL( );

	/** @see CActive::DoCancel() */
	void DoCancel( );

private: // data

	RTimer iTimer;

	CActiveSchedulerWait iWait;

	// Used if there is a need to cancel an active request;
	// namely in situations where destructor is called when Wait
	// is active.
	TInt iRequestType;
	};

// ----------------------------------------------------------
// CWaitAbsorbingControl
// absorbs all the key presses.
// ----------------------------------------------------------
//
class CWaitAbsorbingControl : public CCoeControl
	{
public:

	/**
	 * Creates instance of the CWaitAbsorbingControl class.
	 *
	 * @return Returns the instance just created.
	 */
	static CWaitAbsorbingControl* NewLC( );

	/**
	 * Destructor.
	 */
	virtual ~CWaitAbsorbingControl( );

private:

	virtual TKeyResponse OfferKeyEventL(const TKeyEvent& /*aKeyEvent*/, TEventCode /*aType*/);

private:

	CWaitAbsorbingControl( );

	void ConstructL( );

private: // data

	CEikAppUi* iAppUi;

	};

#endif

// End of file