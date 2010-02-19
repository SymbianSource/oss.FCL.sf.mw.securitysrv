/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  API for managing WIM Security Module.
*
*/


//INCLUDES
#include "WimSecModule.h"
#include "WimConsts.h"
#include "WimPin.h"
#include "WimMgmt.h"
#include "WimTrace.h"

//CONSTANTS
_LIT( KZeroHex,"0%x" );
_LIT( KHex, "%x" );
//Must be const, else this won't compile to Thumb & arm4
const TUint KSerialNumberMask = 0x0F; 
const TUint KPinNum = 0x02;	

//================= MEMBER FUNCTIONS  for CWimSecModule=======================
//
//  C++ default constructor. Initialize the member variables
//
//
CWimSecModule::CWimSecModule( TWimAddress aWimAddr ): CActive( EPriorityHigh ),
                                                      iReference( aWimAddr ),
                                                      iPinNRsInit( EFalse )
    {
    }


// -----------------------------------------------------------------------------
// CWimSecModule::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CWimSecModule* CWimSecModule::NewL( TWimAddress aWimAddr )
    {
    _WIMTRACE ( _L( "CWimSecModule::NewL()" ) );
    __ASSERT_DEBUG( aWimAddr, User::Leave( KErrArgument ) );    
    CWimSecModule* self = new( ELeave ) CWimSecModule( aWimAddr );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self ); //self
    return self;
    }

// -----------------------------------------------------------------------------
// CWimSecModule::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimSecModule::ConstructL()
    {
    CActiveScheduler::Add( this );
    _WIMTRACE ( _L( "CWimSecModule::ConstructL()" ) );
    iLabel = HBufC::NewL( KLabelLen );
    iManufacturer = HBufC::NewL( KManufacturerLen );
    iSerialNumber = HBufC::NewL( KSerialNumberLen );
    iPinInfoLst = new( ELeave )CArrayFixFlat<TWimPinStruct>(KPinNum);
    }

// -----------------------------------------------------------------------------
// CWimSecModule::IsOpen()
// Checks is WIM already open.
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CWimSecModule::IsOpen()
    {
    _WIMTRACE ( _L( "CWimSecModule::IsOpen()" ) );
    return iClientSession->IsOpen( iReference ); 
    }


// -----------------------------------------------------------------------------
// CWimSecModule::CloseAfter()
// Returns the time which is set as WIM closeout time
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWimSecModule::CloseAfter()
    {
    _WIMTRACE ( _L( "CWimSecModule::CloseAfter()" ) );
    return iClientSession->CloseAfter();  
    }

// -----------------------------------------------------------------------------
// CWimSecModule::TimeRemaining()
// Returns the timeout which tells how long WIM Security Module will be open 
// after it is opened.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWimSecModule::TimeRemaining()
    {
    _WIMTRACE ( _L( "CWimSecModule::TimeRemaining()" ) );
    return iClientSession->TimeRemaining( );      
    }

// -----------------------------------------------------------------------------
// CWimSecModule::PinNrEntriesL()
// Returns WIM Security Module's keys' PIN-NR objects in array.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWimSecModule::PinNrEntriesL( 
                                      const CArrayPtrFlat<CWimPin>*& aPinNRs,
                                      TRequestStatus& aStatus ) 
    {
    _WIMTRACE ( _L( "CWimSecModule::PinNrEntriesL()" ) );
    TInt status = KErrNone;
    
    if( !iPinNRsInit )
	    {
	    status = GetPINModulesL( aStatus );	
	    }
    else
        {
    	aPinNRs = iPinNRs;
    	iClientStatus = &aStatus;
    	User::RequestComplete( iClientStatus, KErrNone );
    	return status;
        }
    
    if ( status == KErrNone )
        {
        aPinNRs = iPinNRs;
        }
    else
        {
        aPinNRs = NULL;
        }
    return status;
    }

