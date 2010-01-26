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



// INCLUDE FILES
#include    "SecModUI.h"
#include    "SecModUIModel.h"
#include    "SecModUIViewAccess.h"
#include    "SecModUIViewCode.h"
#include    "SecModUIViewMain.h"
#include    "SecModUIViewSignature.h"
#include    "SecModUISyncWrapper.h"
#include    "SecModUILogger.h"
#include    <SecModUI.rsg>
#include    <certmanui.rsg>
#include    <CTSecDlgs.rsg>
#include    <aknViewAppUi.h>
#include    <AknUtils.h>
#include    <data_caging_path_literals.hrh>
#include    <bautils.h>
#include    <unifiedkeystore.h>
#include    <aknlists.h>
#include    <mctauthobject.h>
#include    <aknnotewrappers.h>
#include    <StringLoader.h>
#include    <mctkeystore.h>
#include    <akntitle.h>
#include    <aknmessagequerydialog.h>
#include    <eikenv.h>
#include    <securityerr.h>
#include    <AknGlobalNote.h>

// CONSTANTS
_LIT(KResourceFile, "z:SecModUI.rsc");
_LIT(KResourceFile2, "z:CertManUI.rsc");
_LIT(KResourceFile3, "z:CTsecdlgs.rsc");
_LIT(KTab, " \t");
_LIT(KDoubleTab, "\t\t");
_LIT(KFourAsterisks,"****");
_LIT(KEnter, "\n");
_LIT(KDoubleEnter, "\n\n");

_LIT(KSecModUIPanic, "Security Module UI panic");

const TInt KMaxLengthTextDetailsBody = 750;
const TInt KMaxLengthItemValue = 100;
const TInt KWIMStoreUid ( 0x101F79D9 );
const TInt KItemLength (200);

// ============================= LOCAL FUNCTIONS ===============================
GLDEF_C void Panic(TInt aPanic)
  {
  User::Panic(KSecModUIPanic, aPanic);
  }

// ============================ MEMBER FUNCTIONS ===============================
#ifndef RD_GS_RENOVATION
EXPORT_C MSecModUI* MSecModUI::CreateL()
    {
    LOG_CREATE;
    LOG_ENTERFN("MSecModUI::CreateL()");
    LOG_LEAVEFN("MSecModUI::CreateL()");
    return CSecModUIModel::NewL();
    }
#endif

// -----------------------------------------------------------------------------
// CSecModUIModel::CSecModUIModel
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSecModUIModel::CSecModUIModel()
    {
  LOG_WRITE( "CSecModUIModel::CSecModUIModel" );
    }

// Destructor
CSecModUIModel::~CSecModUIModel()
    {
    LOG_ENTERFN("CSecModUIModel::~CSecModUIModel");    
    delete iWrapper;
    if ( iResourceFileOffset )
        {
    iEikEnv->DeleteResourceFile( iResourceFileOffset );
        }
    if ( iResourceFileOffset2 )
        {
    iEikEnv->DeleteResourceFile( iResourceFileOffset2 );
        }
    if ( iResourceFileOffset3 )
        {
    iEikEnv->DeleteResourceFile( iResourceFileOffset3 );
        }    
    ResetAndDestroyCTObjects();
    LOG_LEAVEFN("CSecModUIModel::~CSecModUIModel");
    LOG_DELETE;
    }
