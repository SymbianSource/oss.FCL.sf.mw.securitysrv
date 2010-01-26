/*
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Crypto Token Security Dialog Notifier
*
*/



// INCLUDE FILES
#include "CTSecurityDialogNotifier.h"
#include "CTSecurityDialogsAO.h" 
#include <eikenv.h>
#include <coemain.h>
#include <bautils.h>
#include <StringLoader.h>
#include <data_caging_path_literals.hrh>



// CONSTANTS
_LIT(KCTSecDlgDiskAndFile, "z:CTSecDlgs.rsc");
_LIT(KCTCertManUiResDiskAndFile, "z:CertManUi.rsc");


// ============================= LOCAL FUNCTIONS ===============================

GLDEF_C void Panic(TInt aPanicCode)
    {
	_LIT(KComponentName, "CTSecNotifier");
	User::Panic(KComponentName, aPanicCode);
    }


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::CCTSecurityDialogNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCTSecurityDialogNotifier::CCTSecurityDialogNotifier(): 
    iSaveReceipt(EFalse) 
    {    
    WIMSECURITYDIALOGS_CREATE
    WIMSECURITYDIALOGS_WRITE_TIMESTAMP( "CCTSecurityDialogNotifier::CCTSecurityDialogNotifier()" );
	iEikEnv = CEikonEnv::Static();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::~CCTSecurityDialogNotifier()
// -----------------------------------------------------------------------------
//
CCTSecurityDialogNotifier::~CCTSecurityDialogNotifier()
    {
    WIMSECURITYDIALOGS_DELETE
    Cancel();
	if ( iResourceFileOffset )
        {
		iEikEnv->DeleteResourceFile( iResourceFileOffset );
        }
    if ( iResourceFileOffset2 )
        {
		iEikEnv->DeleteResourceFile( iResourceFileOffset2 );
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCTSecurityDialogNotifier* CCTSecurityDialogNotifier::NewL()
    {
	CCTSecurityDialogNotifier* self = new (ELeave) CCTSecurityDialogNotifier;
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop(); // self
	return self;
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogNotifier::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::Release()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogNotifier::Release()
    {
    WIMSECURITYDIALOGS_WRITE_TIMESTAMP( "CCTSecurityDialogNotifier::Release" );
	delete this;
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::RegisterL()
// -----------------------------------------------------------------------------
//
CCTSecurityDialogNotifier::TNotifierInfo CCTSecurityDialogNotifier::RegisterL()
    {
	// Resource file loading
	RFs& fs = iEikEnv->FsSession();

	TFileName fileName;

	TParse parse;
    parse.Set(KCTSecDlgDiskAndFile, &KDC_RESOURCE_FILES_DIR, NULL);
	
	fileName = parse.FullName();

	BaflUtils::NearestLanguageFile( fs, fileName );
	iResourceFileOffset = iEikEnv->AddResourceFileL( fileName );
	
	parse.Set(KCTCertManUiResDiskAndFile, &KDC_RESOURCE_FILES_DIR, NULL); 
    fileName = parse.FullName();
    BaflUtils::NearestLanguageFile( fs, fileName );
    iResourceFileOffset2 = iEikEnv->AddResourceFileL( fileName );

	return Info();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::Info() 
// -----------------------------------------------------------------------------
//
CCTSecurityDialogNotifier::TNotifierInfo CCTSecurityDialogNotifier::Info() const
    {
	TNotifierInfo info;
	info.iUid = KUidSecurityDialogNotifier;
	info.iChannel = KUidSecurityDialogNotifier;
    info.iPriority = ENotifierPriorityAbsolute;
	return info;
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::StartL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogNotifier::StartL( const TDesC8& aBuffer, TInt aReplySlot,
        const RMessagePtr2& aMessage )
    {
    WIMSECURITYDIALOGS_WRITE( "CCTSecurityDialogNotifier::StartL" );

    TRAPD( err, DoStartL( aBuffer, aReplySlot, aMessage ) );
    if( err )
        {
        if( !iSecurityDialogAODeleted )
            {
            delete iSecurityDialogAO;
            iSecurityDialogAO = NULL;
            }
        if( !aMessage.IsNull() )
            {
            aMessage.Complete( err );
            }
        }
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::DoStartL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogNotifier::DoStartL( const TDesC8& aBuffer, TInt aReplySlot,
        const RMessagePtr2& aMessage )
    {
    // CCTSecurityDialogsAO sets the boolean iSecurityDialogAODeleted when it is
    // constructed and resets it when it is destructed, so iSecurityDialogAODeleted
    // tells whether the iSecurityDialogAO pointer is valid or not.
    iSecurityDialogAO = CCTSecurityDialogsAO::NewL( this, iSecurityDialogAODeleted );
    
    iSecurityDialogAO->StartLD( aBuffer, aReplySlot, aMessage );
    // StartLD() deletes the iSecurityDialogAO object when it is ready processing the
    // request. This may take some time, as CCTSecurityDialogsAO is active object, so
    // iSecurityDialogAO object exists when the above StartLD() returns here. It will
    // be deleted later when the operation completes. Started operation can be cancelled
    // before it is completed by deleting the iSecurityDialogAO object.
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::StartL()
// -----------------------------------------------------------------------------
//
TPtrC8 CCTSecurityDialogNotifier::StartL(const TDesC8& /*aBuffer*/)
    {
	return KNullDesC8();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::Cancel()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogNotifier::Cancel()
    {
    WIMSECURITYDIALOGS_WRITE( "CCTSecurityDialogNotifier::Cancel" );
    
    // If iSecurityDialogAO is not completed yet, we need to cancel it now.
    // We can simply delete iSecurityDialogAO to cancel pending actions.
    if( !iSecurityDialogAODeleted )
        {
        delete iSecurityDialogAO;
        }
    iSecurityDialogAO = NULL;
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::UpdateL()
// -----------------------------------------------------------------------------
//
TPtrC8 CCTSecurityDialogNotifier::UpdateL(const TDesC8& /*aBuffer*/)
    {
	return KNullDesC8();
    }
    
// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::LoadResourceStringLC()
// -----------------------------------------------------------------------------
//
HBufC* CCTSecurityDialogNotifier::LoadResourceStringLC( const TInt aType, const TDesC& aDynamicText, const TDesC& aDynamicText2 )
    {
    if ( aDynamicText2.Length() ) 
        {
        TDialogTypeItem item = GetDialogTypeItem( aType );
        HBufC* stringWithDynamics = StringLoader::LoadL(item.iResourceIdentifier, iEikEnv);
        CleanupStack::PushL(stringWithDynamics);
        
        if ( aDynamicText.Length() && aDynamicText2.Length() )
            {
            _LIT(dynamicTextIdentifier, "%0U");
            
            if (stringWithDynamics->Find(dynamicTextIdentifier) != KErrNotFound)
                {
                CleanupStack::PopAndDestroy(stringWithDynamics);
                
                CDesCArrayFlat* strings = new (ELeave) CDesCArrayFlat(2);
                CleanupStack::PushL(strings);
                      
                strings->AppendL( aDynamicText );
                strings->AppendL( aDynamicText2 );
                
                stringWithDynamics = 
                    StringLoader::LoadL(item.iResourceIdentifier, 
                        *strings, iEikEnv);
          
                CleanupStack::PopAndDestroy(strings);
      
                CleanupStack::PushL(stringWithDynamics);
                }
            else
                {
                HBufC* dynamicText = NULL;
                _LIT(dynamicTextIdentifier, "%U");
                
                if (stringWithDynamics->Find(dynamicTextIdentifier) != KErrNotFound)
                    {
                    CleanupStack::PopAndDestroy(stringWithDynamics); // stringWithDynamics, it wasn't final
                    TPtrC param; 
                    if (item.iDynamicResourceIdentifier)
                        {
                        dynamicText = StringLoader::LoadL( item.iDynamicResourceIdentifier, iEikEnv );
                        CleanupStack::PushL( dynamicText );
                        param.Set( dynamicText->Des() );
                        }
                    else
                        {
                        param.Set( aDynamicText );
                        }
                    stringWithDynamics = 
                        StringLoader::LoadL(item.iResourceIdentifier, 
                                             param, iEikEnv);
                    if (dynamicText)
                        {
                        CleanupStack::PopAndDestroy(dynamicText);
                        }
                    CleanupStack::PushL(stringWithDynamics);
                    }
                else
                    {
                    }
                     
                }
            }
        return stringWithDynamics;      
        }
    else
        {
        TDialogTypeItem item = GetDialogTypeItem( aType );
        HBufC* stringWithDynamics = StringLoader::LoadL(item.iResourceIdentifier, iEikEnv);
        CleanupStack::PushL(stringWithDynamics);
        HBufC* dynamicText = NULL;
    
        if (item.iDynamicResourceIdentifier || aDynamicText.Length())
            {
            _LIT(dynamicTextIdentifier, "%U");
            
            if (stringWithDynamics->Find(dynamicTextIdentifier) != KErrNotFound)
                {
                CleanupStack::PopAndDestroy(stringWithDynamics); // stringWithDynamics, it wasn't final
                TPtrC param; 
                if (item.iDynamicResourceIdentifier)
                    {
                    dynamicText = StringLoader::LoadL( item.iDynamicResourceIdentifier, iEikEnv );
                    CleanupStack::PushL( dynamicText );
                    param.Set( dynamicText->Des() );
                    }
                else
                    {
                    param.Set( aDynamicText );
                    }
          stringWithDynamics = 
                    StringLoader::LoadL(item.iResourceIdentifier, 
                                         param, iEikEnv);
                if (dynamicText)
                    {
                    CleanupStack::PopAndDestroy(dynamicText);
                    }
                CleanupStack::PushL(stringWithDynamics);
                }
            else
                {
                }
            }
            
          return stringWithDynamics;
          }
    
    }


// -----------------------------------------------------------------------------
// CCTSecurityDialogNotifier::GetDialogTypeItem()
// -----------------------------------------------------------------------------
//
TDialogTypeItem CCTSecurityDialogNotifier::GetDialogTypeItem(const TInt aType)
    {
    // If the code panics here, then there's a TDialogType without 
	// any TDialogTypeItem-> Check KDialogTypeToResourceMapper, and add to the mapper!


	// Find the corresponding resource to aType.
	// Algorithm: Best guess : Let's take KDialogTypeToResourceMapper[aType]
	// if its iDialogType field matches with aType -> HIT, OK
	// if not, then browse through the whole resource mapper array. If found, ok, 
	// but if not, we will have to panic, as it is a developer error (fatal).

	const TInt mapperCount = sizeof(KDialogTypeToResourceMapper) / sizeof(TDialogTypeItem);
	__ASSERT_ALWAYS(aType < mapperCount, Panic(KErrOverflow));

	TDialogTypeItem item = KDialogTypeToResourceMapper[aType];

	if (item.iDialogType != aType)
	    {   
		TBool found(EFalse);

		for (TInt i = 0; i < mapperCount; i++)
		    {
			item = KDialogTypeToResourceMapper[i];
			if (item.iDialogType == aType)
			    {
				found = ETrue;
				break;
                } // if
            } // for 
		__ASSERT_ALWAYS(found, Panic(KErrNotFound));
        } // if
	return item; 
    } 