// -----------------------------------------------------------------------------
// CWimSecModule::PinNrEntriesL()
// Returns WIM Security Module's keys' PIN-NR objects in array.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWimSecModule::PinNrEntriesL( 
                                      const CArrayPtrFlat<CWimPin>*& aPinNRs ) 
    {
    _WIMTRACE ( _L( "CWimSecModule::PinNrEntriesL()" ) );
    TInt status = KErrNone;
   
   if( !iPinNRsInit )
       {
   	   status = GetPINModulesL();
       }
    
    if ( status == KErrNone )
        {
        aPinNRs = iPinNRs;
        }
    else
        {
        aPinNRs = NULL;
        }
    return status;
    }

// -----------------------------------------------------------------------------
// CWimSecModule::Label()
// Returns WIM's label.
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CWimSecModule::Label()
    {
    _WIMTRACE ( _L( "CWimSecModule::Label()" ) );
    TInt status = KErrNone;
    TRAPD( err, status = GetWIMInfoL() );
    if ( status == KErrNone && !err )
        {
        return iLabel->Des();
        }
    else
        {
        return TPtrC( NULL, 0 );
        }
    }


// -----------------------------------------------------------------------------
// CWimSecModule::Manufacturer()
// Returns WIM's manufacturer.
// -----------------------------------------------------------------------------
//
EXPORT_C const TDesC& CWimSecModule::Manufacturer()
    {
    _WIMTRACE ( _L( "CWimSecModule::Manufacturer()" ) );
    TInt status = KErrNone;
    TRAPD( err, status = GetWIMInfoL() );
    if ( status == KErrNone && !err )
        {
        return *iManufacturer;
        }
    else
        {
        return KNullDesC;
        }
    }


// -----------------------------------------------------------------------------
// CWimSecModule::Version()
// Returns WIM's version.
// -----------------------------------------------------------------------------
//
EXPORT_C const TDesC& CWimSecModule::Version()
    { 
    _WIMTRACE ( _L( "CWimSecModule::Version()" ) );
    TInt status = KErrNone;
    TRAPD( err, status = GetWIMInfoL() );
    if ( status == KErrNone && !err )
        {
        return *iVersion;
        }
    else
        {
        return KNullDesC;   
        }
    }


// -----------------------------------------------------------------------------
// CWimSecModule::SerialNumber()
// Returns WIM's serial number which identifies it.
// -----------------------------------------------------------------------------
//
EXPORT_C const TDesC& CWimSecModule::SerialNumber()
    {
    _WIMTRACE ( _L( "CWimSecModule::SerialNumber()" ) );
    TInt status = KErrNone;
    TRAPD( err, status = GetWIMInfoL() );
    if ( status == KErrNone && !err )
        {
        return *iSerialNumber; 
        }
    else
        {
        return KNullDesC;
        }
    }
 


// -----------------------------------------------------------------------------
// CWimSecModule::Close()
// Closes the parts of WIM which is opened with PIN-G.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWimSecModule::Close()
    {
    _WIMTRACE ( _L( "CWimSecModule::Close()" ) );
    
    iWimInfoConstructed = EFalse;
    iPinNRsInit = EFalse;
    
    if ( iReference )
        {
        return iClientSession->CloseWIM( iReference );
        }
    else
        {
        return KWimSMInfoError;
        }
    }

// -----------------------------------------------------------------------------
// CWimSecModule::SetCloseAfter()
// Sets the timeout which defines the time after WIM is automatically closed.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimSecModule::SetCloseAfter( const TUint aTimeout )
    {
    _WIMTRACE ( _L( "CWimSecModule::SetCloseAfter()" ) );
    iClientSession->SetCloseAfter( aTimeout );
    }


// -----------------------------------------------------------------------------
// CWimSecModule::~CWimSecModule()
// Destructor, all allocated memory is released.
// -----------------------------------------------------------------------------
//
EXPORT_C CWimSecModule::~CWimSecModule()
    {
    _WIMTRACE ( _L( "CWimSecModule::~CWimSecModule()()" ) );
    Cancel();
    
    if( iPinInfoLst )
        {
    	iPinInfoLst->Reset();
    	delete iPinInfoLst;
        }
    if ( iPinLstAddr )
        {
        iClientSession->FreeAddrLst( iPinLstAddr );
        }
    if ( iPinNRs )
        {
        iPinNRs->ResetAndDestroy();/* Destroy  */
        delete iPinNRs;    /* Array of pointers to PIN-NRs. */
        }
    if ( iRefPing )
        {
        iClientSession->FreeWIMAddr( iRefPing );
        }
    delete iLabel;
    delete iManufacturer;
    delete iSerialNumber;
    if ( iVersion )
        {
        delete iVersion;
        iVersion = NULL;
        }
   
    }

