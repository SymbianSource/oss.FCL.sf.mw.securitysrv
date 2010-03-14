/*
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Wrapper class for Symbian's Security Framework's calls.
*
*/

#ifndef SECMODUISYNCWRAPPER_H
#define SECMODUISYNCWRAPPER_H

#include <e32base.h>
#include <f32file.h>
#include <ct/rcpointerarray.h>
#include <ct/rmpointerarray.h>

class MCTToken;
class MCTAuthenticationObject;
class MCTAuthenticationObjectList;
class CUnifiedKeyStore;
class MCTTokenInterface;
class CCTKeyInfo;
class TCTKeyAttributeFilter;
class MCTKeyStore;
class TCTTokenObjectHandle;


/**
*  class CSecModUISyncWrapper
*  Wrapper class for Symbian's Security Framework's calls.
*
*  @lib wimmanui.dll
*  @since Series 60 2.0
*/
class CSecModUISyncWrapper : public CActive
    {
    public:     // Constructors and destructor

        static CSecModUISyncWrapper* NewLC();
        static CSecModUISyncWrapper* NewL();
        virtual ~CSecModUISyncWrapper();

    public:     // New functions

        TInt Initialize(CUnifiedKeyStore& aKeyStore );

        TInt OpenWIM();

        TInt GetAuthObjectInterface(
            MCTToken& aToken,
            MCTTokenInterface*& aTokenInterface);

        TInt ListAuthObjects(
            MCTAuthenticationObjectList& aAuthObjList,
            RMPointerArray<MCTAuthenticationObject>& aAuthObjects );

        TInt ListKeys(
            MCTKeyStore& aKeyStore,
            RMPointerArray<CCTKeyInfo>& aKeysInfos,
		    const TCTKeyAttributeFilter& aFilter);

	    TInt DeleteKey(
	        CUnifiedKeyStore& aKeyStore,
	        TCTTokenObjectHandle aHandle);

        TInt ChangeReferenceData( MCTAuthenticationObject& aAuthObject );

        TInt UnblockAuthObject( MCTAuthenticationObject& aAuthObject );

        TInt EnableAuthObject( MCTAuthenticationObject& aAuthObject );

        TInt DisableAuthObject( MCTAuthenticationObject& aAuthObject );

        TInt CloseAuthObject( MCTAuthenticationObject& aAuthObject );

        TInt TimeRemaining( MCTAuthenticationObject& aAuthObject, TInt& aStime );

    protected:      // From CActive

        void DoCancel();
	    void RunL();

    private:        // New functions

        CSecModUISyncWrapper();

    private:        // Data

        // Internal operation states.
        enum TOperation
            {
            EOperationInit,
            EOperationGetAOInterface,
            EOperationListAOs,
            EOperationListKeys,
            EOperationDelKey,
            EOperationChangeReferenceData,
            EOperationUnblockAO,
            EOperationEnableAO,
            EOperationDisableAO,
            EOperationCloseAO,
            EOperationTimeRemAO,
            EOperationNone
            };

        // For wrapping asynchronous calls.
        CActiveSchedulerWait    iWait;

        // Internal state of operation.
        TOperation              iOperation;

        // CT objects
        TAny* iObject;
    };

#endif // SECMODUISYNCWRAPPER_H
