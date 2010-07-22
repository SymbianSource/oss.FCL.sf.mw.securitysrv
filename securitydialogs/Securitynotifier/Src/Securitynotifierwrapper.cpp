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
* Description:  Securitynotifierwrapper acts as a proxy between
*               clients and Securitynotifierappserver. 
*
*/


#include <ecom/implementationproxy.h>
#include <e32notif.h>
#include <AknNotifierWrapper.h> // link against aknnotifierwrapper.lib

#define KMyNotifierUid TUid::Uid(0x10005988) //  uid
#define KMyPriority TInt(MEikSrvNotifierBase2::ENotifierPriorityHigh)
#define KMyAppServerUid TUid::Uid(0x102071FB)
 
void CleanupArray(TAny* aArray)
    {
    CArrayPtrFlat<MEikSrvNotifierBase2>*     
        subjects=static_cast<CArrayPtrFlat<MEikSrvNotifierBase2>*>(aArray);
    TInt lastInd = subjects->Count()-1;
    for (TInt i=lastInd; i >= 0; i--)
        subjects->At(i)->Release();	    	    
    delete subjects;
    }

CArrayPtr<MEikSrvNotifierBase2>* DoCreateNotifierArrayL()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYNOTIFIER)WRAPPER DoCreateNotifierArrayL BEGIN"));
	#endif
    CArrayPtrFlat<MEikSrvNotifierBase2>* subjects=
        new (ELeave)CArrayPtrFlat<MEikSrvNotifierBase2>(3);
    
    CleanupStack::PushL(TCleanupItem(CleanupArray, subjects));

    // Create Wrappers

    // Session owning notifier(if default implementation is enough)
    CAknCommonNotifierWrapper* master = 
        CAknCommonNotifierWrapper::NewL( KMyNotifierUid,
                                   KMyNotifierUid,
                                   KMyPriority,
                                   _L("SecurityNotifier.dll"),
                                   1, // we don't use synch reply
                                   ETrue); // preload library once app server is up and running 
	  	
	   
    subjects->AppendL( master );
    CleanupStack::Pop();	// array cleanup
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECURITYNOTIFIER)WRAPPER DoCreateNotifierArrayL END"));
	#endif
    
    return(subjects);
    }


CArrayPtr<MEikSrvNotifierBase2>* NotifierArray()
    // old Lib main entry point
    {
    CArrayPtr<MEikSrvNotifierBase2>* array = 0;
    TRAP_IGNORE(array = DoCreateNotifierArrayL()); 	
    return array;
    }

const TImplementationProxy ImplementationTable[] =
	{
#ifdef __EABI__
	{{0x102071fa},(TFuncPtr)NotifierArray}
#else
	{{0x102071fa},NotifierArray}
#endif
	};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy) ;
	return ImplementationTable;
	}