// -----------------------------------------------------------------------------
// CWimSecModule::TokenNumber()
// Returns TUint8 iReader which identifies the slot 
// where the WIM card is actually.
// -----------------------------------------------------------------------------
//
EXPORT_C TUint8 CWimSecModule::TokenNumber()
    {
    _WIMTRACE ( _L( "CWimSecModule::TokenNumber()" ) );
    return iReader;
    }

// -----------------------------------------------------------------------------
// CWimSecModule::NotifyOnRemoval()
// Notifies the client when the token has been removed.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimSecModule::NotifyOnRemoval( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimSecModule::NotifyOnRemoval()" ) );

    TIpcArgs args;
    iClientSession->SendReceiveData( ENotifyOnRemoval, args, aStatus );
    }

// -----------------------------------------------------------------------------
// CWimSecModule::CancelNotifyOnRemoval()
// Cancels the NotifyOnRemoval request.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimSecModule::CancelNotifyOnRemoval()
    {
    _WIMTRACE ( _L( "CWimSecModule::CancelNotifyOnRemoval()" ) );

    TIpcArgs args;
    iClientSession->SendReceiveData( ECancelNotifyOnRemoval, args );
    }

// -----------------------------------------------------------------------------
// CWimSecModule::GetWIMInfoL()
// Gets label,manufacturer,serialnumber and the number of the slot 
// where card is.
// -----------------------------------------------------------------------------
//
TInt CWimSecModule::GetWIMInfoL()
    {
    _WIMTRACE ( _L( "CWimSecModule::GetWIMInfoL()" ) );

    TBuf<KVersionNumberLen> temporalVersionNum;   

    TInt status = KErrNone;
    if ( !iWimInfoConstructed )
        {
        HBufC* temporalSerialNumber = HBufC::NewLC( KSerialNumberLen );                
        TWimSecModuleStruct wimSecModuleStruct;
        TPtr label = iLabel->Des();
        TPtr manufacturer = iManufacturer->Des();
        TPtr serialnumber = temporalSerialNumber->Des();
        wimSecModuleStruct.iLabel = label;
        wimSecModuleStruct.iManufacturer = manufacturer;
        wimSecModuleStruct.iSerialNumber = serialnumber;
        status = iClientSession->WIMInfo( iReference, wimSecModuleStruct );
        if ( status == KErrNone ) 
            {
            temporalVersionNum.Num( ( TInt )wimSecModuleStruct.iVersion );
            iVersion = temporalVersionNum.AllocL();
            iReader = wimSecModuleStruct.iReader;
            label.Copy( wimSecModuleStruct.iLabel );
            manufacturer.Copy( wimSecModuleStruct.iManufacturer );
            serialnumber.Copy( wimSecModuleStruct.iSerialNumber );
            label.ZeroTerminate();
            manufacturer.ZeroTerminate();
            serialnumber.ZeroTerminate();
            iRefPing = wimSecModuleStruct.iRefPinG;
            
            //status = GetPINModulesL( aStatus );
            
            if ( status == KErrNone )
                {
                if ( iReader != KSoftId )
                    {
                    ConvertSerialNumberL( temporalSerialNumber );                    
                    }
                else
                    {
                    TBuf<KVersionNumberLen> newSerialNumber;
                    TPtr ptr = temporalSerialNumber->Des();
                    newSerialNumber.Copy( ptr ); 
                    delete iSerialNumber;
                    iSerialNumber = NULL;
                    iSerialNumber = newSerialNumber.AllocL();
                    }
                iWimInfoConstructed = ETrue;                
                }
            }                       //temporalSerialNumber
        CleanupStack::PopAndDestroy( temporalSerialNumber );               
        }
    return status;
    }

