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
* Description:  Container for the Device & SIM security sub-folder
*
*/


#include "GSSimSecPluginContainer.h"
#include "GSSimSecPluginModel.h"
#include "GSSimSecPlugin.hrh"

#include <aknlists.h>
#include <etelmm.h>
#include <RSSSettings.h>
#include <StringLoader.h>
#include <featmgr.h>
#include <csxhelp/cp.hlp.hrh>
#include <gsfwviewuids.h>
#include <GSSimSecPluginRsc.rsg>
#include <gslistbox.h>
#include <AknsConstants.h>
#include <secuisecuritysettings.h>


#ifdef RD_REMOTELOCK
#include <RemoteLockSettings.h>
_LIT( KRemoteLockEmptyItem, " " );
#endif

#include    <SCPClient.h>                                                  
// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS
_LIT( CodeItem,"****" );
_LIT( SecurityCodeItem, "*****" );

const TInt KMaxStringLength = 50;

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ==============================


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::ConstructL()
// 
// Symbian OS two phased constructor
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::ConstructL( const TRect& aRect )
    { 
    iListBox = new( ELeave ) CAknSettingStyleListBox;
    BaseConstructL( aRect, R_GS_SIM_SECURITY_VIEW_TITLE, R_SIM_SECURITY_LBX );
    FeatureManager::InitializeLibL();
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::~CGSSettListSecurityContainer()
// 
// Destructor 
// ---------------------------------------------------------------------------
//

CGSSimSecPluginContainer::~CGSSimSecPluginContainer()
    {
    if(iCodeItems)
        delete iCodeItems;
    if(iUpinItems)
        delete iUpinItems;
    if(iAutoLockItems)
        delete iAutoLockItems;
    if(iPinItems)
        delete iPinItems;
    if(iSecurityItems)
        delete iSecurityItems;
    if(iSatItems)
        delete iSatItems;
    if(iSecurity)
        delete iSecurity;
    if(iListboxItemArray)
        delete iListboxItemArray;
#ifdef RD_REMOTELOCK
    if(iRemoteLockItems)
        delete iRemoteLockItems;
#endif // RD_REMOTELOCK    
FeatureManager::UnInitializeLib();
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::CGSSimSecPluginContainer()
// 
// Constructor
// ---------------------------------------------------------------------------
//
CGSSimSecPluginContainer::CGSSimSecPluginContainer(
                          CGSSimSecPluginModel* aModel )
    : iModel ( aModel )
    {
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::ConstructListBoxL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::ConstructListBoxL( TInt aResLbxId )
    {
    iSecurity= CSecuritySettings::NewL();
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
        {
        //check if UPIN code is supported and not rejected
        TBool resp = iSecurity->IsUpinSupportedL();
        if( resp )
            {
            iUpinCodeSupported = ETrue;
    
            //check if UPIN code is active
            if( iSecurity->IsUpinActive() )
                {
                iUpinCodeActive = ETrue;
                }
            else
                {
                iUpinCodeActive = EFalse;
                }
            
            //Check whether UPIN is blocked
            resp = iSecurity->IsUpinBlocked();
    
            if( !resp )
                {
                iUpinCodeRejected = EFalse;
                }
            else
                {
                iUpinCodeRejected = ETrue;    
                }
            }
        else
            {
            iUpinCodeSupported = EFalse;
            iUpinCodeActive = EFalse;
            }
        }
    

    iListBox->ConstructL( this, EAknListBoxSelectionList );
    iListboxItemArray = CGSListBoxItemTextArray::NewL( aResLbxId, 
                        *iListBox, *iCoeEnv );
    iListBox->Model()->SetItemTextArray( iListboxItemArray );
    iListBox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );

    iAutoLockItems = iCoeEnv->ReadDesC16ArrayResourceL( R_AUTOLOCK_ARRAY );

#ifdef RD_REMOTELOCK
    #ifdef _DEBUG
    RDebug::Print( _L( "(GS)CGSSettListSimSecurityContainer::ConstructListBoxL() - Reading R_REMOTELOCK_ARRAY resource" ) );
    #endif // DEBUG

    iRemoteLockItems = iCoeEnv->ReadDesC16ArrayResourceL( R_REMOTELOCK_ARRAY );
#endif // RD_REMOTELOCK


    if(wcdmaSupported || upinSupported)
      {
        iUpinItems = iCoeEnv->ReadDesC16ArrayResourceL( R_UPIN_ARRAY );
        iCodeItems = iCoeEnv->ReadDesC16ArrayResourceL( R_CODE_ARRAY );
      }
    else
        {
        iUpinItems = NULL;
        iCodeItems = NULL;
        }

    iPinItems = iCoeEnv->ReadDesC16ArrayResourceL( R_PIN_ARRAY );
    iSecurityItems = iCoeEnv->ReadDesC16ArrayResourceL( R_SECURITY_ARRAY );
    iSatItems = iCoeEnv->ReadDesC16ArrayResourceL( R_SAT_ARRAY );

    CreateListBoxItemsL();
    }


// ---------------------------------------------------------------------------
// CGSSettListIdleContainer::CreateListBoxItemsL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::CreateListBoxItemsL()
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));

    if(wcdmaSupported || upinSupported)
      {
        if( iUpinCodeSupported && !iUpinCodeRejected )
            {
            MakeCodeInUseItemL();
            }
    
        if( iUpinCodeActive )
            {
            MakeUpinRequestItemL();
            MakeCodeItemL( EGSSettIdUpinCode );
            }
        else
            {
            MakePinRequestItemL();
            MakeCodeItemL( EGSSettIdPinCode );
            }
       }
    else //not wcdma or upin
        {   
            MakePinRequestItemL();
            MakeCodeItemL( EGSSettIdPinCode );
        }

    MakeCodeItemL( EGSSettIdPin2Code );
    MakeAutolockItemL();
    
