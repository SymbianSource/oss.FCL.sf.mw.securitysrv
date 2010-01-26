/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Predefined contacts engine (state machine)
*
*/


#ifndef DEVENCSTARTERENGINE_H_
#define DEVENCSTARTERENGINE_H_

#include <e32std.h>						
#include <e32base.h>
#include <e32property.h>

class CDevEncStarterUtils;
class CDevEncStarterMmcObserver;

NONSHARABLE_CLASS( CDevEncStarterEngine ): public CActive
    {
    public:
	/**
	    * Symbian 1st phase constructor
	    * @return Self pointer to CPdcEngine pushed to
	    * the cleanup stack.
	    */
	    static CDevEncStarterEngine* NewLC();

	    /**
	    * Destructor.
	    */
	    virtual ~CDevEncStarterEngine();
	
    private: // C++ constructor and the Symbian second-phase constructor
        CDevEncStarterEngine();
        void ConstructL();
	    
    private: // From CActive
        void DoCancel();
    	void RunL();
    	
    private:
    	CDevEncStarterUtils* iUtils; // owned
    	CDevEncStarterMmcObserver* iMmcObserver; // owned
    	RProperty iProperty;
    };

#endif /* DEVENCSTARTERENGINE_H_ */