// -----------------------------------------------------------------------------
// CWimSecModule::ConvertSerialNumberL()
// Converts serialnumber. 
// -----------------------------------------------------------------------------
//
void CWimSecModule::ConvertSerialNumberL( HBufC*& aSerialNumber )
    {
    _WIMTRACE ( _L( "CWimSecModule::ConvertSerialNumberL()" ) );

    TBuf<KVersionNumberLen> orginalVersionNumber;
    TBuf<KVersionNumberLen> newSerialNumber;
    TPtr ptr = aSerialNumber->Des();
    orginalVersionNumber.Copy( ptr );
    TInt len = orginalVersionNumber.Length(); 
    TInt i;
    TUint item;
    for ( i = 0; i<len; i++ )
        {
        item = orginalVersionNumber.operator[]( i ); 
        if ( item <= KSerialNumberMask )
            {
            //In this case we need to add an extra zero before actual value.
            newSerialNumber.AppendFormat( KZeroHex, 
                TUint( orginalVersionNumber.operator[]( i ) ) );
            }
        else
            {
            //No need for further processing, simply append. 
            newSerialNumber.AppendFormat( KHex, 
                TUint( orginalVersionNumber.operator[]( i ) ) );            
            }
        }
    delete iSerialNumber;
    iSerialNumber = NULL;
    iSerialNumber = newSerialNumber.AllocL();
    }

// -----------------------------------------------------------------------------
// CWimSecModule::GetPINModulesL( ) Asynchronous
// Gets PIN references and creates new CWimPin objects.
// -----------------------------------------------------------------------------
//
TInt CWimSecModule::GetPINModulesL( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimSecModule::GetPINModulesL()" ) );

    TInt status = KErrNone;
    TUint pinCount= iClientSession->PINCount( iReference );
    iPinCount = pinCount;
    iPinInfoLst->Reset();
    
    if ( pinCount )
        {
        TPinAddress* pinLst = new( ELeave ) TPinAddress[ pinCount ];
        CleanupStack::PushL( TCleanupItem( CleanupPinAddressList, pinLst ) );
        
        status = iClientSession->PINRefs( iReference, iPinLstAddr, pinLst,
                                        ( TText8 )pinCount );
        if ( status == KErrNone ) 
            {
            if( iPinNRs == NULL )
	            {
	            iPinNRs = new( ELeave ) CArrayPtrFlat<CWimPin>( pinCount );	
	            }
            else
	            {
	            iPinNRs->Reset();
	            iPinNRs->ResizeL( pinCount );	
	            }
            /* Construct PIN  modules */
            TUint8 index;
            for ( index = 0; index < pinCount; index++ )
                {
                CWimPin* pinModule = CWimPin::NewL( EWimPinNR, 
                                                    pinLst[index],
                                                    *iLabel );
                CleanupStack::PushL( pinModule );
                pinModule->SetClientSession( iClientSession );
                iPinNRs->AppendL( pinModule );
                CleanupStack::Pop( pinModule );/* Pop pinModule  */
                }
              
            if(iPinInfoLst->Count() != pinCount )
                {
            	iPinInfoLst->ResizeL( pinCount );
                }
            
            iClientStatus = &aStatus;
            
            iStatus = KRequestPending;
            
            SetActive();
 
            iClientSession->PINsInfo( iReference, *iPinInfoLst,(TText8)pinCount, iStatus);
             
            }
        
        CleanupStack::PopAndDestroy( pinLst );/* pinLst */   
        
        }
    return status;
    }



