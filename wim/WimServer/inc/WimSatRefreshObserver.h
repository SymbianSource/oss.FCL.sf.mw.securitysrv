/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Observes 'SIM file changed' events from SAT. If changed file
*               is WIM related, WimiLib's cache is cleared and WimiLib closed.
*               Prevents WIM from operating with out-dated cache.
*
*/


#ifndef CWIMSATREFRESHOBSERVER_H
#define CWIMSATREFRESHOBSERVER_H

//  INCLUDES
#include <e32base.h>
#include <msatrefreshobserver.h>

// FORWARD DECLARATIONS
class RSatRefresh;
class RSatSession;
class CWimServer;

#if defined SAT_REFRESH_TIMER_TRIGGER
class CSatRefreshTimerTrigger;
#endif

// CLASS DECLARATION

/**
*  Notifies WimServer when SAT refresh notifications arrive.
*
*  @since 3.0
*/
class CWimSatRefreshObserver : public CBase, public MSatRefreshObserver
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CWimSatRefreshObserver* NewL();

        /**
        * Destructor.
        */
        ~CWimSatRefreshObserver();

    public: // Functions from base classes

		/**
		* From MSatRefreshObserver
		* @param aType is not used, because we lose file path and thus have to
		   react for every refresh request
		* @param aFiles is not used, because we lose file path and thus have to
		   react for every refresh request
		* @return boolean whether we allow refresh or not
		*/
        TBool AllowRefresh( TSatRefreshType /*aType*/,
                            const TSatRefreshFiles& /*aFiles*/ );

		/**
		* From MSatRefreshObserver.
		* @param aType is not used, because we lose file path and thus have to
		   react for every refresh request
		* @param aFiles is not used, because we lose file path and thus have to
		   react for every refresh request
		* @return void
		*/
		void Refresh( TSatRefreshType /*aType*/, 
		              const TSatRefreshFiles& /*aFiles*/ );

    private: // New functions

        /**
        * C++ default constructor.
        */
        CWimSatRefreshObserver();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private: // Data

        /// SAT refresh event client
		RSatRefresh* iRefreshClient;
        /// SAT session object
		RSatSession* iSatSession;
        /// Pointer to WimServer.
		CWimServer* iWimSvr;
#if defined SAT_REFRESH_TIMER_TRIGGER
		/// Simulated Sat Event trigger
		CSatRefreshTimerTrigger* iTimerTrigger;
#endif
    };

#endif      // CWIMSATREFRESHOBSERVER_H

// End of File
