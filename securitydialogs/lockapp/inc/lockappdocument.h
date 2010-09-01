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
* Description:  LockApp application document class
 *
*/


#ifndef __LOCKAPP_DOCUMENT_H__
#define __LOCKAPP_DOCUMENT_H__

// INCLUDES
#include <AknDoc.h>

// FORWARD DECLARATIONS
class CLockAppAppUi;
class CEikApplication;

/**
 *  CLockAppDocument class is derived from CAknDocument,
 *  based on standard Avkon document template.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppDocument : public CAknDocument
	{
	public:

		/**
		 * Two-phased constructor.
		 *
		 * @param aApp application class
		 */
		static CLockAppDocument* NewL( CEikApplication& aApp );
		static CLockAppDocument* NewLC( CEikApplication& aApp );

		/**
		 * Destructor
		 */
		~CLockAppDocument( );

	public:

		/*
		 * From @c CAknDocument. Creates AppUi.
		 *
		 * @return generic AppUI object
		 */
		CEikAppUi* CreateAppUiL( );

	private:

		/**
		 * Second constructor that can fail (leave).
		 */
		void ConstructL( );

		/**
		 * C++ default constructor.
		 */
		CLockAppDocument( CEikApplication& aApp );

	};

#endif // __LOCKAPP_DOCUMENT_H__