// -----------------------------------------------------------------------------
// CWimSecModule::GetPINModulesL( ) Synchronous
// Gets PIN references and creates new CWimPin objects.
// -----------------------------------------------------------------------------
//
TInt CWimSecModule::GetPINModulesL()
    {
    _WIMTRACE ( _L( "CWimSecModule::GetPINModulesL()" ) );

    TInt status = KErrNone;
    TUint pinCount= iClientSession->PINCount( iReference );
    iPinCount = pinCount;
    iPinInfoLst->Reset();
    
    if ( pinCount )
        {
        TPinAddress* pinLst = new( ELeave ) TPinAddress[ pinCount ];
        CleanupStack::PushL( TCleanupItem( CleanupPinAddressList, pinLst ) );
        
        status = iClientSession->PINRefs( iReference, iPinLstAddr, pinLst,
                                        ( TText8 )pinCount );
        if ( status == KErrNone ) 
            {
            if( iPinNRs == NULL )
	            {
	            iPinNRs = new( ELeave ) CArrayPtrFlat<CWimPin>( pinCount );	
	            }
            else
	            {
	            iPinNRs->Reset();
	            iPinNRs->ResizeL( pinCount );	
	            }
            /* Construct PIN  modules */
            TUint8 index;
            for ( index = 0; index < pinCount; index++ )
                {
                CWimPin* pinModule = CWimPin::NewL( EWimPinNR, 
                                                    pinLst[index],
                                                    *iLabel );
                CleanupStack::PushL( pinModule );
                pinModule->SetClientSession( iClientSession );
                iPinNRs->AppendL( pinModule );
                CleanupStack::Pop( pinModule );/* Pop pinModule  */
                }
            
            if(iPinInfoLst->Count() != pinCount )
                {
            	iPinInfoLst->ResizeL( pinCount );
                }
          
          TInt err = iClientSession->PINsInfo( iReference, *iPinInfoLst,(TText8)pinCount );
           
          if( err != KErrNone )
             {
             User::LeaveIfError( KErrGeneral );	
             }
          
          for(TInt i=0; i<iPinCount; i++ )
	          {
	          ( *iPinNRs )[ i]->SetLabel( (*iPinInfoLst)[ i].iLabel );
	    	  ( *iPinNRs )[ i]->SetPinStatus( (*iPinInfoLst)[ i].iStatus );
	          ( *iPinNRs )[ i]->SetPinNumber( (*iPinInfoLst)[ i].iPinNumber );	
	          }
            
            iPinNRsInit = ETrue;
            iPinInfoLst->Reset();    
            }
            
        CleanupStack::PopAndDestroy( pinLst );/* pinLst */
        }
    return status;
    }



// -----------------------------------------------------------------------------
// CWimSecModule::CleanupPinAddressList()
// Handles cleanup of an TPinAddress array
// -----------------------------------------------------------------------------
//
void CWimSecModule::CleanupPinAddressList( TAny* aObject )
    {
    _WIMTRACE ( _L( "CWimSecModule::Cleanup()" ) );
    TPinAddress* pinLst = static_cast< TPinAddress* >( aObject );
    delete[] pinLst;
    aObject = NULL;
    }

// -----------------------------------------------------------------------------
// CWimSecModule::SetClientSession()
// Sets pointer RWimMgmt object to modifier iClientSession
// -----------------------------------------------------------------------------
//
void CWimSecModule::SetClientSession( RWimMgmt* aClientSession )
    {
    _WIMTRACE ( _L( "CWimSecModule::SetClientSession()" ) );
    iClientSession = aClientSession;
    }


// -----------------------------------------------------------------------------
// CWimSecModule::RunL()
// -----------------------------------------------------------------------------
//
void CWimSecModule::RunL()
    {
    _WIMTRACE ( _L( "CWimSecModule::RunL()" ) );
    
    if( iStatus.Int() != KErrNone )
        {
    	return;
        } 
        
    for(TInt i=0; i<iPinCount; i++ )
        {
    	( *iPinNRs )[ i]->SetLabel( (*iPinInfoLst)[ i].iLabel );
    	( *iPinNRs )[ i]->SetPinStatus( (*iPinInfoLst)[ i].iStatus );
        ( *iPinNRs )[ i]->SetPinNumber( (*iPinInfoLst)[ i].iPinNumber );
        }
    
    iPinNRsInit = ETrue;
     
    iPinInfoLst->Reset();    
    User::RequestComplete( iClientStatus, KErrNone );
    }
        
// -----------------------------------------------------------------------------
// CWimSecModule::DoCancel()
// -----------------------------------------------------------------------------
//
void CWimSecModule::DoCancel()
    {
	_WIMTRACE ( _L( "CWimSecModule::DoCancel() " ) );
	User::RequestComplete( iClientStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// CWimSecModule::RunError()
// -----------------------------------------------------------------------------
//
TInt CWimSecModule::RunError( TInt aError )
    {
	_WIMTRACE ( _L( "CWimSecModule::RunError() " ) );
	User::RequestComplete( iClientStatus, aError );
	return KErrNone;
    }

// End of File
