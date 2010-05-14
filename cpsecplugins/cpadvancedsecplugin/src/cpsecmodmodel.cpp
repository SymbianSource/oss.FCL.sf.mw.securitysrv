/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of the CSecModUIModel class
*
*/

#include <unifiedkeystore.h>
#include <mctauthobject.h>
#include <mctkeystore.h>
#include <securityerr.h>

#include <qvector.h>
#include <qpair.h>
#include <qlabel.h>
#include <qerrormessage.h>
#include <qstring.h>

#include <hbmessagebox.h>

#include "cpsecmodmodel.h"
#include "cpsecmodsyncwrapper.h"

const TInt KWIMStoreUid ( 0x101F79D9 );
_LIT(KSecModUIPanic, "Security Module UI panic");

// ============================= LOCAL FUNCTIONS ===============================
GLDEF_C void Panic(TInt aPanic)
  {
  User::Panic(KSecModUIPanic, aPanic);
  }

// -----------------------------------------------------------------------------
// CSecModUIModel::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSecModUIModel* CSecModUIModel::NewL()
    {
    CSecModUIModel* self = new( ELeave ) CSecModUIModel();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::CSecModUIModel
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CSecModUIModel::CSecModUIModel()
    {}

// -----------------------------------------------------------------------------
// CSecModUIModel::~CSecModUIModel
// C++ destructor
// -----------------------------------------------------------------------------
//

CSecModUIModel::~CSecModUIModel()
    {
    delete iWrapper;
    ResetAndDestroyCTObjects();
    }
// -----------------------------------------------------------------------------
// CSecModUIModel::ResetAndDestroyCTObjects(): Cleans up all objects referenced
// through the crypto token framework.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ResetAndDestroyCTObjects()
    {
    ResetAndDestroyAOs();
    iAOKeyStores.Reset();
    delete iUnifiedKeyStore;
    iUnifiedKeyStore = NULL;
    iKeyStore = NULL;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ResetAndDestroyAOs(): Cleans up the list of auth objects.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ResetAndDestroyAOs()
    {
    if (iAOList)
        {
        iAOList->Release();
        iAOList = NULL;
        }
    if (iAOArray.Count() > 0 &&
        iAOArray[0]->Token().TokenType().Type().iUid == KTokenTypeFileKeystore)
        {
        iAOArray.Reset();
        }
    else
        {
        iAOArray.Close();
        }
    }
    
// -----------------------------------------------------------------------------
// CSecModUIModel::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ConstructL()
    {
    InitializeKeyStoreL();
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::InitializeKeyStoreL()
// -----------------------------------------------------------------------------
//
void CSecModUIModel::InitializeKeyStoreL()
    {
    ResetAndDestroyCTObjects();
    
    // Ensure that file server session is succesfully created
    User::LeaveIfError( iRfs.Connect() );
        
    iUnifiedKeyStore = CUnifiedKeyStore::NewL(iRfs);

    if (iWrapper == NULL)
        {
        iWrapper = CSecModUISyncWrapper::NewL();
        }

    TInt err = iWrapper->Initialize(*iUnifiedKeyStore);

    ShowErrorNote(err);
    if (KErrNone != err && KErrHardwareNotAvailable != err)
        {
        User::Leave(err);
        }
    TInt keyStoreCount = iUnifiedKeyStore->KeyStoreCount();
    if (0 >= keyStoreCount)
        {
        return;
        }

    RMPointerArray<CCTKeyInfo> keyInfos;
    CleanupClosePushL(keyInfos);
    TCTKeyAttributeFilter filter;
    TInt keyInfoCount = 0;
    filter.iPolicyFilter = TCTKeyAttributeFilter::EAllKeys;

    for (TInt i = 0; i < keyStoreCount; i++)
        {
		err = iWrapper->ListKeys(iUnifiedKeyStore->KeyStore(i), keyInfos, filter);
		User::LeaveIfError(err);
        //If list ok, append the AO, otherwise go next
		keyInfoCount = keyInfos.Count();
		for (TInt j = 0; j < keyInfoCount; j++)
			{
			// Check that keystore has at least one AO.
			if (NULL != keyInfos[j]->Protector())
				{
				// If keystore has AO, add it to the array.
				User::LeaveIfError(
					iAOKeyStores.Append(&(iUnifiedKeyStore->KeyStore(i))));
				break;
				}
			}     
        }
    CleanupStack::PopAndDestroy(&keyInfos);
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::LoadTokenLabelsL()
// Retrieves the description and location of all tokens.
// -----------------------------------------------------------------------------
//
QMap<QString,QString> CSecModUIModel::LoadTokenLabelsL()
    {
	
    TInt keyStoreCount = iAOKeyStores.Count();
    QMap<QString, QString> labelList;
    
    if (0 == keyStoreCount)
        {
        return labelList;
        }
    
    for (TInt i = 0; i < keyStoreCount; i++)
        {
		QString keystoreLabel = 0;
        if (iAOKeyStores[i]->Token().TokenType().Type().iUid == KTokenTypeFileKeystore )
            {
			keystoreLabel = "Phone keystore";
            }
        else if(iAOKeyStores[i]->Token().TokenType().Type().iUid == KWIMStoreUid )
            {                
			const TDesC& label = iAOKeyStores[i]->Token().Label();
			keystoreLabel = QString( (QChar*) label.Ptr(), label.Length());
            }
        else
        	{
			continue;
        	}
         QString location = Location(iAOKeyStores[i]->Token().TokenType().Type());
        labelList.insert(keystoreLabel,location);
        }
    return labelList;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::Location() 
// Identifies the location of the keystore based on the Uid.
// -----------------------------------------------------------------------------
//
QString CSecModUIModel::Location( TUid aUid ) const
    {
    QString location = 0;

    switch ( aUid.iUid )
        {
        case KTokenTypeFileKeystore:
            {
            location = "\nPhone memory";
            break;
            }
        case KWIMStoreUid:
            {
            location = "\nSmart card";
            break;
            }
        default:
            {
            break;
            }
        }
    return location;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ShowErrorNote(TInt aError) const
// Displays appropriate error message based on the error code.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ShowErrorNote(TInt aError) const
    {
	QString sError;

    switch (aError)
        {
        case KErrCancel:
        case KErrLocked:
        case KErrNone:
            {
            break;
            }
        case KErrInUse:
            {
            sError = "Key is currently in use. Unable to delete. Close applications and try again.";
            break;
            }
        case KErrBadPassphrase:
            {
            sError = "Code incorrect";
            break;
            }
        case KErrHardwareNotAvailable: // flow thru
        default:
            {
            sError = "Security module internal error";
            break;
            }
        }

    if ( sError.length() != 0 )
		{
		HbMessageBox::information(sError);
		}
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::OpenTokenL(TInt aTokenIndex)
// Retrieves the appropriate token based on the index.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::OpenTokenL(TInt aTokenIndex)
    {
    ResetAndDestroyAOs();
    iKeyStore = iAOKeyStores[aTokenIndex];
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::CheckCodeViewStringsL()
// Initialize the auth objects and return the total count.
// -----------------------------------------------------------------------------
//
TInt CSecModUIModel::CheckCodeViewStringsL()
    {
    InitAuthObjectsL();
    ListAuthObjectsL();
    return iAOArray.Count();
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::InitAuthObjectsL()
// Retrieve the authentication object associated with the appropriate key store 
// implementation.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::InitAuthObjectsL()
    {
    TInt err = KErrNone;
    // Symbian's file key store does not support
    // MCTAuthenticationObjectList interface, so we need to use
    // other way to get AO.
    if (iKeyStore->Token().TokenType().Type().iUid == KTokenTypeFileKeystore)
        {
        if (iAOArray.Count()==0)
            {
            RMPointerArray<CCTKeyInfo> keyInfos;
            CleanupClosePushL(keyInfos);
            TCTKeyAttributeFilter filter;
            TInt keyInfoCount = 0;
            filter.iPolicyFilter = TCTKeyAttributeFilter::EAllKeys;
            err = iWrapper->ListKeys(*iKeyStore, keyInfos, filter);
            ShowErrorNote(err);
            User::LeaveIfError(err);
            keyInfoCount = keyInfos.Count();
            for (TInt j = 0; j < keyInfoCount; j++)
                {
                // Check that keystore has at least one AO.
                if (NULL != keyInfos[j]->Protector())
                    {
                    // If keystore has AO, add it to the array.
                    User::LeaveIfError(iAOArray.Append(keyInfos[j]->Protector()));
                    break;
                    }
                }
            CleanupStack::PopAndDestroy(&keyInfos);  //keyInfos
            }
        }
    else
        {
        if (NULL == iAOList)
            {
            MCTTokenInterface* tokenInterface = NULL;
            err = iWrapper->GetAuthObjectInterface(
                iKeyStore->Token(), tokenInterface);
            if ( KErrNone != err || NULL == tokenInterface )
                {
                ShowErrorNote(err);
                User::Leave(err);
                }
            iAOList = (MCTAuthenticationObjectList*)tokenInterface;
            }
        }
    }
// -----------------------------------------------------------------------------
// CSecModUIModel::ListAuthObjectsL()
// list the auth objects.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ListAuthObjectsL()
    {
    if (0 >= iAOArray.Count())
        {
        __ASSERT_ALWAYS(iAOList, Panic(EPanicNullPointer));
        TInt err = iWrapper->ListAuthObjects(*iAOList, iAOArray);
        if (err)
            {
            ShowErrorNote(err);
            User::Leave(err);
            }
        }
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::TokenLabelForTitle() const
// Retrieves description of the token based on UID of the current active keystore.
// -----------------------------------------------------------------------------
//
QString CSecModUIModel::TokenLabelForTitle() const
    {
	QString qLabel;
    if (iKeyStore->Token().TokenType().Type().iUid 
        == KTokenTypeFileKeystore)
        {
        qLabel = "Phone keystore";   
        }
    else
        {            
        const TDesC& label = iKeyStore->Token().Label();
        qLabel = QString((QChar*)label.Ptr(),label.Length());
        }
    return qLabel;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::AuthStatus(TInt aIndex) const
// Returns status of the selected auth object. 
// -----------------------------------------------------------------------------
//
TUint32 CSecModUIModel::AuthStatus(TInt aIndex) const
		{
		return iAOArray[aIndex]->Status();
		}

// -----------------------------------------------------------------------------
// CSecModUIModel::PINStatus( TInt aIndex, TBool aShowBlockedNote ) const
// Returns status of the auth object based on its persistent properties set
// by security module implementation.
// -----------------------------------------------------------------------------
//
QString CSecModUIModel::PINStatus(  TInt aIndex, TBool aShowBlockedNote ) const
    {
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    QString pinStatus;
    TUint32 status = iAOArray[aIndex]->Status();
    
    if (status & EAuthObjectBlocked)
        {
        if (aShowBlockedNote)
            {
			QString information = 0;
            if (status & EUnblockDisabled)
                {
				information = " totalblocked, contact your module vendor.";
                }
            else
                {
				information = " is blocked. PUK code is needed to unblock the code";
                }
            information.prepend(QString( (QChar*)(iAOArray[aIndex]->Label()).Ptr(),iAOArray[aIndex]->Label().Length() ) );
            HbMessageBox::information(information);        
            }            
        pinStatus = "Blocked";
        }
    else if ( PinOpen(aIndex) )
        {
		pinStatus = "Open";
        }
    else
        {
		pinStatus = "Closed";
        }
    return pinStatus;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::PinOpen(TInt aIndex) const
// Checks whether the Pin is still available.
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinOpen(TInt aIndex) const
    {
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TInt timeRemaining = 0;
    TInt err = iWrapper->TimeRemaining(*(iAOArray[aIndex]), timeRemaining);
    TBool ret = ETrue;
    if ( 0 < timeRemaining )
        {
        ret = ETrue;
        }
    else if( timeRemaining == -1 )
        {
        ret = ETrue;
        }
    else
        {
        ret = EFalse;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ChangeOrUnblockPinL(TInt aIndex)
// Based on the property of auth object the pin can be changed or unblocked.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ChangeOrUnblockPinL(TInt aIndex)
    {
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TUint32 status = iAOArray[aIndex]->Status();

    if ((status & EAuthObjectBlocked) && (status & EUnblockDisabled))
        {
        return; // We cannot do anything. PIN is total bolcked.
        }
    else if (status & EAuthObjectBlocked)
        {
        // Let's unblock the PIN
        UnblockPinL(aIndex);
        }
    else if (status & EEnabled)
        {
        // PIN is enabled. Let's change the PIN.
        ChangePinL(aIndex);
        }
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::UnblockPinL(TInt aIndex)
// Unblocks PIN.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::UnblockPinL(TInt aIndex)
    {
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TInt err = iWrapper->UnblockAuthObject(*(iAOArray[aIndex]));
    if (KErrNone == err)
        {
        HbMessageBox::information("Code unblocked");
        }
    else
        {
        ShowErrorNote(err);
        }
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ChangePinL(TInt aIndex)
// Changes the value of the PIN.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ChangePinL(TInt aIndex)
    {
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TInt err = KErrNone;
  
    err = iWrapper->ChangeReferenceData(*(iAOArray[aIndex]));
    if (err)
        {
        ShowErrorNote(err);
        }
    else
        {
		HbMessageBox::information("Code changed");
        }
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ChangeCodeRequestL(TInt aIndex)
// Toggles the value of auth object if possible.
// -----------------------------------------------------------------------------
//
TInt CSecModUIModel::ChangeCodeRequest(TInt aIndex)
    {
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TUint32 status = iAOArray[aIndex]->Status();
    TInt err = KErrNone;
    if (status & EDisableAllowed &&  !(status & EAuthObjectBlocked))
        {
        if ( status & EEnabled )
            {
            err = iWrapper->DisableAuthObject(*(iAOArray[aIndex]));
            }
        else
            {
            err = iWrapper->EnableAuthObject(*(iAOArray[aIndex]));
            }
        ShowErrorNote(err);
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::CloseAuthObjL(TInt aIndex)
// If the auth object is open then closes it.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::CloseAuthObjL(TInt aIndex)
    {
    if (PinOpen(aIndex))
        {
        __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
        TInt err = iWrapper->CloseAuthObject(*(iAOArray[aIndex]));
        if (err)
            {
            ShowErrorNote(err);
            User::Leave(err);
            }
        }
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::DeleteKeysL(TInt aTokenIndex)
// Deletes key from the appropriate keystore.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::DeleteKeysL(TInt aTokenIndex)
    {
    __ASSERT_ALWAYS(aTokenIndex < iAOKeyStores.Count(), Panic(EPanicIndexOutOfRange));
    RMPointerArray<CCTKeyInfo> keyInfos;
	CleanupClosePushL(keyInfos);
	TCTKeyAttributeFilter filter;
	filter.iPolicyFilter = TCTKeyAttributeFilter::EAllKeys;
	TInt err = iWrapper->ListKeys(*iAOKeyStores[aTokenIndex], keyInfos, filter);
	ShowErrorNote(err);
	User::LeaveIfError(err);
	for (TInt i = 0; i < keyInfos.Count(); ++i)
		{
		err = iWrapper->DeleteKey(*iUnifiedKeyStore, keyInfos[i]->Handle());
		ShowErrorNote(err);

		if (KErrHardwareNotAvailable == err )
			{
			break; // Break the loop, if keystore not available
			}
		}
	CleanupStack::PopAndDestroy(&keyInfos);  //keyInfos
	ResetAndDestroyCTObjects();
	InitializeKeyStoreL();
	}

// -----------------------------------------------------------------------------
// CSecModUIModel::SecModDetailsL(TInt aTokenIndex)
// Retrieve security module details and cleansup all currently held auth objects.
// -----------------------------------------------------------------------------
//
QVector< QPair<QString,QString> > CSecModUIModel::SecModDetailsL(TInt aTokenIndex)
    {
    iKeyStore = iAOKeyStores[aTokenIndex];
    InitAuthObjectsL();
    ListAuthObjectsL();
    QVector< QPair<QString,QString> > securityDetails = GetSecModDetailsL();
    iKeyStore = NULL; // not owned
    if (iAOList)
        {
        iAOList->Release();
        iAOList = NULL;
        }
    iAOArray.Reset();
    return securityDetails;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::GetSecModDetailsL()
// Retrieve security module details.
// -----------------------------------------------------------------------------
//
QVector< QPair<QString,QString> > CSecModUIModel::GetSecModDetailsL()
    {
    MCTToken& token = iKeyStore->Token();
    QVector< QPair<QString,QString> > securityModDetails;
    
    if (token.TokenType().Type().iUid == KTokenTypeFileKeystore)
        { 
		QPair<QString,QString> name("Name:","Phone keystore");
        securityModDetails.append(name);
        }
    else 
        {            
		QPair<QString,QString> name("Name:","(No label)");
        securityModDetails.append(name);
        }

    AppendItem(securityModDetails, "Version:", token.Information( MCTToken::EVersion ), "(Not defined)" );
    AppendLocation(securityModDetails, token.TokenType().Type(), QString("Security module location:"));
    
    if (token.TokenType().Type().iUid == KTokenTypeFileKeystore)
        { 
		QPair<QString,QString> accessCode("Access code:","Phone keystore code");
        securityModDetails.append(accessCode);
        }
    else 
        {            
        AppendItem(securityModDetails, "Access code:", iAOArray[0]->Label(), "Access code:" );
        }

    AppendPinNRs(securityModDetails);

    AppendItem(	securityModDetails, "Serial number:",
				token.Information( MCTToken::ESerialNo),
				"(Not defined)");

    AppendItem(	securityModDetails, "Manufacturer:",
				token.Information( MCTToken::EManufacturer),
				"(Not defined)");
    return securityModDetails;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::AppendLocation()
// Appends the location of the keystore.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::AppendLocation( QVector< QPair<QString,QString> >& aSecModDetails, 
									 TUid aLocUid, 
									 QString aCertDisplayDetails )
    {
    QString locationDetails = "(Not defined)";

    switch ( aLocUid.iUid )
        {
    	case KTokenTypeFileKeystore:
        	locationDetails = "Phone memory";
            break;

        case KWIMStoreUid:
        	locationDetails = "Smart card";
            break;

        default:
            break;
        }
    aSecModDetails.append(QPair<QString,QString>(aCertDisplayDetails, locationDetails));
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::AppendItem()
// Appends the value of a label if present otherwise appends empty descriotion.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::AppendItem( 	QVector< QPair<QString,QString> >& aSecModDetails, 
									QString aLabel, 
									const TDesC& aValue, 
									QString aNullDescription)
	{
	if( aValue.Length() == 0 )
		{
		aSecModDetails.append(QPair<QString,QString>(aLabel, aNullDescription));
		}
	else
		{
		QString value = QString((QChar*) aValue.Ptr(),aValue.Length());
		aSecModDetails.append(QPair<QString,QString>(aLabel, value));
		}
	}

// -------------------------------------------------------------------------------
// CSecModUIModel::AppendPinNRs(QVector< QPair<QString,QString> >& aSecModDetails)
// Generate details of signing security module.
// -------------------------------------------------------------------------------
//
void CSecModUIModel::AppendPinNRs(QVector< QPair<QString,QString> >& aSecModDetails)
    {
    TInt pinCount = iAOArray.Count();
    TInt j = 0;

	for (TInt i=1; i<pinCount; i++)
		{
		QString noLabelDetails = "Signing code ";
		const TDesC& label = iAOArray[i]->Label();
		if(label.Length() == 0)
			{
			noLabelDetails.append(++j);
			}
		QString signingCode = "Signing code:";
		AppendItem(aSecModDetails, signingCode , iAOArray[i]->Label(), noLabelDetails);
		}
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::AuthDetails() const
// Return the list of pair of description and status of each auth object.
// -----------------------------------------------------------------------------
//
QVector<QPair<QString, TUint32> > CSecModUIModel::AuthDetails()
	{
	QVector<QPair<QString, TUint32> > authDetails;
	
	// Here is assumed that the first AO is always PIN-G
	for (TInt i = 1; i < iAOArray.Count(); i++)
		{
		const TDesC& label = iAOArray[i]->Label();
		TUint32 status = iAOArray[i]->Status();
		QString sLabel = QString((QChar*) label.Ptr(), label.Length());
		QPair<QString, TUint32> pair(sLabel, status);
		authDetails.append(pair);
		}
	return authDetails;
	}
	
// -----------------------------------------------------------------------------
// CSecModUIModel::ChangePinNrL(TInt aIndex)
// Change the pin value for the singing module.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ChangePinNrL(TInt aIndex)
    {
    ChangePinL(aIndex+1);
    }


// -----------------------------------------------------------------------------
// CSecModUIModel::UnblockPinNrL(TInt aIndex)
// Unblock the value for the signing code module.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::UnblockPinNrL(TInt aIndex)
    {
    UnblockPinL(aIndex+1);
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::KeyStoreUID()
// Retrieves the UID of the current active keystore.
// -----------------------------------------------------------------------------
//
TInt CSecModUIModel::KeyStoreUID()
	{
	return iKeyStore->Token().TokenType().Type().iUid;
	}

// -----------------------------------------------------------------------------
// CSecModUIModel::AuthObj(TInt aIndex)
// Retrieves the auth object at the speficied index.
// -----------------------------------------------------------------------------
//
const MCTAuthenticationObject& CSecModUIModel::AuthObj(TInt aIndex)
	{
	return *iAOArray[aIndex];
	}

// -----------------------------------------------------------------------------
// CSecModUIModel::PinChangeable(TInt aIndex) const
// Returns the status accordingly if the pin is changeable or not.
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinChangeable(TInt aIndex) const
    {
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TUint32 status = iAOArray[aIndex]->Status();
    TBool ret = ETrue;
    if ( (status & EAuthObjectBlocked) || (status & EChangeDisabled) )
        {
        ret = EFalse;
        }
    else if ( status & EEnabled )
        {
        ret = ETrue;
        }
    else // PIN is disabled;
        {
        ret = EFalse;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::PinUnblockable(TInt aIndex) const
// Returns the status accordingly if the pin is unblockable or not.
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinUnblockable(TInt aIndex) const
    {
    TUint32 status = iAOArray[aIndex]->Status();
    TBool ret = ETrue;
    if ( status & EUnblockDisabled )
        {
        ret = EFalse;
        }
    else if ( status & EAuthObjectBlocked )
        {
        ret = ETrue;
        }
    else // PIN is not blocked;
        {
        ret = EFalse;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::PinRequestChangeable(TInt aIndex) const
// Returns the status accordingly if the pin is changeable or not.
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinRequestChangeable(TInt aIndex) const
    {
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TUint32 status = iAOArray[aIndex]->Status();
    TBool ret = EFalse;
    if ( status & EAuthObjectBlocked )
        {
        ret = EFalse;
        }
    else if ( status & EDisableAllowed )
        {
        ret = ETrue;
        }
    else // Disable not allowed;
        {
        ret = EFalse;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::IsTokenDeletable(TInt aTokenIndex)
// Returns the status accordingly if the token is deletable or not.
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::IsTokenDeletable(TInt aTokenIndex)
    {
    __ASSERT_ALWAYS(aTokenIndex < iAOKeyStores.Count(), Panic(EPanicIndexOutOfRange));
    TBool ret = EFalse;
    if (iAOKeyStores[aTokenIndex]->Token().TokenType().Type().iUid
        == KTokenTypeFileKeystore)
        {
        ret = ETrue;
        }
    else
        {
        ret = EFalse;
        }
    return ret;
    }

//  End of File