#ifdef RD_REMOTELOCK
    MakeRemoteLockItemL();
#endif // RD_REMOTELOCK
    
    MakeCodeItemL( EGSSettIdSecurityCode );
    MakeSimChangeItemL();
		if(!FeatureManager::FeatureSupported( KFeatureIdFfNoCugSupport ))
    	{ 
    		#ifdef _DEBUG
    		RDebug::Print( _L( "(GS)CGSSettListSimSecurityContainer::CreateListBoxItemsL() CUG supported!" ) );
    		#endif // DEBUG
    		MakeClosedUserGroupItemL();
    	}
    else
    {
    #ifdef _DEBUG
    RDebug::Print( _L( "(GS)CGSSettListSimSecurityContainer::CreateListBoxItemsL() CUG Not supported!" ) );
    #endif // DEBUG
    }
    
    TInt support = iModel->ConfirmSatOperationsSupport();
    
    if ( support == 1 ) // Confirmation allowed.
        {
        MakeSatOperationsItemL();
        }
    }

// ---------------------------------------------------------------------------
// CGSSettListIdleContainer::UpdateListBoxL( TInt aFeatureId )
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::UpdateListBoxL( TInt aFeatureId )
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    switch( aFeatureId )
        {
        case EGSSettIdCodeInUse:
            if(wcdmaSupported || upinSupported)
              {
                  RecreatePinItemsL();
              }
            break;
        case EGSSettIdUpinRequest:
            if(wcdmaSupported || upinSupported)
              {
                  MakeUpinRequestItemL();
              }
            break;
        case EGSSettIdUpinCode:
            if(wcdmaSupported || upinSupported)
              {
                  MakeCodeItemL( EGSSettIdUpinCode );
              }
            break;
        case EGSSettIdPinRequest:
            MakePinRequestItemL();
            break;
        case EGSSettIdPinCode:
            MakeCodeItemL( EGSSettIdPinCode );
            break;
        case EGSSettIdPin2Code:
            MakeCodeItemL( EGSSettIdPin2Code );
            break;
        case EGSSettIdAutolock:
            MakeAutolockItemL();
            break;
        case EGSSettIdSecurityCode:
            MakeCodeItemL( EGSSettIdSecurityCode );
            break;
        case EGSSettIdSimChange:
            MakeSimChangeItemL();
            break;
            
    #ifdef RD_REMOTELOCK
        case EGSSettIdRemoteLock:
            MakeRemoteLockItemL();
            break;
    #endif // RD_REMOTELOCK    
        case EGSSettIdClosedUserGroup:
        		if(!FeatureManager::FeatureSupported( KFeatureIdFfNoCugSupport ))
    				{ 
            	MakeClosedUserGroupItemL();
            }
            break;
        case EGSSettIdSatOperations:
            MakeSatOperationsItemL();
            break;
        default:
            return;
        }
    
    iListBox->HandleItemAdditionL();
    }

// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::MakeCodeItemL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::MakeCodeItemL( const TInt aItemType )
    {
    HBufC* dynamicText = HBufC::NewLC( KGSBufSize128 );
    TPtr ptrBuffer ( dynamicText->Des() );
    
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));

    if(wcdmaSupported || upinSupported)
      {
        iUpinCodeSupported = iSecurity->IsUpinSupportedL();
        iUpinCodeActive = iSecurity->IsUpinActive(); 
        iUpinCodeRejected = iSecurity->IsUpinBlocked();
      }
    else
      {
        iUpinCodeSupported = EFalse;
        iUpinCodeActive = EFalse; 
        iUpinCodeRejected = EFalse;
      }

    switch ( aItemType )
        {
        case EGSSettIdUpinCode:
            if( iUpinCodeSupported && !iUpinCodeRejected && iUpinCodeActive )
                {
                ptrBuffer =  CodeItem;
                iListboxItemArray->SetDynamicTextL( EGSSettIdUpinCode, ptrBuffer );
                iListboxItemArray->SetItemVisibilityL( EGSSettIdUpinCode,
                                   CGSListBoxItemTextArray::EVisible );
                }
            else
                {
                iListboxItemArray->SetItemVisibilityL( EGSSettIdUpinCode,
                                   CGSListBoxItemTextArray::EInvisible );
                }
            break;
        case EGSSettIdPin2Code:
            ptrBuffer =  CodeItem;
            iListboxItemArray->SetDynamicTextL( EGSSettIdPin2Code, ptrBuffer );
            iListboxItemArray->SetItemVisibilityL( EGSSettIdPin2Code, 
                               CGSListBoxItemTextArray::EVisible );
            break;
        case EGSSettIdSecurityCode:
            ptrBuffer =  SecurityCodeItem;
            iListboxItemArray->SetDynamicTextL( EGSSettIdSecurityCode, 
                                                ptrBuffer );
        if(FeatureManager::FeatureSupported(KFeatureIdSapTerminalControlFw ))
        {
        
            {                                                      
            // Check the code change policy from the SCP server.
            TInt allowChange = 1;
            
            RSCPClient scpClient;
            TInt ret = scpClient.Connect();
            if ( ret == KErrNone )
                {                                
                TBuf<KSCPMaxIntLength> policyBuf;
                policyBuf.Zero();
                if ( scpClient.GetParamValue( ESCPCodeChangePolicy, policyBuf ) == KErrNone )
                    {
                    TLex lex( policyBuf );
                    lex.Val( allowChange );
                    #if defined(_DEBUG)
                        RDebug::Print(_L("(GS)CGSSettListSimSecurityContainer::\
                            MakeCodeInUseItemL(): Policy retrieved: %d"), allowChange );
                    #endif //DEBUG                    
                    }
                else
                    {
                    #if defined(_DEBUG)
                       RDebug::Print(_L("(GS)CGSSettListSimSecurityContainer::\
                            MakeCodeInUseItemL(): ERROR: Failed to retrieve the policy value") );
                    #endif //DEBUG                                                      
                    }
                scpClient.Close();
                }
            else
                {
                #if defined(_DEBUG)
                    RDebug::Print(_L("(GS)CGSSettListSimSecurityContainer::MakeCodeInUseItemL(): \
                        ERROR: Failed to connect to SCP") );
                #endif //DEBUG                                  
                }
            
            if ( allowChange == 1 )
                {            
                #if defined(_DEBUG)
                    RDebug::Print(_L("(GS)CGSSettListSimSecurityContainer::MakeCodeInUseItemL(): \
                        Code change allowed"));
                #endif //DEBUG                                        
        
                iListboxItemArray->SetItemVisibilityL( EGSSettIdSecurityCode, 
		        CGSListBoxItemTextArray::EVisible );		        				
                }
            else
                {
                #if defined(_DEBUG)
                    RDebug::Print(_L("(GS)CGSSettListSimSecurityContainer::MakeCodeInUseItemL():\
                        Code change disallowed"));
                #endif //DEBUG
                
                iListboxItemArray->SetItemVisibilityL( EGSSettIdSecurityCode, 
    		        CGSListBoxItemTextArray::EInvisible );
                }
            }
            
      }  
      else
      {      
        
            iListboxItemArray->SetItemVisibilityL( EGSSettIdSecurityCode, 
                               CGSListBoxItemTextArray::EVisible );
		                		        
	}
            break;
        case EGSSettIdPinCode:
        default:
            if( !iUpinCodeSupported || iUpinCodeRejected || !iUpinCodeActive )
                {
                ptrBuffer =  CodeItem;
                iListboxItemArray->SetDynamicTextL( EGSSettIdPinCode, 
                                                    ptrBuffer );
                iListboxItemArray->SetItemVisibilityL( EGSSettIdPinCode, 
                                   CGSListBoxItemTextArray::EVisible );
                }
            else
                {
                iListboxItemArray->SetItemVisibilityL( EGSSettIdPinCode, 
                                   CGSListBoxItemTextArray::EInvisible );
                }
            break;
        }
    CleanupStack::PopAndDestroy( dynamicText );
    
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::MakeAutolockItemL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::MakeAutolockItemL()
    {
    HBufC* dynamicText = HBufC::NewLC( KGSBufSize128 );
    TPtr ptrBuffer ( dynamicText->Des() );

    TInt period = iModel->AutoLockPeriod();
    
    switch ( period )
        {
        case 0:
            ptrBuffer = ( *iAutoLockItems )[ 0 ];
            break;
        case 1:
            {
            TBuf<KGSBufSize128> tempString;
            StringLoader::Format( tempString, 
                                ( ( *iAutoLockItems )[1] ),
                                  -1, // no index in the key string
                                  period );
            ptrBuffer = tempString;
            }
            break;
        default:
            {
            TBuf<KGSBufSize128> tempString;
            StringLoader::Format( tempString, 
                                ( ( *iAutoLockItems )[2] ),
                                  -1, // no index in the key string
                                  period );
            ptrBuffer = tempString;
            }
            break;
        }
    iListboxItemArray->SetDynamicTextL( EGSSettIdAutolock, ptrBuffer );
    CleanupStack::PopAndDestroy( dynamicText );
    iListboxItemArray->SetItemVisibilityL( EGSSettIdAutolock, 
                       CGSListBoxItemTextArray::EVisible );
    }

// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::MakeSimChangeItemL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::MakeSimChangeItemL()
    {
    HBufC* dynamicText = HBufC::NewLC( KGSBufSize128 );
    TPtr ptrBuffer ( dynamicText->Des() );
   
    if ( iSecurity->IsLockEnabledL( RMobilePhone::ELockPhoneToICC ) )
        {
        ptrBuffer = ( *iSecurityItems )[0];
        }
    else
        {
        ptrBuffer = ( *iSecurityItems )[1];
        }

    iListboxItemArray->SetDynamicTextL( EGSSettIdSimChange, ptrBuffer );
    CleanupStack::PopAndDestroy( dynamicText );
    iListboxItemArray->SetItemVisibilityL( EGSSettIdSimChange, 
                       CGSListBoxItemTextArray::EVisible );
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer:: MakePinRequestItemL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer:: MakePinRequestItemL()
    {
    HBufC* dynamicText = HBufC::NewLC( KGSBufSize128 );
    TPtr ptrBuffer ( dynamicText->Des() );

    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {
        iUpinCodeSupported = iSecurity->IsUpinSupportedL();
        iUpinCodeActive = iSecurity->IsUpinActive(); 
        iUpinCodeRejected = iSecurity->IsUpinBlocked();
      }
    else
        {
         iUpinCodeSupported = EFalse;
         iUpinCodeActive = EFalse; 
         iUpinCodeRejected = EFalse;       
        }

    if( !iUpinCodeSupported || !iUpinCodeActive || iUpinCodeRejected )
        {
        if ( iSecurity->IsLockEnabledL( RMobilePhone::ELockICC ) )
            {
            ptrBuffer = ( *iPinItems )[0];
            }
        else
            {
            ptrBuffer = ( *iPinItems )[1];
            }   

        iListboxItemArray->SetDynamicTextL( EGSSettIdPinRequest, ptrBuffer );
        iListboxItemArray->SetItemVisibilityL( EGSSettIdPinRequest, 
                           CGSListBoxItemTextArray::EVisible );
        }
    else
        {
        iListboxItemArray->SetItemVisibilityL( EGSSettIdPinRequest, 
                           CGSListBoxItemTextArray::EInvisible );
        }
    CleanupStack::PopAndDestroy( dynamicText );
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer:: MakeUpinRequestItemL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer:: MakeUpinRequestItemL()
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {   
        HBufC* dynamicText = HBufC::NewLC( KGSBufSize128 );
        TPtr ptrBuffer ( dynamicText->Des() );
    
        iUpinCodeSupported = iSecurity->IsUpinSupportedL();
        iUpinCodeActive = iSecurity->IsUpinActive();
        iUpinCodeRejected = iSecurity->IsUpinBlocked();
    
    
        if( iUpinCodeSupported && !iUpinCodeRejected && iUpinCodeActive )
            {
        
            if ( iSecurity->IsLockEnabledL( RMobilePhone::ELockUniversalPin ) )
                {
                ptrBuffer = ( *iPinItems )[0];
                }
            else
                {
                ptrBuffer = ( *iPinItems )[1];
                }
    
            iListboxItemArray->SetDynamicTextL( EGSSettIdUpinRequest, ptrBuffer );
            iListboxItemArray->SetItemVisibilityL( EGSSettIdUpinRequest, 
                               CGSListBoxItemTextArray::EVisible );
            }
        else
            {
            iListboxItemArray->SetItemVisibilityL( EGSSettIdUpinRequest,
                               CGSListBoxItemTextArray::EInvisible );
            }
        
        CleanupStack::PopAndDestroy( dynamicText );
      }
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer:: MakeCodeInUseItemL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer:: MakeCodeInUseItemL()
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {
        HBufC* dynamicText = HBufC::NewLC( KGSBufSize128 );
        TPtr ptrBuffer ( dynamicText->Des() );
        iUpinCodeSupported = iSecurity->IsUpinSupportedL();
        iUpinCodeRejected = iSecurity->IsUpinBlocked();
        if( iUpinCodeSupported && !iUpinCodeRejected )
            {
            if ( !iSecurity->IsUpinActive() )
                {
                ptrBuffer = ( *iCodeItems )[0];
                }
            else
                {
                ptrBuffer = ( *iCodeItems )[1];
                }
    
            iListboxItemArray->SetDynamicTextL( EGSSettIdCodeInUse, ptrBuffer );
            iListboxItemArray->SetItemVisibilityL( EGSSettIdCodeInUse, 
                               CGSListBoxItemTextArray::EVisible );
            }
        else
            {
    
             iListboxItemArray->SetItemVisibilityL( EGSSettIdCodeInUse, 
                                CGSListBoxItemTextArray::EInvisible );
            }
        
        CleanupStack::PopAndDestroy( dynamicText );
      }
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer:: MakeClosedUserGroupItemL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::MakeClosedUserGroupItemL()
    {
    HBufC* dynamicText = HBufC::NewLC( KGSBufSize128 );
    TPtr ptrBuffer ( dynamicText->Des() );

    TInt cugMode = 0;
    TInt cugDefault;
    TInt err = KErrNone;

    RSSSettings cugSettings;
    User::LeaveIfError( cugSettings.Open() );

    err = cugSettings.Get( ESSSettingsDefaultCug, cugDefault );
    User::LeaveIfError( err );

    if ( cugSettings.Get( ESSSettingsCug , cugMode ) != KErrNone )
        {// getting mode was not succesful
        cugMode = cugDefault;
        }
    
    TBuf<KMaxStringLength> string;

    if ( cugMode == cugDefault )
        {
        StringLoader::Load( string, R_CUG_NETWORK_DEFAULT );
        ptrBuffer = string;
        }
    else
        {
        switch ( cugMode )
            {
            case ESSSettingsCugSuppress:
                StringLoader::Load( string, R_CUG_OFF );
                ptrBuffer = string;    
                break;
            default:
                StringLoader::Load( string, R_CUG_ON );
                ptrBuffer = string;    
                break;
            }
        }
    
    cugSettings.Close();
    
    iListboxItemArray->SetDynamicTextL( EGSSettIdClosedUserGroup, ptrBuffer );
    iListboxItemArray->SetItemVisibilityL( EGSSettIdClosedUserGroup, 
                       CGSListBoxItemTextArray::EVisible );
    CleanupStack::PopAndDestroy( dynamicText );

    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer:: MakeSatOperationsItemL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::MakeSatOperationsItemL()
    {
    HBufC* dynamicText = HBufC::NewLC( KGSBufSize128 );
    TPtr ptrBuffer ( dynamicText->Des() );

    TInt satOperation = iModel->SatOperations();
        
    if ( satOperation == 1 )
        {
        ptrBuffer = ( *iSatItems )[1];
        }
    else
        {
        ptrBuffer = ( *iSatItems )[0];
        }

    iListboxItemArray->SetDynamicTextL( EGSSettIdSatOperations, ptrBuffer );
    iListboxItemArray->SetItemVisibilityL( EGSSettIdSatOperations, 
                       CGSListBoxItemTextArray::EVisible );
    CleanupStack::PopAndDestroy( dynamicText );

    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::CurrentFeatureId()
//  
// ---------------------------------------------------------------------------
//
TInt CGSSimSecPluginContainer::CurrentFeatureId() const
    {
    return iListboxItemArray->CurrentFeature( );
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::GetHelpContext() const
// Gets Help 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidGS;
    aContext.iContext = KSET_HLP_SECURITY_DEVICE_SIM;
    }


// ---------------------------------------------------------------------------
// CGSSettListIdleContainer::RecreatePinItemsL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::RecreatePinItemsL()
    {
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    if(wcdmaSupported || upinSupported)
      {
        //check if UPIN code is supported and not rejected
        if( iSecurity->IsUpinSupportedL() )
            {
            iUpinCodeSupported = ETrue;
    
            //check if UPIN code is active
            if( iSecurity->IsUpinActive() )
                {
                iUpinCodeActive = ETrue;
                }
            else
                {
                iUpinCodeActive = EFalse;
                }
            
            //Check whether UPIN is blocked
    
            if( !iSecurity->IsUpinBlocked() )
                {
                iUpinCodeRejected = EFalse;
                }
            else
                {
                iUpinCodeRejected = ETrue;
                }
        }
        else
            {
            iUpinCodeSupported = EFalse;
            iUpinCodeActive = EFalse;
            }
    
        MakeCodeInUseItemL();
        MakeUpinRequestItemL();
        MakeCodeItemL( EGSSettIdUpinCode );
        MakePinRequestItemL();
        MakeCodeItemL( EGSSettIdPinCode );
      }
    }


// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::HandleResourceChangeL()
// 
// Updates view layout 
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::HandleResourceChangeL( TInt aType )
    {
    if( aType == KAknsMessageSkinChange ||
        aType == KEikDynamicLayoutVariantSwitch )
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, 
                                           mainPaneRect );
        SetRect( mainPaneRect );
        DrawNow();
        }
    }
    
    

// ---------------------------------------------------------------------------
// CGSSimSecPluginContainer::MakeRemoteLockItemL()
// 
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPluginContainer::MakeRemoteLockItemL()
    {
    #ifdef RD_REMOTELOCK
    #ifdef _DEBUG
    RDebug::Print( _L( "(SECUI)CGSSimSecPluginContainer::MakeRemoteLockItemL() - Enter" ) );
    #endif // DEBUG

    HBufC* statusItemText = HBufC::NewLC( KGSBufSize128 );
    TPtr ptrRemoteLockStatus( statusItemText->Des() );
    TBool remoteLockStatus( EFalse );
    CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewLC();

    if ( remoteLockSettings->GetEnabled( remoteLockStatus ) )
        {
        if ( remoteLockStatus )
            {
            // Remote lock enabled
            ptrRemoteLockStatus = (*iRemoteLockItems)[0];
            }
        else
            {
            // Remote lock disabled
            ptrRemoteLockStatus = (*iRemoteLockItems)[1];
            }
        }
    else
        {
        // Failed to get remote lock status
        // Undefined situation. Display empty item
        ptrRemoteLockStatus = KRemoteLockEmptyItem;
        }

    CleanupStack::PopAndDestroy( remoteLockSettings );
    remoteLockSettings = NULL;

    // Remote Lock item
	iListboxItemArray->SetDynamicTextL( EGSSettIdRemoteLock, ptrRemoteLockStatus );
    CleanupStack::PopAndDestroy( statusItemText );
	iListboxItemArray->SetItemVisibilityL( EGSSettIdRemoteLock, CGSListBoxItemTextArray::EVisible );

    #ifdef _DEBUG
    RDebug::Print( _L( "(SECUI)CGSSimSecPluginContainer::MakeRemoteLockItemL() - Exit" ) );
    #endif // DEBUG
	#endif // RD_REMOTELOCK
    return;
    }
    
    
//End of File
