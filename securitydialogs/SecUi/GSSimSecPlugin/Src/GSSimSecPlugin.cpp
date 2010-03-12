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
* Description:  View for Device & SIM Security sub-folder
*
*/


// INCLUDE FILES
#include <coeaui.h>
#include <hlplch.h>             // For HlpLauncher
#include <bautils.h>
#include <gulicon.h>
#include <eikfrlbd.h>
#include <eiktxlbx.h>
#include <aknradiobuttonsettingpage.h>
#include <aknPopup.h>
#include <aknlists.h>
#include <RSSSettings.h>
#include <AknQueryDialog.h>
#include <aknnotedialog.h>
#include <aknViewAppUi.h>
#include <featmgr.h>
#include <StringLoader.h>
#include <secui.h>
#include <secuisecuritysettings.h>
#include <BTSapDomainPSKeys.h>
#include <e32property.h>

#include <GSSimSecPluginRsc.rsg>
#include <gsprivatepluginproviderids.h>
#include <gsmainview.h>
#include <gsbasecontainer.h>

#include "GSSimSecPlugin.h"
#include "GSSimSecPluginContainer.h"
#include "GSSimSecPlugin.hrh"


#ifdef RD_REMOTELOCK
#include    <RemoteLockSettings.h>  
#endif // RD_REMOTELOCK


// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS
const TInt KEmptyCugIndex( -100000000 );
_LIT( KGSSimSecPluginResourceFileName, "z:GSSimSecPluginRsc.rsc" );

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

/**
* CCugQuery
* It defines CCugQuery used in closed user group settings
*/
class CCugQuery
    : public CAknNumberQueryDialog
    {
    public: // Constructors and destructors
        /**
        * C++ constructor.
        */
        CCugQuery( TInt& aNumber,const TTone aTone = ENoTone );
    protected: // From base classes
        /**
        * From CCAknNumberQueryDialog Left softkey is allways OK.
        */
        void  UpdateLeftSoftKeyL();
    };

// ============================= LOCAL FUNCTIONS ==============================

// ========================= MEMBER FUNCTIONS =================================

// ---------------------------------------------------------------------------
// GSSimSecPlugin::NewL()
// 
// ---------------------------------------------------------------------------
CGSSimSecPlugin* CGSSimSecPlugin::NewL( TAny* /*aInitParams*/ )
    {
    CGSSimSecPlugin* self = new( ELeave ) CGSSimSecPlugin();
    
    CleanupStack::PushL( self );
    self->ConstructL();    
    CleanupStack::Pop( self );

    return self;
    }