// -----------------------------------------------------------------------------
// CSecModUIModel::ResetAndDestroyCTObjects()
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ResetAndDestroyCTObjects()
    {
    LOG_ENTERFN("CSecModUIModel::ResetAndDestroyCTObjects()");

    ResetAndDestroyAOs();
    
    iAOKeyStores.Reset();

    delete iUnifiedKeyStore;
    iUnifiedKeyStore = NULL;
    iKeyStore = NULL;
    LOG_LEAVEFN("CSecModUIModel::ResetAndDestroyCTObjects()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ResetAndDestroyAOs()
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ResetAndDestroyAOs()
    {
    LOG_ENTERFN("CSecModUIModel::ResetAndDestroyAOs()");
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
    LOG_LEAVEFN("CSecModUIModel::ResetAndDestroyAOs()");
    }
    
// -----------------------------------------------------------------------------
// CSecModUIModel::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ConstructL()
    {
    LOG_ENTERFN("CSecModUIModel::ConstructL()");
    iEikEnv = CEikonEnv::Static();
    AddResourceFileL();

#ifndef RD_GS_RENOVATION

    CSecModUIViewMain* viewMain = CSecModUIViewMain::NewLC(*this);
    ((CAknViewAppUi*)iEikEnv->EikAppUi())->AddViewL(viewMain);
    CleanupStack::Pop(viewMain);

    CSecModUIViewCode* viewCode = CSecModUIViewCode::NewLC(*this);
    ((CAknViewAppUi*)iEikEnv->EikAppUi())->AddViewL(viewCode);
    CleanupStack::Pop(viewCode);

    CSecModUIViewAccess* viewAccess = CSecModUIViewAccess::NewLC(*this);
    ((CAknViewAppUi*)iEikEnv->EikAppUi())->AddViewL(viewAccess);
    CleanupStack::Pop(viewAccess);

    CSecModUIViewSignature* viewSignature = 
        CSecModUIViewSignature::NewLC(*this);
    ((CAknViewAppUi*)iEikEnv->EikAppUi())->AddViewL(viewSignature);
    CleanupStack::Pop(viewSignature);
#endif
    InitializeKeyStoreL();
    LOG_LEAVEFN("CSecModUIModel::ConstructL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSecModUIModel* CSecModUIModel::NewL()
    {
    LOG_ENTERFN("CSecModUIModel::NewL()");
    CSecModUIModel* self = new( ELeave ) CSecModUIModel();

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    LOG_LEAVEFN("CSecModUIModel::NewL()");
    return self;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::InitializeKeyStoreL()
// -----------------------------------------------------------------------------
//
TInt CSecModUIModel::InitializeKeyStoreL()
    {
    LOG_ENTERFN("CSecModUIModel::InitializeKeyStoreL()");
    ResetAndDestroyCTObjects();
    iUnifiedKeyStore = CUnifiedKeyStore::NewL(iEikEnv->FsSession());

    if (iWrapper == NULL)
        {
        iWrapper = CSecModUISyncWrapper::NewL();
        }

    TInt err = iWrapper->Initialize(*iUnifiedKeyStore);

    ShowErrorNoteL(err);
    if (KErrNone != err && KErrHardwareNotAvailable != err)
        {
        LOG_LEAVEFN("CSecModUIModel::InitializeKeyStoreL()");
        return err;
        }
    TInt keyStoreCount = iUnifiedKeyStore->KeyStoreCount();
    if (0 >= keyStoreCount)
        {
        LOG_LEAVEFN("CSecModUIModel::InitializeKeyStoreL()");
        return KErrNone;
        }

    RMPointerArray<CCTKeyInfo> keyInfos;
    CleanupClosePushL(keyInfos);
    TCTKeyAttributeFilter filter;
    TInt keyInfoCount = 0;
    filter.iPolicyFilter = TCTKeyAttributeFilter::EAllKeys;

    for (TInt i = 0; i < keyStoreCount; i++)
        {
        err = iWrapper->ListKeys(iUnifiedKeyStore->KeyStore(i), keyInfos, filter);
        LOG_WRITE_FORMAT( "CSecModUIModel::InitializeKeyStoreL() list err = %d ", err );
        
        //If list ok, append the AO, otherwise go next
        if ( err == KErrNone )
            {
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
        keyInfos.Close();
        }
    CleanupStack::PopAndDestroy(&keyInfos);  //keyInfos

    LOG_LEAVEFN("CSecModUIModel::InitializeKeyStoreL()");
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::AddResourceFileL()
// -----------------------------------------------------------------------------
//
void CSecModUIModel::AddResourceFileL()
  {
  LOG_ENTERFN("CSecModUIModel::AddResourceFileL()");
    // Resource file loading
  RFs& fs = iEikEnv->FsSession();

  TFileName fileName;

  TParse parse;
  // secmodui.rsc
    parse.Set(KResourceFile, &KDC_RESOURCE_FILES_DIR, NULL);

  fileName = parse.FullName();

  BaflUtils::NearestLanguageFile( fs, fileName );
  iResourceFileOffset = iEikEnv->AddResourceFileL( fileName );

  // CertManUI.rsc
  parse.Set(KResourceFile2, &KDC_RESOURCE_FILES_DIR, NULL);
    fileName = parse.FullName();
    BaflUtils::NearestLanguageFile( fs, fileName );
    iResourceFileOffset2 = iEikEnv->AddResourceFileL( fileName );

  // CTSecDlg.rsc
  parse.Set(KResourceFile3, &KDC_RESOURCE_FILES_DIR, NULL);
    fileName = parse.FullName();
    BaflUtils::NearestLanguageFile( fs, fileName );
    iResourceFileOffset3 = iEikEnv->AddResourceFileL( fileName );

  LOG_LEAVEFN("CSecModUIModel::AddResourceFileL()");
  }

// -----------------------------------------------------------------------------
// CSecModUIModel::LoadTokenLabelsL(CEikTextListBox& aListBox)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::LoadTokenLabelsL(CEikTextListBox& aListBox)
    {
    LOG_ENTERFN("CSecModUIModel::LoadTokenLabelsL()");

    TInt keyStoreCount = iAOKeyStores.Count();

    if (0 == keyStoreCount)
        {
        LOG_WRITE_FORMAT("keyStoreCount == %i",keyStoreCount);
        LOG_LEAVEFN("CSecModUIModel::LoadTokenLabelsL()");
        //disable the scrollbar if no keystore
        aListBox.ScrollBarFrame()->SetScrollBarVisibilityL(
	    CEikScrollBarFrame::EOff,CEikScrollBarFrame::EOff);
        return;
        }
    TBuf<KItemLength> item;
    TBuf<KItemLength> label;

    CDesCArray* itemArray =
        STATIC_CAST(CDesCArray*, aListBox.Model()->ItemTextArray());
    for (TInt i = 0; i < keyStoreCount; i++)
        {
        if (iAOKeyStores[i]->Token().TokenType().Type().iUid 
            == KTokenTypeFileKeystore)
            {
            AppendResourceL(label, R_QTN_KEYSTORE_LIST_TEXT_PHONE_KEYSTORE);
            }
        else 
            {                
            label = iAOKeyStores[i]->Token().Label();
            }
        item += KTab;
        item += label;
        item += KTab;
        AppendLocationL(item, iAOKeyStores[i]->Token().TokenType().Type());
        item.SetLength(item.Length() - 1); // Remove \n from the end
        itemArray->AppendL(item);
        label.Zero();
        item.Zero();
        }

    LOG_LEAVEFN("CSecModUIModel::LoadTokenLabelsL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::CheckCodeViewStringsL(MDesCArray& aItemArray)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::CheckCodeViewStringsL(MDesCArray& aItemArray)
    {
    LOG_ENTERFN("CSecModUIModel::CheckCodeViewStringsL()");       
    InitAuthObjectsL();
    ListAuthObjectsL();
        
    if (0 >= iAOArray.Count())
        {
        // Handle error
        }
    else if (1 == iAOArray.Count())
        {
        // Only PIN-G exists, let's remove "signing codes" from the list
        CDesCArray* itemArray = STATIC_CAST(CDesCArray*, &aItemArray);
        itemArray->Delete(1); // signing code is in the second position
        }
    else
        {
        // Do nothing
        }
    LOG_LEAVEFN("CSecModUIModel::CheckCodeViewStringsL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::InitAuthObjectsL()
// -----------------------------------------------------------------------------
//
void CSecModUIModel::InitAuthObjectsL()
    {
    LOG_ENTERFN("CSecModUIModel::InitAuthObjectsL()");
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
            ShowErrorNoteL(err);
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
                ShowErrorNoteL(err);
                LOG_LEAVEFN("CSecModUIModel::InitAuthObjectsL()");
                User::Leave(err);
                }
            iAOList = (MCTAuthenticationObjectList*)tokenInterface;
            }
        }
    LOG_LEAVEFN("CSecModUIModel::InitAuthObjectsL()");
    }
// -----------------------------------------------------------------------------
// CSecModUIModel::ListAuthObjectsL()
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ListAuthObjectsL()
    {
    LOG_ENTERFN("CSecModUIModel::ListAuthObjectsL()");
    if (0 >= iAOArray.Count())
        {
        __ASSERT_ALWAYS(iAOList, Panic(EPanicNullPointer));
        TInt err = iWrapper->ListAuthObjects(*iAOList, iAOArray);
        if (err)
            {
            ShowErrorNoteL(err);
            User::Leave(err);
            }
        }
    LOG_LEAVEFN("CSecModUIModel::ListAuthObjectsL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::OpenTokenL(TInt aTokenIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::OpenTokenL(TInt aTokenIndex)
    {
    LOG_ENTERFN("CSecModUIModel::OpenTokenL()");
    ResetAndDestroyAOs();
    iKeyStore = iAOKeyStores[aTokenIndex];
    ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL(KSecModUIViewCodeId);
    LOG_LEAVEFN("CSecModUIModel::OpenTokenL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::OpenAuthObjViewL(TInt aIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::OpenAuthObjViewL(TInt aIndex)
    {
    LOG_ENTERFN("CSecModUIModel::OpenAuthObjViewL()");
    if (KPinGSettIndex==aIndex)
        {
        ((CAknViewAppUi*)iAvkonAppUi)->ActivateLocalViewL(KSecModUIViewAccessId);
        }
    else if (KPinNrSettIndex==aIndex)
        {
        ((CAknViewAppUi*)iAvkonAppUi)->
            ActivateLocalViewL(KSecModUIViewSignatureId);
        }
    else
        {

        }
    LOG_LEAVEFN("CSecModUIModel::OpenAuthObjViewL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::CloseAuthObjL(TInt aIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::CloseAuthObjL(TInt aIndex)
    {
    LOG_ENTERFN("CSecModUIModel::CloseAuthObjL()");
    if (PinOpen(aIndex))
        {
        __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
        TInt err = iWrapper->CloseAuthObject(*(iAOArray[aIndex]));
        if (err)
            {
            ShowErrorNoteL(err);
            User::Leave(err);
            }
        }
    LOG_LEAVEFN("CSecModUIModel::CloseAuthObjL()");
    }


// -----------------------------------------------------------------------------
// CSecModUIModel::ViewSecModDetailsL(TInt aTokenIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ViewSecModDetailsL(TInt aTokenIndex)
    {
    LOG_ENTERFN("CSecModUIModel::ViewSecModDetailsL(TInt aTokenIndex)");
    iKeyStore = iAOKeyStores[aTokenIndex];
    InitAuthObjectsL();
    ListAuthObjectsL();
    ViewOpenedSecModDetailsL();
    iKeyStore = NULL; // not owned
    if (iAOList)
        {
        iAOList->Release();
        iAOList = NULL;
        }
    iAOArray.Reset();

    LOG_LEAVEFN("CSecModUIModel::ViewSecModDetailsL(TInt aTokenIndex)");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ViewOpenedSecModDetailsL()
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ViewOpenedSecModDetailsL()
    {
    LOG_ENTERFN("CSecModUIModel::ViewSecModDetailsL()");
    // Create message buffer
    HBufC* message = HBufC::NewLC(KMaxLengthTextDetailsBody);
    TPtr messagePtr = message->Des();
    MCTToken& token = iKeyStore->Token();
    
    if (token.TokenType().Type().iUid == KTokenTypeFileKeystore)
        { 
        AppendItemL(messagePtr, R_QTN_WIM_NAME,
            KNullDesC, R_QTN_KEYSTORE_LIST_TEXT_PHONE_KEYSTORE);        
        }
    else 
        {            
        AppendItemL(messagePtr, R_QTN_WIM_NAME,
            token.Label(), R_TEXT_RESOURCE_VIEW_NO_LABEL_DETAILS);
        }

    AppendItemL(messagePtr, R_QTN_WIM_CARD_VERSION,
        token.Information( MCTToken::EVersion ),
        R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED );

    AppendLocationL(messagePtr, token.TokenType().Type(), R_QTN_WIM_SECURITY_MODULE_LOCATION);
    messagePtr.Append(KEnter); // AppendLocationL does not add enter as others
    
    if (token.TokenType().Type().iUid == KTokenTypeFileKeystore)
        { 
        AppendItemL(messagePtr, R_QTN_WIM_SETT_PIN_G,
            KNullDesC, R_QTN_SECMOD_TITLE_PHONE_KEYSTORE_CODE);        
        }
    else 
        {            
        AppendItemL(messagePtr, R_QTN_WIM_SETT_PIN_G,
            iAOArray[0]->Label(), R_QTN_WIM_PIN_G_NO_LABEL_DETAILS);
        }

    AppendPinNRsL(messagePtr);

    AppendItemL(messagePtr, R_QTN_WIM_SERIAL_NRO,
        token.Information( MCTToken::ESerialNo),
        R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED);

    AppendItemL(messagePtr, R_QTN_WIM_MANUF,
        token.Information( MCTToken::EManufacturer),
        R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED);

    CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL(*message);
    dlg->PrepareLC(R_SECMOD_DETAILS_VIEW);
    dlg->RunLD();
    CleanupStack::PopAndDestroy(message);
    LOG_LEAVEFN("CSecModUIModel::ViewSecModDetailsL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::AppendPinNRsL(TDes& aMessage) const
// -----------------------------------------------------------------------------
//
void CSecModUIModel::AppendPinNRsL(TDes& aMessage) const
    {
    TInt pinCount = iAOArray.Count();
    HBufC* value = NULL;
    TInt j = 1;

    for (TInt i=1; i<pinCount; i++)
      {
      AppendResourceAndEnterL(aMessage, R_QTN_WIM_KEY_PIN);
      if (iAOArray[i]->Label().Length() > 0)
          {
          aMessage.Append(iAOArray[i]->Label());
          }
      else
          {
          value = StringLoader::LoadLC(R_QTN_WIM_PIN_NR_NO_LABEL_DETAILS, j);
          aMessage.Append(*value);
          j++;
          }
      aMessage.Append(KDoubleEnter);
      }
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::AppendLocationL(...)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::AppendLocationL(
    TDes& aMessage,
    TUid aUid,
    TInt aItemRes) const
    {
    TInt location = 0;


    switch ( aUid.iUid )
        {
        case KTokenTypeFileKeystore:
            {
            location = R_TEXT_RESOURCE_DETAILS_VIEW_LOCATION_PHONE_MEMORY;
            break;
            }
        case KWIMStoreUid:
            {
            location = R_TEXT_RESOURCE_DETAILS_VIEW_LOCATION_SMART_CARD;
            break;
            }
        default:
            {
            break;
            }
        }
    if (0 != aItemRes)
        {
        AppendResourceAndEnterL(aMessage, aItemRes);
        }
    AppendResourceAndEnterL(aMessage, location);
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::AppendItemL(...)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::AppendItemL(
    TDes& aMessage,
    TInt aResItem,
    const TDesC& aValue,
    TInt aResNotDefined) const
    {
    AppendResourceAndEnterL(aMessage, aResItem);
    AppendValueL(aMessage, aValue, aResNotDefined);
    aMessage.Append(KEnter);
    }

// ---------------------------------------------------------
// CSecModUIModel::AppendResourceL(TDes& aMessage, TInt aResource)
// Appends aResource to aMessage
// ---------------------------------------------------------
//
void CSecModUIModel::AppendResourceL(TDes& aMessage, TInt aResource) const
  {
  HBufC* stringHolder = StringLoader::LoadLC(aResource);
  aMessage.Append(*stringHolder);
  CleanupStack::PopAndDestroy(stringHolder);  
  }
  
// ---------------------------------------------------------
// CSecModUIModel::AppendResourceL(TDes& aMessage, TInt aResource)
// Appends aResource to aMessage
// ---------------------------------------------------------
//
void CSecModUIModel::AppendResourceAndEnterL(TDes& aMessage, TInt aResource) const
  {
  HBufC* stringHolder = StringLoader::LoadLC(aResource);
  aMessage.Append(*stringHolder);
  CleanupStack::PopAndDestroy(stringHolder);
  aMessage.Append(KEnter);
  }

// -----------------------------------------------------------------------------
// CSecModUIModel::AppendValueL(...)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::AppendValueL(
    TDes& aMessage,
    const TDesC& aValue,
    TInt aResNotDefined) const
  {
  HBufC* buf = HBufC::NewLC(KMaxLengthItemValue);
  buf->Des() = aValue;
  buf->Des().TrimLeft();
  TInt length = buf->Des().Length();
  if (length == 0 )
    {
    AppendResourceAndEnterL(aMessage, aResNotDefined);
    }
  else
    {
    aMessage.Append(*buf);
    aMessage.Append(KEnter);
    }
  CleanupStack::PopAndDestroy(buf);
  }

// -----------------------------------------------------------------------------
// CSecModUIModel::DeleteKeyStoreL(TInt aTokenIndex)
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::DeleteKeyStoreL(TInt aTokenIndex)
    {
    LOG_ENTERFN("CSecModUIModel::DeleteKeyStoreL()");
    __ASSERT_ALWAYS(aTokenIndex < iAOKeyStores.Count(), Panic(EPanicIndexOutOfRange));
    // ask confirmation from the user
    if (ShowConfirmationQueryL(R_QTN_CM_CONFIRM_DELETE_KEYS))
        {
        RMPointerArray<CCTKeyInfo> keyInfos;
        CleanupClosePushL(keyInfos);
        TCTKeyAttributeFilter filter;
        filter.iPolicyFilter = TCTKeyAttributeFilter::EAllKeys;
        TInt err = iWrapper->ListKeys(*iAOKeyStores[aTokenIndex], keyInfos, filter);
        ShowErrorNoteL(err);
        User::LeaveIfError(err);
        for (TInt i = 0; i < keyInfos.Count(); ++i)
            {
            err = iWrapper->DeleteKey(*iUnifiedKeyStore, keyInfos[i]->Handle());
            ShowErrorNoteL(err);

            if (KErrHardwareNotAvailable == err )
                {
                break; // Break the loop, if keystore not available
                }
            }
        CleanupStack::PopAndDestroy(&keyInfos);  //keyInfos
        ResetAndDestroyCTObjects();
        InitializeKeyStoreL();
        LOG_LEAVEFN("CSecModUIModel::DeleteKeyStoreL()");
        return ETrue;
        }
    else
        {
        LOG_LEAVEFN("CSecModUIModel::DeleteKeyStoreL()");
        return EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::IsTokenDeletable(TInt aTokenIndex)
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::IsTokenDeletable(TInt aTokenIndex)
    {
    LOG_ENTERFN("CSecModUIModel::IsTokenDeletable()");
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
    LOG_LEAVEFN("CSecModUIModel::IsTokenDeletable()");
    return ret;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::LoadPinNRLabelsL(...)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::LoadPinNRLabelsL(CEikTextListBox& aListBox, TBool aShowBlockedNote)
    {
    LOG_ENTERFN("CSecModUIModel::LoadPinNRLabelsL()");
    TBuf<KMaxSettItemSize> item;
    TBuf<KMaxSettItemSize> label;
    CDesCArray* itemArray = STATIC_CAST(CDesCArray*, aListBox.Model()->ItemTextArray());
    itemArray->Reset();

    LOG_WRITE_FORMAT("CSecModUIModel::LoadPinNRLabelsL, iAOArray.Count() == %i",iAOArray.Count());
    TUint32 status = 0;
    // Here is assumed that the first AO is always PIN-G
    for (TInt i = 1; i < iAOArray.Count(); i++)
        {
        label = iAOArray[i]->Label();
        status = iAOArray[i]->Status();
        LOG_WRITE_FORMAT("PIN-NR status == %i", status);
        if (status & EAuthObjectBlocked)
            {
            if (aShowBlockedNote)
                {
                if (status & EUnblockDisabled)
                    {
                    ShowInformationNoteL(R_QTN_WIM_PIN_TOTALBLOCKED, label);
                    }
                else
                    {
                    ShowInformationNoteL(R_QTN_WIM_ERR_PIN_BLOCKED, label);
                    }
                }
            HBufC* value = StringLoader::LoadLC(R_QTN_WIM_STATUS_BLOCKED);
            CreateSettingItem(label, *value, item );
            CleanupStack::PopAndDestroy(value);
            }
        else
            {
            CreateSettingItem(label, KFourAsterisks, item);
            }
        itemArray->AppendL(item);
        label.Zero();
        item.Zero();
        }
    LOG_LEAVEFN("CSecModUIModel::LoadPinNRLabelsL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::LoadPinGItemsL(CEikTextListBox& aListBox)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::LoadPinGItemsL(CEikTextListBox& aListBox)
    {
    LOG_ENTERFN("CSecModUIModel::LoadPinGItemsL()");
    TBuf<KMaxSettItemSize> item;
    TBuf<KMaxSettItemSize> label;

    CDesCArray* itemArray = STATIC_CAST(CDesCArray*,
        aListBox.Model()->ItemTextArray());

    if (iKeyStore->Token().TokenType().Type().iUid 
        == KTokenTypeFileKeystore)
        {
        AppendResourceL(label, R_QTN_SECMOD_TITLE_PHONE_KEYSTORE_CODE);
        }
    else
        {                
        label = iAOArray[KPinGIndex]->Label();
        }
    CreateSettingItem(label, KFourAsterisks, item);
    itemArray->AppendL(item);
    item.Zero();
    PINRequestItemL(KPinGIndex, item);
    itemArray->AppendL(item);
    item.Zero();
    PINStatusItemL(KPinGIndex, item, ETrue);
    itemArray->AppendL(item);
    LOG_LEAVEFN("CSecModUIModel::LoadPinGItemsL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::PINRequestItemL(...)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::PINRequestItemL(TInt aIndex, TDes& aItem) const
    {
    LOG_ENTERFN("CSecModUIModel::PINRequestItemL()");
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TUint32 status = iAOArray[aIndex]->Status();
    HBufC* title = NULL;
    HBufC* value = NULL;
    title = StringLoader::LoadLC(R_QTN_WIM_PIN_REQUEST);

    if (status & EEnabled)
        {
        value = StringLoader::LoadLC(R_QTN_WIM_REQUEST_ON);
        CreateSettingItem(*title, *value, aItem );
        }
    else
        {
        value = StringLoader::LoadLC(R_QTN_WIM_REQUEST_OFF);
        CreateSettingItem(*title, *value, aItem );
        }
    CleanupStack::PopAndDestroy(2, title);
    LOG_LEAVEFN("CSecModUIModel::PINRequestItemL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::PINStatusItemL(...)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::PINStatusItemL(
    TInt aIndex,
    TDes& aItem,
    TBool aShowBlockedNote) const
    {
    LOG_ENTERFN("CSecModUIModel::PINStatusItemL()");
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TUint32 status = iAOArray[aIndex]->Status();
    HBufC* title = NULL;
    HBufC* value = NULL;
    title = StringLoader::LoadLC(R_QTN_WIM_SETT_STATUS);

    LOG_WRITE_FORMAT("PIN-G status == %i", status);

    if (status & EAuthObjectBlocked)
        {
        if (aShowBlockedNote)
            {
            if (status & EUnblockDisabled)
                {
                ShowInformationNoteL(
                    R_QTN_WIM_PIN_TOTALBLOCKED, iAOArray[aIndex]->Label());
                }
            else
                {
                ShowInformationNoteL(
                    R_QTN_WIM_ERR_PIN_BLOCKED, iAOArray[aIndex]->Label());
                }
            }            
        value = StringLoader::LoadLC(R_QTN_SECMOD_STATUS_VALUE_BLOCKED);
        }
    else if ( PinOpen(aIndex) )
        {
        value = StringLoader::LoadLC(R_QTN_WIM_OPT_OPEN);
        }
    else
        {
        value = StringLoader::LoadLC(R_QTN_WIM_STATUS_CLOSED);
        }
    CreateSettingItem(*title, *value, aItem );
    CleanupStack::PopAndDestroy(2, title);
    LOG_LEAVEFN("CSecModUIModel::PINStatusItemL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::CreateSettingItem(...)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::CreateSettingItem(
    const TDesC& aTitle, const TDesC& aValue, TDes& aItem) const
  {
    LOG_ENTERFN("CSecModUIModel::CreateSettingItem()");
    aItem += KTab;
    aItem += aTitle;
    aItem += KDoubleTab;
    aItem += aValue;
    LOG_LEAVEFN("CSecModUIModel::CreateSettingItem()");
  }

// -----------------------------------------------------------------------------
// CSecModUIModel::eOrUnblockPinNrL(TInt aIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ChangeOrUnblockPinNrL(TInt aIndex)
    {
    LOG_ENTERFN("CSecModUIModel::ChangeOrUnblockPinNrL()");
    // Add PIN-G to the list
    ChangeOrUnblockPinL(aIndex+1);
    LOG_LEAVEFN("CSecModUIModel::ChangeOrUnblockPinNrL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ChangeOrUnblockPinL(TInt aIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ChangeOrUnblockPinL(TInt aIndex)
    {
    LOG_ENTERFN("CSecModUIModel::ChangeOrUnblockPinL()");
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
    else if (status & EChangeDisabled)
        {
        // We cannot do anything. Change is disabled.
        }
     else if (status & EEnabled)
        {
        // PIN is enabled. Let's change the PIN.
        ChangePinL(aIndex);
        }
     else
        {
        // PIN is disabled. Not possible to change PIN.
        }
    LOG_LEAVEFN("CSecModUIModel::ChangeOrUnblockPinL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ChangePinNrL(TInt aIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ChangePinNrL(TInt aIndex)
    {
    LOG_ENTERFN("CSecModUIModel::ChangePinNrL()");
    ChangePinL(aIndex+1);
    LOG_LEAVEFN("CSecModUIModel::ChangePinNrL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ChangePinL(TInt aIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ChangePinL(TInt aIndex)
    {
    LOG_ENTERFN("CSecModUIModel::ChangePinL()");
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TInt err = KErrNone;
  
    err = iWrapper->ChangeReferenceData(*(iAOArray[aIndex]));
    if (err)
        {
        ShowErrorNoteL(err);
        }
    else
        {
        ShowConfirmationNoteL(R_QTN_WIM_INFO_PIN_CHANGED);
        }
    
    LOG_LEAVEFN("CSecModUIModel::ChangePinL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::UnblockPinNrL(TInt aIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::UnblockPinNrL(TInt aIndex)
    {
    LOG_ENTERFN("CSecModUIModel::UnblockPinNrL()");
    UnblockPinL(aIndex+1);
    LOG_LEAVEFN("CSecModUIModel::UnblockPinNrL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::UnblockPinL(TInt aIndex)
// -----------------------------------------------------------------------------
//
void CSecModUIModel::UnblockPinL(TInt aIndex)
    {
    LOG_ENTERFN("CSecModUIModel::UnblockPinL()");
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TInt err = iWrapper->UnblockAuthObject(*(iAOArray[aIndex]));
    if (KErrNone == err)
        {
        ShowConfirmationNoteL(R_QTN_WIM_INFO_PIN_UNBLOCKED);
        }
    else
        {
        ShowErrorNoteL(err);
        }
    LOG_LEAVEFN("CSecModUIModel::UnblockPinL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::PinNrChangeable(TInt aIndex) const
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinNrChangeable(TInt aIndex) const
    {
    // Add PIN-G to the list
    return PinChangeable(aIndex+1);
    }
// -----------------------------------------------------------------------------
// CSecModUIModel::PinChangeable(TInt aIndex) const
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinChangeable(TInt aIndex) const
    {
    LOG_ENTERFN("CSecModUIModel::PinChangeable()");
    __ASSERT_ALWAYS(aIndex < iAOArray.Count(), Panic(EPanicIndexOutOfRange));
    TUint32 status = iAOArray[aIndex]->Status();
    TBool ret = ETrue;
    if ( (status & EAuthObjectBlocked) || (status & EChangeDisabled) )
    //if ( status & (EAuthObjectBlocked | EChangeDisabled))
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
    LOG_LEAVEFN("CSecModUIModel::PinChangeable()");
    return ret;
    }
// -----------------------------------------------------------------------------
// CSecModUIModel::PinNrUnblockable(TInt aIndex) const
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinNrUnblockable(TInt aIndex) const
    {
    // Add PIN-G to the list
    return PinUnblockable(aIndex+1);
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::PinUnblockable(TInt aIndex) const
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinUnblockable(TInt aIndex) const
    {
    LOG_ENTERFN("CSecModUIModel::PinUnblockable()");
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
    LOG_LEAVEFN("CSecModUIModel::PinUnblockable()");
    return ret;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::PinOpen(TInt aIndex) const
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinOpen(TInt aIndex) const
    {
    LOG_ENTERFN("CSecModUIModel::PinOpen()");
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
    LOG_LEAVEFN("CSecModUIModel::PinOpen()");
    return ret;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ChangeCodeRequestL(TInt aIndex)
// -----------------------------------------------------------------------------
//
TInt CSecModUIModel::ChangeCodeRequestL(TInt aIndex)
    {
    LOG_ENTERFN("CSecModUIModel::ChangeCodeRequestL()");
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
        ShowErrorNoteL(err);
        }
    LOG_LEAVEFN("CSecModUIModel::ChangeCodeRequestL()");
    return err;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::PinRequestChangeable(TInt aIndex) const
// -----------------------------------------------------------------------------
//
TBool CSecModUIModel::PinRequestChangeable(TInt aIndex) const
    {
    LOG_ENTERFN("CSecModUIModel::PinRequestChangeable()");
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
    LOG_LEAVEFN("CSecModUIModel::PinRequestChangeable()");
    return ret;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ShowErrorNoteL(TInt aError) const
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ShowErrorNoteL(TInt aError) const
    {
    LOG_ENTERFN("CSecModUIModel::ShowErrorNoteL()");
    HBufC* text = NULL;

    switch (aError)
        {
        case KErrCancel:
        case KErrLocked:
        case KErrNone:
            {
            LOG_WRITE_FORMAT("aError == %i",aError);
            LOG_LEAVEFN("CSecModUIModel::ShowErrorNoteL()");
            break;
            }
        case KErrInUse:
            {
            text = StringLoader::LoadLC(R_QTN_KEYSTORE_INFO_UNABLE_TO_DELETE);
            break;
            }
        case KErrBadPassphrase:
            {
            text = StringLoader::LoadLC(R_QTN_WIM_ERR_WRONG_PIN);
            break;
            }
        case KErrHardwareNotAvailable: // flow thru
        default:
            {
            LOG_WRITE_FORMAT("ERROR: aError == %i",aError);
            text = StringLoader::LoadLC(R_QTN_WIM_ERR_IO_ERROR);
            break;
            }
        }

    if (text)
        {
        CAknErrorNote* note = new (ELeave) CAknErrorNote;
        note->ExecuteLD(*text);
        CleanupStack::PopAndDestroy(text);
        }
    LOG_LEAVEFN("CSecModUIModel::ShowErrorNoteL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ShowConfirmationQueryL(TInt aResourceId) const
// -----------------------------------------------------------------------------
//
TInt CSecModUIModel::ShowConfirmationQueryL(TInt aResourceId) const
    {
    LOG_ENTERFN("CSecModUIModel::ShowConfirmationQueryL ()");
    HBufC* text = NULL;
    text = StringLoader::LoadLC(aResourceId);
    CAknQueryDialog* dlg = CAknQueryDialog::NewL(CAknQueryDialog::ENoTone);
    TInt selection = dlg->ExecuteLD(R_SECMOD_CONFIRMATION_QUERY_YES_NO, *text);
    CleanupStack::PopAndDestroy(text);
    LOG_LEAVEFN("CSecModUIModel::ShowConfirmationNoteL()");
    return selection;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ShowConfirmationNoteL(TInt aResourceId) const
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ShowConfirmationNoteL(TInt aResourceId) const
    {
    LOG_ENTERFN("CSecModUIModel::ShowConfirmationNoteL()");
    HBufC* text = NULL;
    text = StringLoader::LoadLC(aResourceId);
    CAknConfirmationNote * note = new (ELeave) CAknConfirmationNote;
    note->ExecuteLD(*text);
    CleanupStack::PopAndDestroy(text);
    LOG_LEAVEFN("CSecModUIModel::ShowConfirmationNoteL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ShowInformationNoteL(...)
// Global note needs to be used here because otherwise view might cancel
// the query too early when the view is activated.
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ShowInformationNoteL(
    TInt aResourceId,
    const TDesC& aString) const
    {
    LOG_ENTERFN("CSecModUIModel::ShowInformationNoteL()");
    HBufC* text = NULL;
    if (0 == aString.Length())
        {
        text = StringLoader::LoadLC(aResourceId);
        }
    else
        {
        text = StringLoader::LoadLC(aResourceId, aString);
        }
    CAknGlobalNote * note = CAknGlobalNote::NewLC();
    note->ShowNoteL(EAknGlobalInformationNote, *text);
    CleanupStack::PopAndDestroy(2, text);
    LOG_LEAVEFN("CSecModUIModel::ShowInformationNoteL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ActivateTitleL(const TDesC& aTitle) const
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ActivateTitleL(const TDesC& aTitle) const
    {
    LOG_ENTERFN("CSecModUIModel::ActivateTitleL()");
    TUid titlePaneUid;
    titlePaneUid.iUid = EEikStatusPaneUidTitle;
    CEikStatusPane* sp = iAvkonAppUi->StatusPane();

    CEikStatusPaneBase::TPaneCapabilities subPane =
      sp->PaneCapabilities(titlePaneUid);

    if (subPane.IsPresent() && subPane.IsAppOwned())
      {
      // Fetch pointer to the default title pane control
      CAknTitlePane* title = STATIC_CAST(
        CAknTitlePane*, sp->ControlL(titlePaneUid) );
        title->SetTextL(aTitle);
      }
    LOG_LEAVEFN("CSecModUIModel::ActivateTitleL()");
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::ActivateTokenLabelToTitleL() const
// -----------------------------------------------------------------------------
//
void CSecModUIModel::ActivateTokenLabelToTitleL() const
    {
    if (iKeyStore->Token().TokenType().Type().iUid 
        == KTokenTypeFileKeystore)
        {
        HBufC* stringHolder = 
            StringLoader::LoadLC(R_QTN_KEYSTORE_TITLE_PHONE_KEYSTORE);
        ActivateTitleL(*stringHolder);
        CleanupStack::PopAndDestroy(stringHolder);        
        }
    else
        {            
        ActivateTitleL(iKeyStore->Token().Label());
        }
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::Wrapper()
// -----------------------------------------------------------------------------
//
CSecModUISyncWrapper& CSecModUIModel::Wrapper()
    {
    return *iWrapper;
    }

// -----------------------------------------------------------------------------
// CSecModUIModel::SecurityModuleCount() const
// -----------------------------------------------------------------------------
//
TInt CSecModUIModel::SecurityModuleCount() const
    {
    return iAOKeyStores.Count();
    }

//  End of File
