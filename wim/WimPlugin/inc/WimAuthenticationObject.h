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
* Description:  An implementation of single authentication object
*
*/


#ifndef CWIMAUTHENTICATIONOBJECT_H
#define CWIMAUTHENTICATIONOBJECT_H

// INCLUDES

#include <mctauthobject.h>
#include <mctkeystore.h>
#include <unifiedcertstore.h>
#include <mctwritablecertstore.h>
#include <unifiedkeystore.h>
#include "WimToken.h"

// FORWARD DECLARATIONS

class CCTKeyInfo;
class CWimPin;

// CLASS DECLARATION

/**
*  Represents one authentication object (for example a pin code)
*
*  @lib   WimPlugin
*  @since Series 60 2.1
*/

class CWimAuthenticationObject : public CActive, public MCTAuthenticationObject
    {

    public: // Two-phased constructor

        /**
        * @param  aToken   A reference to current token
        * @param  aWimPin  A reference to current pin manager
        * @param  aLabel   A reference to label for this object
        * @param  aType    Type of this object
        * @param  aHandle  Handle of this object
        * @param  aObjectStatus  Status of this object
        * @return An instance of this class
        */
        static CWimAuthenticationObject* NewLC( CWimToken& aToken, 
                                                CWimPin& aWimPin,
                                                const TDesC& aLabel,
                                                const TUid aType,
                                                const TInt aHandle,
                                                TUint32 aObjectStatus );

    public: // Functions from base class MCTTokenObject

        /**
        * Returns a reference to this authentication object's 
        * human-readable label.
        * @return A reference to object's label.
        */
        const TDesC& Label() const;

        /**
        * Returns a reference to current token.
        * @return A reference to current token.
        */
        MCTToken& Token() const;

        /**
        * Returns type (UID) of current authentication object. 
        * Must be unique within a token. WIM_PIN_G_UID or WIM_PIN_NR_UID
        * @return UID of the current object.
        */
        TUid Type() const;      

        /**
        * Returns a handle of the authentication object.
        * @return A handle
        *         Must be unique within a token.
        */
        TCTTokenObjectHandle Handle() const;    

    public: // Functions from base class MCTAuthenticationObject

        /**
        * Returns a list of all the objects which this authentication object 
        * protects; for example keys and certificates.
        * The caller of this functions owns all its parameters.
        * @param  aObjects (OUT) The returned objects will be appended 
        *         to this array.
        * @param  aStatus (IN/OUT) Completed with the return code when 
        *         the operation completes.
        *         KErrNone, if no errors detected. Note: if protected
        *         objects are not found, it is also KErrNone
        *         KErrCancel, if call is canceled
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void ListProtectedObjects( RMPointerArray<MCTTokenObject>& aObjects,
                                   TRequestStatus& aStatus );

        /**
        * Cancels an ongoing ListProtectedObjects operation.
        * @return void
        */
        void CancelListProtectedObjects();

        /**
        * Changes the reference data (e.g. PIN value). The security
        * dialog component will prompt for the old and new reference data.
        * @param  aStatus (IN/OUT) Completed with the return code when the 
        *         operation completes.
        *         KErrNone, if no errors detected.
        *         KErrLocked, if this authentication object is blocked
        *         KErrNotSupported, if authentication object reference data 
        *         cannot be changed
        *         KErrBadData, if user entered wrong authentication data
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void ChangeReferenceData( TRequestStatus &aStatus );

        /**
        * Cancels an ongoing ChangeReferenceData operation. 
        * @return void
        */
        void CancelChangeReferenceData();

        /**
        * Unblocks the authentication object. The security dialog component will
        * prompt for the unblocking authentication object.
        * @param  aStatus (IN/OUT) Completed with the return code when the 
        *         operation completes. 
        *         KErrNone, if no errors detected, or authentication object was
        *         not blocked
        *         KErrNotSupported, if authentication object cannot be unblocked
        *         KErrBadData, if user entered wrong authentication data
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void Unblock( TRequestStatus &aStatus );
    
        /**
        * Cancels an ongoing Unblock operation.
        * @return void
        */
        void CancelUnblock();

        /**
        * Gets the status of the authentication object. 
        * @return  See TCTAuthenticationStatus for the format of the 
        *          return value.
        *          When status cannot be fetched for some reasons, this value 
        *          is zero.
        */
        TUint32 Status() const;

        /**
        * Disables the authentication object.
        * @param  aStatus (IN/OUT) Completed with the return code when the 
        *         operation completes.
        *         KErrNone, if no errors detected
        *         KErrLocked, if authentication object is blocked
        *         KErrNotFound, if authentication object cannot be found
        *         KErrBadData, if user entered wrong authentication data
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void Disable( TRequestStatus &aStatus );

        /**
        * Cancels an ongoing Disable operation.
        * @return void
        */
        void CancelDisable();

        /**
        * Enables the authentication object.
        * @param  aStatus (IN/OUT) Completed with the return code when 
        *         the operation completes.
        *         KErrNone, if no errors detected
        *         KErrLocked, if authentication object is blocked
        *         KErrNotFound, if authentication object cannot be found
        *         KErrBadData, if user entered wrong authentication data
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void Enable( TRequestStatus &aStatus );

        /**
        * Cancels an ongoing Enable operation.
        * @return void
        */
        void CancelEnable();

        /** 
        * Opens the authentication object, meaning that the protected objects 
        * can be accessed without provoking the authentication mechanism for 
        * the duration of the timeout period. Note. It is not strictly 
        * necessary to call this function, as the authentication object will 
        * be opened when any operation that needs it to be opened is called, 
        * but it is sometimes useful to control the timing of authentication 
        * dialogs. Note also that this function will do nothing if the 
        * authentication object is open, or if it authentication object requires
        * the authentication.
        * @param aStatus  TRequestStatus from the caller.(IN/OUT)
        *         KErrNone, if no errors detected,  or authentication object
        *         is open
        *         KErrLocked, if authentication object is blocked
        *         KErrNotFound, if authentication object cannot be found
        *         KErrNotSupported, if authentication object is not openable 
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void Open( TRequestStatus& aStatus );

        /** 
        * Closes an authentication object.
        * @param aStatus  TRequestStatus from the caller.(IN/OUT)
        *         KErrNone, if no errors detected or authentication object
        *         is closed.
        *         KErrLocked, if authentication object is blocked
        *         KErrNotSupported, if authentication object is not closeable
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void Close( TRequestStatus& aStatus );

        /** 
        * Returns the amount of time in seconds that the authentication object
        * will remain open for, or 0 if it is closed. Thís call is valid
        * only for authentication object, that can be opened (e.g. PIN-G)
        * @param aStime   Time (in seconds) to be returned. (OUT)
        * @param aStatus  TRequestStatus from the caller. (IN/OUT)
        *         KErrNone, if no errors detected, or authentication object
        *         is closed.
        *         KErrLocked, if authentication object is blocked
        *         KErrNotSupported, if authentication object is not allowed to
        *         be asked for timing information
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void TimeRemaining( TInt& aStime, TRequestStatus& aStatus ); 

        /** 
        * Sets the time in seconds for this authentication
        * object. Permitted values include 0, meaning always ask, and -1,
        * meaning until it's explicitly closed. Particular authentication
        * objects might restrict the range of values permitted. Thís call 
        * is valid only for authentication object, that can be opened 
        * (e.g. PIN-G)
        * @param aTime    (IN) Timeout (in seconds) to be set.
        * @param aStatus  TRequestStatus from the caller. (IN/OUT)
        *         KErrNone, if no errors detected, or authentication object
        *         is closed.
        *         KErrLocked, if authentication object is blocked
        *         KErrArgument, if aTime is not allowed. Permitted values 
        *         include 0, meaning always ask, and -1, meaning until it's 
        *         explicitly closed.
        *         KErrNotSupported, if authentication object is not allowed to
        *         be set time
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void SetTimeout( TInt aTime, TRequestStatus& aStatus ); 

        /** 
        * Gets the current timeout value, in seconds. See SetTimeout for
        * an explanation of the values. Thís call is valid only for 
        * authentication object, that can be opened (e.g. PIN-G)
        * @param aTime  Current timeout (in seconds) to be returned. (OUT)
        * @param aStatus  TRequestStatus from the caller. (IN/OUT)
        *         KErrNone, if no errors detected, or authentication object
        *         is closed.
        *         KErrNotSupported, if authentication object is not allowed to
        *         be queried about timing information
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void Timeout( TInt& aTime, TRequestStatus& aStatus ); 

        /** 
        * Returns more token information than framework's MCTToken.
        * Needed in MCTAuthenticationObjectList interface when creating
        * authentication objects.
        * @return  A reference to CWimToken.
        */
        CWimToken& TokenWider() const;

        /** 
        * Returns a reference to pin module.
        * Needed in MCTAuthenticationObjectList interface when creating
        * authentication objects.
        * @return  A reference to CWimPin.
        */
        CWimPin& PinModule() const;

    protected:  // From base class MCTTokenObject
        
        virtual void DoRelease();

    protected:  // From base class CActive

        void RunL();

        /** 
        * Leaves in RunL are handled here.
        * @param  aError  Leaving code
        * @return An integer that should be KErrNone if leave was handled.
        */
        TInt RunError( TInt aError );
        
        void DoCancel();

    private: // Constructors

        /**
        * Default constructor
        * @param  aToken   A reference to current token
        * @param  aWimPin  A reference to current pin manager
        * @param  aType    Type of this object
        * @param  aHandle  Handle of this object
        * @param  aObjectStatus  Status of this object
        */
        CWimAuthenticationObject( CWimToken& aToken,
                                  CWimPin& aWimPin,
                                  const TUid aType,
                                  const TInt aHandle,
                                  TUint32 aObjectStatus );

        /**
        * Second phase constructor
        * @param  aLabel  A reference to this object's label
        */
        void ConstructL( const TDesC& aLabel );

        // Destructor
        virtual ~CWimAuthenticationObject();

    private: // Own functions

        /** 
        * Frees unified key store resources.
        * @return void
        */
        void FreeUnifiedKeyStore();

        /** 
        * Frees unified cert store resources.
        * @return void
        */
        void FreeUnifiedCertStore();

        /** 
        * Selects keys according to given filter.
        * @return void
        */
        void DoFilterKeys();

        /** 
        * Validates protected certificates
        * @return void
        */
        void DoListCertEntries();

        /** 
        * Constructs final protected objects
        * @return void
        */
        void DoMakeObjectsL();

        /** 
        * General function for creating authentication objects.
        * @param  aHandle  Handle of authentication object
        * @return  A pointer to created object
        */
        MCTAuthenticationObject* MakeAuthObjectL();

        /** 
        * Returns true or false indicating if token is removed or not
        * @return  Boolean true or false
        */
        TBool TokenRemoved();

        // Checks if token is removed or is this object active
        TBool EnteringAllowed( TRequestStatus& aStatus );

    private: // Data

        // State flag for RunL

        enum TPhase
            {
            EGetKeyInfos = 0,
            EFilterKeys,
            EGetCerts,
            EInitCertStore,
            EListCertEntries,
            EMakeObjects,
            EIdle,
            EEnablePinQuery,
            EDisablePinQuery,
            EChangePin,
            EUnblockPin,
            EVerifyPin
            };

        // Flag for internal state machine
        TPhase iPhase;

        // Used for saving caller status 
        // This class don't own the pointed object
        TRequestStatus* iOriginalRequestStatus;

        // A reference to current token. Will be asssigned in construction.
        // This class don't own that referenced object.
        CWimToken& iToken;

        // Pointer to pin manager, this class don't own pointed object.
        CWimPin& iWimPin;

        // A pointer to buffer containing human readable label for 
        // this authentication object. This class owns this buffer.
        HBufC* iLabel;

        // Contains the type of this authentication object.
        // For example WIM_PIN_G_UID or WIM_PIN_NR_UID
        TUid iType;     

        // The handle of authentication object. An integer value
        // running from 0 to (number of authentication objects) - 1
        TInt iObjectId;

        // Holds a status of this authentication object. 
        // See enum TCTAuthenticationStatus in MCTAuthObject.h 
        // for status values.
        TUint32 iObjectStatus;

        // Used with unified key store to save keys for a while
        RMPointerArray<CCTKeyInfo> iKeyInfos;

        // Used with unified key store to filter keys
        TCTKeyAttributeFilter iKeyFilter;   

        // Temporal array for object pointers
        // This array is used to append objects in List operation
        // This class don't own the pointed objects
        RMPointerArray<MCTTokenObject>* iObjects;

        // A pointer to unified key store
        // This class owns the pointed object
        CUnifiedKeyStore* iUnifiedKeyStore;

        // A pointer to unified cert store
        // This class owns the pointed object
        CUnifiedCertStore* iUnifiedCertStore;

        // A pointer to certificate filter object
        // This class owns the pointed object.
        CCertAttributeFilter* iCertFilter;

        // Temporal array for certificates this authentication 
        // object protects.
        RMPointerArray<CCTCertInfo> iCertStoreEntries;

        // A Reference to wider implementation specific token
        // for internal functions.
        CWimToken& iTokenWider;

        // For call to Unified KeyStore, which might have file key store
        RFs iFs;

        // Array of pointers to PIN modules.This class don't own pointed
        // objects.
        const CArrayPtrFlat<CWimPin>* iPinNRs; 
    };


#endif      // CWIMAUTHENTICATIONOBJECT_H   
            
// End of File