// ---------------------------------------------------------------------------
// GSSimSecPlugin::CGSSimSecPlugin()
// 
// ---------------------------------------------------------------------------
CGSSimSecPlugin::CGSSimSecPlugin()
    : iResourceLoader( *iCoeEnv )
    {    
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::ConstructL()
// 
// Symbian OS two-phased constructor
// ---------------------------------------------------------------------------
void CGSSimSecPlugin::ConstructL()
    {
    FeatureManager::InitializeLibL();

    iModel = CGSSimSecPluginModel::NewL();
    //PS listener initialization
    iBtSapListener = CGSPubSubsListener::NewL( 
                     KPSUidBluetoothSapConnectionState,
                     KBTSapConnectionState, this );

    iSecurity= CSecuritySettings::NewL();    
    if(!FeatureManager::FeatureSupported( KFeatureIdFfNoCugSupport ))
    {    
    	User::LeaveIfError( iCugSettings.Open() );
		}
    // Find the resource file
    TParse parse;
    parse.Set( KGSSimSecPluginResourceFileName, 
               &KDC_RESOURCE_FILES_DIR, NULL );
    TFileName fileName( parse.FullName() );
    
    // Get language of resource file
    BaflUtils::NearestLanguageFile( iCoeEnv->FsSession(), fileName );

    // Open resource file
    iResourceLoader.OpenL( fileName );
    
    BaseConstructL( R_GS_SIM_SECURITY_VIEW );

    // secui resource file
    TSecUi::InitializeLibL();
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::~CGSSettListSecurityView()
// 
// 
// ---------------------------------------------------------------------------
CGSSimSecPlugin::~CGSSimSecPlugin()
    {
    FeatureManager::UnInitializeLib();
    // close resource loader
    iResourceLoader.Close();
	if(!FeatureManager::FeatureSupported( KFeatureIdFfNoCugSupport ))
    { 
    iCugSettings.Close();
  	}
    if ( iSecurity )
        {
        delete iSecurity;
        }

    if ( iModel )
        {
        delete iModel;
        }
    delete iBtSapListener;

    TSecUi::UnInitializeLib();  
    }


// ---------------------------------------------------------------------------
// TUid CGSSettSimListSecurityView::Id()
// 
// 
// ---------------------------------------------------------------------------
TUid CGSSimSecPlugin::Id() const
    {
    return KGSSimSecPluginUid;
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::HandleCommandL()
// 
// 
// ---------------------------------------------------------------------------
void CGSSimSecPlugin::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EGSCmdAppChange:
        case EAknSoftkeyChange:
            {           
            const TInt currentFeatureId = Container()->CurrentFeatureId();

            if ( currentFeatureId != EGSSettIdSatOperations )
                {
                HandleListBoxSelectionL();
                }
            else
                {
                if(aCommand == EGSCmdAppChange)
                    SetSatWithSettingPageL();
                else //user pressed MSK, don't show setting page.
                    SetSatOperationsL();
                }
            }
            break;
        case EAknSoftkeyBack:
            iAppUi->ActivateLocalViewL( KGSSecurityPluginUid );  
            break;
        case EAknCmdHelp:
            {
            if( FeatureManager::FeatureSupported( KFeatureIdHelp ) )
                {
                HlpLauncher::LaunchHelpApplicationL(
                    iEikonEnv->WsSession(), iAppUi->AppHelpContextL() );
                }
            }
            break;
        default:
            iAppUi->HandleCommandL( aCommand );
            break;
        }
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::UpdateListBoxL
// 
// Update the current item in the listbox.
// ---------------------------------------------------------------------------
void CGSSimSecPlugin::UpdateListBoxL( TInt aItemId )
    {
    Container()->UpdateListBoxL( aItemId );
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::DoActivateL(...)
// 
// 
// ---------------------------------------------------------------------------
void CGSSimSecPlugin::DoActivateL( const TVwsViewId& aPrevViewId, 
                                   TUid aCustomMessageId, 
                                   const TDesC8& aCustomMessage )
    {
    CGSBaseView::DoActivateL( aPrevViewId, aCustomMessageId, aCustomMessage );    
    }


// ----------------------------------------------------------------------------
// CGSSimSecPlugin::Container
// 
// Return handle to container class.
// ----------------------------------------------------------------------------
//
CGSSimSecPluginContainer* CGSSimSecPlugin::Container()
    {
    return static_cast<CGSSimSecPluginContainer*>( iContainer );
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::NewContainerL()
// 
// Creates new iContainer.
// ---------------------------------------------------------------------------
//
void CGSSimSecPlugin::NewContainerL()
    {
    iContainer = new( ELeave ) CGSSimSecPluginContainer( iModel );
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::HandleListBoxSelectionL()
// 
// 
// ---------------------------------------------------------------------------
void CGSSimSecPlugin::HandleListBoxSelectionL() 
    {
    const TInt currentFeatureId = Container()->CurrentFeatureId();
    TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
    TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
    switch ( currentFeatureId )
        {
        case EGSSettIdCodeInUse:
            if(wcdmaSupported || upinSupported)
                {
	                if(iSecurity->SwitchPinCodesL())
	                {
	                	UpdateListBoxL( currentFeatureId );
	              	}
                }       
            break;
        case EGSSettIdUpinRequest:
            if(wcdmaSupported || upinSupported)
                 {
                    if(iSecurity->ChangeUPinRequestL())
                    {            
                    	UpdateListBoxL( currentFeatureId );
                    }
                 }
            break;
        case EGSSettIdUpinCode:
            if(wcdmaSupported || upinSupported)
              {
                  iSecurity->ChangeUPinL();
              }
            break;
        case EGSSettIdPinRequest: 
        		if(iSecurity->ChangePinRequestL())
            {               
            	UpdateListBoxL( currentFeatureId );
            }
            break;
        case EGSSettIdPinCode:            
            iSecurity->ChangePinL();
            break;
        case EGSSettIdPin2Code:
            iSecurity->ChangePin2L();
            break;
        case EGSSettIdAutolock:        
            SetAutolockTimeL( iSecurity->ChangeAutoLockPeriodL( 
                                         iModel->AutoLockPeriod() ) );
            break;
        case EGSSettIdSecurityCode:
            iSecurity->ChangeSecCodeL();
            break;
        case EGSSettIdSimChange:    
            if(iSecurity->ChangeSimSecurityL())
            {
            	UpdateListBoxL( currentFeatureId );
            }
            break; 
            
#ifdef RD_REMOTELOCK
        case EGSSettIdRemoteLock:
            SetRemoteLockStatusL( iModel->AutoLockPeriod() );
            break;
            
#endif                
        case EGSSettIdClosedUserGroup:
        	if(!FeatureManager::FeatureSupported( KFeatureIdFfNoCugSupport ))
    		{        
	            CugModeL();
	            UpdateListBoxL( currentFeatureId );
	          }
            break;
        case EGSSettIdSatOperations:
            SetSatOperationsL();
            break;
        default:
            break;
        }
    
    }

// ---------------------------------------------------------------------------
// CGSSimSecPlugin::SetAutolockTimeL()
// 
// 
// ---------------------------------------------------------------------------
void CGSSimSecPlugin::SetAutolockTimeL( TInt aPeriod )
    { 
    iModel->SetAutoLockPeriod( aPeriod );
    UpdateListBoxL( EGSSettIdAutolock );
    }

// ---------------------------------------------------------------------------
// CGSSimSecPlugin::SetSatOperationsL()
// 
// 
// ---------------------------------------------------------------------------
void CGSSimSecPlugin::SetSatOperationsL()
    {
    TInt i = iModel->SatOperations();
    
    if ( i == 0 )
        {
        iModel->SetSatOperations( 1 );
        }
    else
        {
        iModel->SetSatOperations( 0 );
        }
    UpdateListBoxL( EGSSettIdSatOperations );
    }

// ---------------------------------------------------------------------------
// CGSSimSecPlugin::SetSatOperationsL()
// 
// 
// ---------------------------------------------------------------------------
void CGSSimSecPlugin::SetSatWithSettingPageL()
    {
    TInt currentItem = iModel->SatOperations();

    CDesCArrayFlat* items =  iCoeEnv->ReadDesC16ArrayResourceL( R_SAT_ARRAY );
    CleanupStack::PushL( items );

    CAknRadioButtonSettingPage* page = 
        new( ELeave ) CAknRadioButtonSettingPage( R_SAT_SETTING_PAGE, 
                                                  currentItem, items );

    if ( page->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ) )            
        {    
        iModel->SetSatOperations( currentItem );
        }
    
    CleanupStack::PopAndDestroy( items );

    UpdateListBoxL( EGSSettIdSatOperations );
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::CugModeL()
// 
// 
// ---------------------------------------------------------------------------
void CGSSimSecPlugin::CugModeL()
    {    
    TInt cugIndex = 0;
    TInt currentItem = 0;
    TInt cugMode = 0;
    TInt cugDefault;
    TInt err = KErrNone;

    err = iCugSettings.Get( ESSSettingsDefaultCug, cugDefault );
    User::LeaveIfError( err );

    
    if ( iCugSettings.Get( ESSSettingsCug, cugMode ) != KErrNone )
        {// getting mode was not succesful
        cugMode = cugDefault;
        }

    if ( cugMode == cugDefault )
        {
        currentItem = EGSCugNetworkDefault;
        if ( iCugSettings.PreviousCugValue( cugIndex ) != KErrNone )        
            {
            cugIndex = 0;
            }
        }
    else
        {
        switch ( cugMode )
            {
            case ESSSettingsCugSuppress:
                currentItem = EGSCugOff;
                if ( iCugSettings.PreviousCugValue( cugIndex ) != KErrNone )
                    {
                    cugIndex = 0;
                    }
                break;
            default:
                currentItem = EGSCugOn;
                cugIndex = cugMode;
                break;
            }
        }


    CDesCArrayFlat* items = iCoeEnv->ReadDesC16ArrayResourceL( R_CUG_LBX );
    CleanupStack::PushL( items );

    CAknRadioButtonSettingPage* page = 
        new( ELeave ) CAknRadioButtonSettingPage( R_CUG_SETTING_PAGE, 
                                                  currentItem, items );

    // call the appropriate cug- methods here
    if ( page->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ) )    
        {    
        
        switch ( currentItem )
            {
            case EGSCugNetworkDefault://cug mode set default, 
                User::LeaveIfError( iCugSettings.Set( ESSSettingsCug, 
                                                      cugDefault ) );
                break;
            case EGSCugOn: //1, index is queried, if query ok then
                // set the index and mode
                if ( CugIndexQueryL( cugIndex, cugDefault )  )
                    {
                    if ( iCugSettings.IsValueValidCugIndex( cugIndex ) )
                        {
                        User::LeaveIfError( iCugSettings.Set( ESSSettingsCug, 
                                                              cugIndex ) );
                        }
                    else
                        {
                        User::LeaveIfError( iCugSettings.Set( ESSSettingsCug, 
                                                              cugDefault ) );
                        User::LeaveIfError( iCugSettings.ResetPreviousCugValue() );
                        }
                    }
                break;
            case EGSCugOff://2, sets cug mode "inactive"
                User::LeaveIfError( iCugSettings.Set( ESSSettingsCug, 
                                    ESSSettingsCugSuppress ) );
                break;
            default:
                break;
            }
        }
    CleanupStack::PopAndDestroy( items );
    return;    
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::CugIndexQueryL()
// 
// 
// ---------------------------------------------------------------------------
TInt CGSSimSecPlugin::CugIndexQueryL( TInt& aCugIndex, TInt& aCugDefault )
    {    
    while ( ETrue )
        {
        CCugQuery* dlg = new( ELeave ) CCugQuery( aCugIndex,
                                       CAknQueryDialog::ENoTone );
    
        if ( dlg->ExecuteLD( R_CUG_INDEX ) )
            {
            // check content validity (?-32767), 
            // ui spec concerned only values above limit
            if( !iCugSettings.IsValueValidCugIndex( aCugIndex )
                && ( aCugIndex != KEmptyCugIndex )
                && ( aCugIndex != aCugDefault ) )
                {
                CAknNoteDialog* dlg = new ( ELeave ) 
                    CAknNoteDialog( CAknNoteDialog::EErrorTone, 
                                    CAknNoteDialog::ELongTimeout );
                dlg->ExecuteLD( R_CUG_INDEX_ERROR_NOTE );
                }
            else //index ok, break
                {
                break;
                }
            }
        else // query cancelled
            {
            return EFalse;
            }
        }

    return ETrue;    
    }


// ---------------------------------------------------------------------------
// CCugQuery::CCugQuery()
// 
// ---------------------------------------------------------------------------
//
CCugQuery::CCugQuery( TInt& aNumber, const TTone aTone )
    : CAknNumberQueryDialog( aNumber, aTone )
    {
    }


// ---------------------------------------------------------------------------
// CCugQuery::UpdateLeftSoftKeyL()
// Left softkey is allways visible
// ---------------------------------------------------------------------------
//
void CCugQuery::UpdateLeftSoftKeyL()
    {
    MakeLeftSoftkeyVisible( ETrue );
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::HandleResourceChangeL( TInt aType )
// Updates view layout
//  
// ---------------------------------------------------------------------------
//
void CGSSimSecPlugin::HandleResourceChangeL( TInt aType )
    {
    if( aType == KAknsMessageSkinChange ||
        aType == KEikDynamicLayoutVariantSwitch )
        {
        //iContainer->HandleResourceChangeL( aType );
        }
    }


// ---------------------------------------------------------------------------
// CGSSimSecPlugin::HandleNotifyPSL
//
// Handling PS keys change
// ---------------------------------------------------------------------------
//  
void CGSSimSecPlugin::HandleNotifyPSL( const TUid aUid, const TInt& aKey,
                                       const TRequestStatus& /* aStatus */ )
    {
    if ( aUid == KPSUidBluetoothSapConnectionState && 
         aKey == KBTSapConnectionState )
        {
        Visible();
        }
    }


// ========================= From CGSPluginInterface ==================

// ----------------------------------------------------------------------------
// CGSSimSecPlugin::GetCaption
// 
// Return application/view caption.
// ----------------------------------------------------------------------------
//
void CGSSimSecPlugin::GetCaptionL( TDes& aCaption ) const
    {
    // the resource file is already opened.
    HBufC* result = StringLoader::LoadL( R_GS_SIM_SECURITY_VIEW_CAPTION );
    
    aCaption.Copy( *result );
    delete result;
    }
    

// ----------------------------------------------------------------------------
// CGSSimSecPlugin::PluginProviderCategory
// 
// A means to identify the location of this plug-in in the framework.
// ----------------------------------------------------------------------------
//
TInt CGSSimSecPlugin::PluginProviderCategory() const
    {
    //To identify internal plug-ins.
    return KGSPluginProviderInternal;
    }
    

// ----------------------------------------------------------------------------
// CGSSimSecPlugin::Visible
// 
// Provides the visibility status of self to framework.
// ----------------------------------------------------------------------------
//    
TBool CGSSimSecPlugin::Visible() const
    {
    TInt btSapConnectionState;
    TBool visible = ETrue;

    iBtSapListener->Get( btSapConnectionState );

    if ( btSapConnectionState == EBTSapConnected )
        {
        visible = EFalse;
        }
    
    return visible;
    }



// ---------------------------------------------------------------------------
// CGSSimSecPlugin::SetRemoteLockStatusL()
// 
// 
// ---------------------------------------------------------------------------
TInt CGSSimSecPlugin::SetRemoteLockStatusL( TInt aAutoLockPeriod )
    {
    #ifdef RD_REMOTELOCK
    #ifdef _DEBUG
    RDebug::Print( _L( "(GS)CGSSettListSimSecurityView::SetRemoteLockStatusL() - Enter" ) );
    #endif // DEBUG

    TInt retValue( KErrNone );
    TBool remoteLockStatus( EFalse );
    TBuf<KRLockMaxLockCodeLength> remoteLockCode;

    CRemoteLockSettings* remoteLockSettings = CRemoteLockSettings::NewLC();

    // Get the current remote lock status 
    if ( remoteLockSettings->GetEnabled( remoteLockStatus ) )
        {
        // Show remote lock setting page
        retValue = iSecurity->ChangeRemoteLockStatusL( remoteLockStatus, remoteLockCode, aAutoLockPeriod );

        if ( retValue == KErrNone )
            {
            if ( remoteLockStatus )
                {
                // Set remote lock status
                if ( remoteLockSettings->SetEnabledL( remoteLockCode ) )
                    {
                    // Remote lock status was set successfully
                    #ifdef _DEBUG
                    RDebug::Print( _L( "(GS)CGSSettListSimSecurityView::SetRemoteLockStatusL() - Remote lock enabled and code set" ) );
                    #endif // DEBUG
                    }
                else
                    {
                    // Failed to enabled remote lock 
                    retValue = KErrGeneral;

                    #ifdef _DEBUG
                    RDebug::Print( _L( "(GS)CGSSettListSimSecurityView::SetRemoteLockStatusL() - remoteLockSettings->SetEnabledL failed" ) );
                    #endif // DEBUG
                    }
                }
            else
                {
                // Disable remote lock
                if ( remoteLockSettings->SetDisabled() )
                    {
                    // Remote lock disabled
                    #ifdef _DEBUG
                    RDebug::Print( _L( "(GS)CGSSettListSimSecurityView::SetRemoteLockStatusL() - Remote lock disabled" ) );
                    #endif // DEBUG
                    }
                else
                    {
                    // Failed to disable remote lock
                    retValue = KErrGeneral;

                    #ifdef _DEBUG
                    RDebug::Print( _L( "(GS)CGSSettListSimSecurityView::SetRemoteLockStatusL() - remoteLockSettings->SetDisabled failed" ) );
                    #endif // DEBUG
                    }
                }
            }
        else
            {
            // User interaction (setting page) failed for some reason
            #ifdef _DEBUG
            RDebug::Print( _L( "(GS)CGSSettListSimSecurityView::SetRemoteLockStatusL() - iSecurity->ChangeRemoteLockStatusL failed" ) );
            #endif // DEBUG
            }
        }
    else
        {
        // Failed to retreive the current remote lock status
        retValue = KErrGeneral;

        #ifdef _DEBUG
        RDebug::Print( _L( "(GS)CGSSettListSimSecurityView::SetRemoteLockStatusL() - remoteLockSettings->GetEnabled failed" ) );
        #endif // DEBUG
        }

    CleanupStack::PopAndDestroy( remoteLockSettings );
    remoteLockSettings = NULL;

    UpdateListBoxL( EGSSettIdRemoteLock );

    #ifdef _DEBUG
    RDebug::Print( _L( "(GS)CGSSettListSimSecurityView::SetRemoteLockStatusL() - Exit" ) );
    #endif // DEBUG
	return retValue;
	#else //!RD_REMOTELOCK 
    return KErrNotSupported;
    #endif // RD_REMOTELOCK  
    }
    
    
// ----------------------------------------------------------------------------
// CGSSimSecPlugin::DynInitMenuPaneL()
// 
// Display the dynamic menu
// ----------------------------------------------------------------------------
void CGSSimSecPlugin::DynInitMenuPaneL( TInt aResourceId,
                                          CEikMenuPane* aMenuPane )
    {

	// show or hide the 'help' menu item when supported
    if( aResourceId == R_GS_SIM_SEC_MENU_ITEM_HELP )
        {
        User::LeaveIfNull( aMenuPane );    
        if ( FeatureManager::FeatureSupported( KFeatureIdHelp ) )
            {
            aMenuPane->SetItemDimmed( EAknCmdHelp, EFalse );
            }
        else
            {
            aMenuPane->SetItemDimmed( EAknCmdHelp, ETrue );
            }
        }

    } 
// End of File  
