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
* Description: 
*  
*
*/



#ifndef RFSEXAMPLEPLUGIN_H
#define RFSEXAMPLEPLUGIN_H

//  INCLUDES
#include <e32base.h>
#include "rfsPlugin.h"
#include "DevEncKeyUtils.h"

// CLASS DECLARATION

class CDevEncRfsPlugin: public CRFSPlugin
    {
public:

    /**
    * Two-phased constructor.
	* @param	aInitParams initial parameters
    */
    static CDevEncRfsPlugin* NewL(TAny* aInitParams);

    /**
    * Destructor.
    */
    virtual ~CDevEncRfsPlugin();

    /**
    * 
	* @param
    */
    void RestoreFactorySettingsL( const TRfsReason aType );

    /**
    * 
	* @param
    */
    void GetScriptL( const TRfsReason aType, TDes& aPath );

    /**
    * 
	* @param
    */
    void ExecuteCustomCommandL( const TRfsReason aType, TDesC& aCommand );
private:

    /**
    * C++ default constructor.
    */
    CDevEncRfsPlugin();

    /**
    * Constructor.
    * @param	aInitParams initial parameters
	*/
	CDevEncRfsPlugin(TAny* aInitParams);

    /**
    * Constructor
    */
    void ConstructL();

    CDevEncKeyUtils*    iKeyUtils;

    };




#endif      // RFSEXAMPLEPLUGIN_H

// End of File
