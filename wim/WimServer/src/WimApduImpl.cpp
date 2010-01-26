/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Apdu handling between WimServer and Scard
*
*/



// INCLUDE FILES
#include    "WimApduImpl.h"
#include    "WimConsts.h"   // Error codes
#include    "WimDefs.h"
#include    "ScardBase.h"
#include    "ScardConnectionRequirement.h"
#include    "ScardComm.h"
#include    "ScardReaderQuery.h"
#include    "WimUtilityFuncs.h"
#include    "WimTrace.h"

//APDU for retrieving response from WIM-card.
TUint8 iResponseAPDU[] = {0x80, 0xc0, 0x00, 0x00, 0x00};


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CWimApdu::CWimApdu() : CActive( EPriorityStandard )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::CWimApdu | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimApdu::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimApdu* CWimApdu::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::NewL | Begin"));
    CWimApdu* self = new( ELeave ) CWimApdu();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimApdu::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimApdu::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::ConstructL | Begin"));
    CActiveScheduler::Add( this );
    iReaderComm = new( ELeave ) CArrayFixFlat<CScardComm*>( 1 );
    //Reserve space for each possible reader.
    iReaderComm->SetReserveL( KMaxReaderCount );
    iReaderNames = new( ELeave ) CArrayFixFlat<TScardReaderName>( 1 );
    //Reserve space for each possible reader.
    iReaderNames->SetReserveL( KMaxReaderCount );
    iServer = RScard::NewL();
    iResponseBuffer = HBufC8::NewL( KMaxApduLen );
    iWimScardListenerArray = new( ELeave ) CArrayPtrFlat<CWimScardListener>(1);
    }

// Destructor
CWimApdu::~CWimApdu()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::~CWimApdu | Begin"));
    
    Cancel();
    delete iResponseBuffer;
    delete iReaderNames;

    if ( iReaderComm )
        {
        for ( TUint8 i( 0 ); i < iReaderComm->Count(); i++ )
            {
            delete (*iReaderComm)[i];
            (*iReaderComm)[i] = 0;
            }
        delete iReaderComm; 
        }

    delete iServer;

    if ( iWimScardListenerArray )
        {
        iWimScardListenerArray->ResetAndDestroy();
        delete iWimScardListenerArray;
        }
    }
                  
