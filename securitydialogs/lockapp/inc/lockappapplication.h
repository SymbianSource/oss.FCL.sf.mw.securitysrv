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
* Description:  Application class that also creates the appserver
 *
*/


#ifndef __LOCKAPP_APPLICATION_H__
#define __LOCKAPP_APPLICATION_H__

// INCLUDES
#include <aknapp.h>

/**
 *  CLockAppApplication class, is the application part of the Avkon 
 *  framework.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppApplication : public CAknApplication
	{
	public:

		/**
		 * @return LockApp application Uid.
		 */
		TUid AppDllUid( ) const;

	protected:

		CApaDocument* CreateDocumentL( );

	};

#endif // __LOCKAPP_APPLICATION_H__