// -----------------------------------------------------------------------------
// CWimApdu::RequestList
// List readers and get cababilities of each card
// -----------------------------------------------------------------------------
//
TUint8 CWimApdu::RequestListL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::RequestListL | Begin"));
    CScardReaderQuery* query = NULL;  //Class used to list resources 
                                   //owned by Smart Card Server.
    CArrayFixFlat<TScardReaderName>* readerNames = NULL;
    CArrayFixFlat<TScardReaderName>* groupNames = NULL;
    TInt8 position = 0;
    TInt error;

    //Reset member data associated with query data.
    iReaderStatusLength = 0;
    iReaderStatusses.Delete( 0, KMaxReaderCount );
    iReaderStatusses.Zero();

    TUint8 i;
    for ( i = 0; i < iReaderComm->Count(); i++ )
        {
        delete (*iReaderComm)[i];
        (*iReaderComm)[i] = NULL;
        }
    iReaderComm->Delete( 0, iReaderComm->Count() );
    iReaderNames->Delete( 0, iReaderNames->Count() );

    TRAP(
        error,
        query = CScardReaderQuery::NewL();
        readerNames = 
            new( ELeave ) CArrayFixFlat<TScardReaderName>( KMaxReaderCount );
        groupNames =
            new( ELeave ) CArrayFixFlat<TScardReaderName>( KMaxReaderCount );
        query->ListGroupsL( groupNames )
        );

    if ( error )
        {
        delete query;
        readerNames->Reset();
        delete readerNames;
        groupNames->Reset();
        delete groupNames;

        iResponseStatus = KWimApduNoMemory;

        //any return value <> 0 means error by WIMlib specs
        return KWimStatusIOError;
        }

    // Get readers from all groups and add them to readerNames.
    for ( i = 0; i < groupNames->Count(); i++ )
        {
        //Get reader names from this group.
        TRAP( error, query->ListReadersL( readerNames, (*groupNames)[i] ) );
        
        if ( error )
            {
            delete query;
            readerNames->Reset();
            delete readerNames;
            groupNames->Reset();
            delete groupNames;

            iResponseStatus = KWimApduNoMemory;

            return KWimStatusIOError;
            }

        //Add all reader names found from this group to iReaderNames.
        for ( TInt8 k = 0; k < readerNames->Count(); k++ )
            {
            //Should do something if maxreadernames == readerNames->Count()!
            TRAP( error, iReaderNames->AppendL( (*readerNames)[k] ) );

            if ( error )
                {
                delete query;
                readerNames->Reset();
                delete readerNames;
                groupNames->Reset();
                delete groupNames;

                iResponseStatus = KWimApduNoMemory;

                return KWimStatusIOError;
                }
            position++;
            }
        }

    //Create CScardComm object for each found reader.
    for ( TUint8 j = 0; j < iReaderNames->Count(); j++ )
        {
        TBuf8<1> tempStatus;        //Buffer used to hold 1 status byte.
        TScardConnectionRequirement rest; //Connection requirements used in
                                          //the creation of CScardComm object.
        TScardReaderName name;

        CScardComm* tempComm = NULL;
        //Set rest to explicit value (name of the current reader) and
        //create CScardComm according to these values. Also check that
        //the found reader name (name) is really what we were looking for
        //(is this needed?). If everything is ok, add created CScardComm
        //to iReaderComm, else NULL.
        iStatus = KRequestPending;
        TRAP( error,
            rest.SetExplicitL( (*iReaderNames)[j] );
            tempComm = CScardComm::NewL( iServer, rest, name, iStatus )
            );
        
        SetActiveAndWait();

        if ( error || iStatus.Int() != KErrNone || ( name != (*iReaderNames)[j] ) )
            {
            iReaderComm->AppendL( NULL );
            _WIMTRACE2(_L("WIM | WIMServer | CWimApdu::RequestListL | CScardComm::NewL leaved with %d"), error);
             
             delete tempComm;
             tempComm = NULL;
             delete query;
             readerNames->Reset();
             delete readerNames;
             groupNames->Reset();
             delete groupNames;
            
            User::Leave ( KErrHardwareNotAvailable );
            }
        else    //all ok.
            {
            TRAP( error,
                iReaderComm->AppendL( tempComm )
                );

            if ( error )
                {
                delete tempComm;
                tempComm = NULL;
                iReaderComm->AppendL( NULL );
                }
            else    //all ok.
                {
                //Get capabilities of the card. With this value GetCapabilities
                //puts status to tempStatus.
                TRAP( error,
                    (*iReaderComm)[j]->GetCapabilitiesL( KCardStatus,
                    tempStatus ) );
                _WIMTRACE3(_L("WIM | WIMServer | CWimApdu::RequestListL | GetCapabilitiesL error = %d, Card status = %d"), error, (TUint8)*tempStatus.Ptr());
                _WIMTRACE2(_L("WIM|WIMServer|CWimApdu::RequestListL|tempStatus.Size() = %d"), tempStatus.Size());

                if ( error == KErrNoMemory )
                    {
                    delete query;
                    readerNames->Reset();
                    delete readerNames;
                    groupNames->Reset();
                    delete groupNames;

                    iResponseStatus = KWimApduNoMemory;
                    return ( TUint8 )KErrCouldNotConnect;
                    }
     
                else if ( error == KErrNone && tempStatus.Size() && *tempStatus.Ptr() )
                    {
                    iReaderStatusLength++;
                    iReaderStatusses.Append( tempStatus );
                    _WIMTRACE(_L("WIM|WIMServer|CWimApdu::RequestListL|Reader status appended"));
                    }   //else if
                else
                    {
                    _WIMTRACE2(_L("WIM|WIMServer|CWimApdu::RequestListL|Leave with error %d"), error);
                    User::Leave( error );
                    }
                }   //else
            }   //else
        }   //for

    //Add Listeners to readers 
    for ( TInt y = 0; y < iReaderNames->Count(); y++ )
        {
        CWimScardListener* wimScardListener;                       
        TRAP( error, wimScardListener = CWimScardListener::NewL( iServer,
            ( TUint8 )y, iReaderNames->At( y ) ) );
        iWimScardListenerArray->AppendL( wimScardListener );
        }

    iResponseStatus = KWimApduOk;

    delete query;   
    readerNames->Reset();   
    delete readerNames;
    groupNames->Reset();
    delete groupNames;

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CWimApdu::Open
// Open connection to Scard
// -----------------------------------------------------------------------------
//
TUint8 CWimApdu::Open( TUint8 aUiReaderId )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::Open | Begin"));
    
    if ( !iServer ) //No Scard
        {
        TRAPD( error, iServer = RScard::NewL() );
        
        if ( error != KErrNone )
            {
            iResponseStatus = KWimApduTransmiossionError;
            return KWimStatusIOError;
            }
        }

    //If aUiReaderId is valid and connection to reader is not opened...
    if ( ( aUiReaderId < iReaderComm->Count() ) &&
        ( (*iReaderComm)[aUiReaderId] == 0 ) )
        {
        _WIMTRACE(_L("WIM | WIMServer | CWimApdu::Open | Reader not opened"));
        //...open connection, try to find the reader with name
        //specified in iReaderNames.
        TScardConnectionRequirement rest;
        TScardReaderName name;
        iStatus = KRequestPending;

        TRAPD( error, 
            rest.SetExplicitL( (*iReaderNames)[aUiReaderId] );
            (*iReaderComm)[aUiReaderId] =
                CScardComm::NewL( iServer, rest, name, iStatus ) );
        
        SetActiveAndWait();

        _WIMTRACE3(_L("WIM | WIMServer | CWimApdu::Open | Error=%d, iStatus=%d"),
        	error, iStatus.Int());

        if ( error == KErrNone && iStatus.Int() == KErrNone )
            {//Add listener to opened reader and append it to array.
            CWimScardListener* wimScardListener;
            TRAP( error,
                wimScardListener = CWimScardListener::NewL( iServer,
                                                            aUiReaderId,
                                                            name );
                iWimScardListenerArray->AppendL( wimScardListener );
                );
            }

        if ( error == KErrNoMemory || iStatus.Int() == KErrNoMemory )
            {
            iResponseStatus = KWimApduNoMemory;
            return ( TUint8 )KWimCardDriverInitError;
            }
        else if ( error || name != (*iReaderNames)[aUiReaderId]
            || iStatus.Int() != KErrNone )
            {
            iResponseStatus = KWimApduTransmiossionError;
            return KWimStatusIOError;
            }
        else
            {
            iResponseStatus = KWimApduOk;
            return KWimStatusOK; //WIMSTA_OK;
            }
        }
    else
        {
        iResponseStatus = KWimApduOk;
        return KWimStatusOK; //WIMSTA_OK;
        }
    }

// -----------------------------------------------------------------------------
// CWimApdu::Close
// Close connection to card
// -----------------------------------------------------------------------------
//
TUint8 CWimApdu::Close( TUint8 aUiReaderId )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::Close | Begin"));

    if ( ( aUiReaderId < iReaderComm->Count() ) &&
        ( (*iReaderComm)[aUiReaderId] != 0 ) )
        {
        delete (*iReaderComm)[aUiReaderId];
        (*iReaderComm)[aUiReaderId] = 0;
        }
    
    if ( iWimScardListenerArray->Count() > aUiReaderId )
        {
        delete iWimScardListenerArray->At( aUiReaderId );
        iWimScardListenerArray->Delete( aUiReaderId );
        }
    iResponseStatus = KWimApduOk;
    return KWimStatusOK; //WIMSTA_OK;
    }

// -----------------------------------------------------------------------------
// CWimApdu::SendAPDU
// Send APDU to card
// -----------------------------------------------------------------------------
//
TUint8 CWimApdu::SendAPDU(
    TUint8  aUiReaderId,
    TUint8* aApdu,
    TUint16 aUiApduLength )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::SendAPDU | Begin"));
    //clean SW
    iResponseSW = 0;
    TPtr8 responsePtr = iResponseBuffer->Des();
    //clean buffer
    responsePtr.Zero();
    if ( ( aUiReaderId < iReaderComm->Count() ) &&
        ( (*iReaderComm)[ aUiReaderId ] != 0 ) )
        {
        iUiReaderId = aUiReaderId;
        
        iStatus = KRequestPending;
        //If apdu to be sent was ManageChannel, 
        //use CScardComms ManageChannel function.
        if ( ( aUiApduLength >= 4 ) && ( aApdu[1] == KManageChannelIns ) )
            {
            switch ( aApdu[2] )
                {
                case KManageChannelParamOpen:
                    {
                    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::SendAPDU | OPEN CHANNEL"));
                    //Last parameter is timeout. No timeout used. 
                    (*iReaderComm)[aUiReaderId]->ManageChannel(
                        EOpenAnyChannel, aApdu[3], responsePtr, iStatus, 0 );
                    break;
                    }
                case KManageChannelParamClose:
                    {
                    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::SendAPDU | CLOSE CHANNEL"));
                    // Check whether state of the SIM/SWIM is OK
                    if ( CWimUtilityFuncs::SimState() != KErrNone ) 
                        {
                        _WIMTRACE(_L("WIM | WIMServer | CWimApdu::SendAPDU | SIM not ready"));
                        // SIM/SWIM not OK, just return KWimStatusOK to WimLib
                        // so WimLib can continue properly
                        return KWimStatusOK;
                        }
                    else // Card OK, close the opened channel
                        {
                        // Close channel with 5 second timeout
                        (*iReaderComm)[aUiReaderId]->ManageChannel( 
                                                ECloseChannel,
                                                aApdu[3],
                                                responsePtr,
                                                iStatus,
                                                KDefaulCloseChannelTimeout );
                        }
                    break;
                    }
                default:    // APDU is not valid.
                    {
                    iResponseStatus = KWimApduFormatError;
                    return KWimStatusOK; //WIMSTA_OK;
                    //break; //unreachable
                    }
                }
            }
        else
            {
            TPtrC8 commandAPDU( aApdu, aUiApduLength );

            //Transmit APDU to card. 
            (*iReaderComm)[aUiReaderId]->TransmitToCard( commandAPDU, 
                responsePtr,  iStatus, 0, (TUint8)( aApdu[0] & 0x0f ) );
            
            }
        SetActiveAndWait(); // Wait APDU response
        
        if ( iStatus != KErrNone ) 
            {
            iResponseStatus = KWimApduTransmiossionError;
            responsePtr.SetLength( 0 );
            return KWimStatusIOError;
            } 
        
        if ( responsePtr.Size() >= 2 ) //At least SW-bytes are needed (2 of'em).
            {
            //Response has to be retrieved.
            if ( responsePtr[responsePtr.Size() - 2] == 0x61 ) 
                {
                iStatus = KRequestPending;
                iResponseAPDU[0] = aApdu[0];
                iResponseAPDU[4] = responsePtr[responsePtr.Size() - 1];
                TPtrC8 commandAPDU( iResponseAPDU, 5 );

                //Transmit APDU to card. Timeout not used.
                (*iReaderComm)[aUiReaderId]->TransmitToCard( commandAPDU,
                    responsePtr, iStatus, 0, (TUint8)( aApdu[0] & 0x0f ) );
                
                SetActiveAndWait(); // Wait APDU response

                if ( iStatus != KErrNone ) 
                    {
                    iResponseStatus = KWimApduTransmiossionError;
                    responsePtr.SetLength( 0 );
                    return KWimStatusIOError;
                    } 
                
                //At least SW-bytes are needed (2 of them).
                else if ( responsePtr.Size() < 2 ) 
                    {
                    iResponseStatus = KWimApduFormatError;   
                    responsePtr.SetLength( 0 );
                    return KWimStatusOK;
                    }
                }
            iResponseStatus = KWimApduOk;
            iResponseSW = 
                ( TUint16 )( ( responsePtr[responsePtr.Size() - 2] << 8 )
                | ( responsePtr[responsePtr.Size() - 1] ) );

            responsePtr.SetLength( responsePtr.Size() - 2 );//All but SW-bytes.
            return KWimStatusOK;
            }
        else
            {
            iResponseStatus = KWimApduFormatError;
            responsePtr.SetLength( 0 );
            return KWimStatusOK; //WIMSTA_OK;
            }
        }
    else
        {
        //Not valid card.
        iResponseStatus = KWimApduReaderNotValid;
        responsePtr.SetLength( 0 );
        return KWimStatusOK; //WIMSTA_OK;
        }           
    }

// -----------------------------------------------------------------------------
// CWimApdu::ResponseApdu
// Return response APDU
// -----------------------------------------------------------------------------
//
TPtrC8 CWimApdu::ResponseApdu() const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::ResponseApdu | Begin"));
    return iResponseBuffer->Des();
    }

// -----------------------------------------------------------------------------
// CWimApdu::ResponseSW
// Return Response SW
// -----------------------------------------------------------------------------
//
TUint16 CWimApdu::ResponseSW() const
    {
    _WIMTRACE2(_L("WIM | WIMServer | CWimApdu::ResponseSW | iResponse = %04x"),
    	iResponseSW );
    return iResponseSW;
    }

// -----------------------------------------------------------------------------
// CWimApdu::StatusList
// Return reader statusses
// -----------------------------------------------------------------------------
//
const TBuf8<KMaxReaderCount>& CWimApdu::StatusList() const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::StatusList | Begin"));
    return iReaderStatusses;
    }

// -----------------------------------------------------------------------------
// CWimApdu::StatusListLength
// Return status list length
// -----------------------------------------------------------------------------
//
TUint8 CWimApdu::StatusListLength() const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::StatusListLength | Begin"));
    return iReaderStatusLength;
    }

// -----------------------------------------------------------------------------
// CWimApdu::RunL
// Stop ActiveSchedulerWait after APDU response
// -----------------------------------------------------------------------------
//
void CWimApdu::RunL()
    {
    _WIMTRACE2(_L("WIM | WIMServer | CWimApdu::RunL, status: %d"), iStatus.Int());
    iWait.AsyncStop();
    }

// -----------------------------------------------------------------------------
// CWimApdu::DoCancel
// Cancel asyncronous request
// -----------------------------------------------------------------------------
//
void CWimApdu::DoCancel()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::DoCancel | Begin"));
    (*iReaderComm)[iUiReaderId]->CancelTransmit();
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::DoCancel | End"));
    }

// -----------------------------------------------------------------------------
// CWimApdu::SetActiveAndWait
// Wait until asynchronous call is completed
// -----------------------------------------------------------------------------
//
void CWimApdu::SetActiveAndWait()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::SetActiveAndWait | Begin"));
    SetActive();
    iWait.Start();
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::SetActiveAndWait | End"));
    }

// -----------------------------------------------------------------------------
// CWimApdu::CancelApduSending
// -----------------------------------------------------------------------------
//    
void CWimApdu::CancelApduSending()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimApdu::CancelApduSending | Begin"));
	if( IsActive() )
	    {
		Cancel();
	    }
	_WIMTRACE(_L("WIM | WIMServer | CWimApdu::CancelApduSending | End"));    
    }

// End of File
