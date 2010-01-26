/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   class CAkaIsaInterface definition
*
*/


#ifndef _AKAISAINTERFACE_H_
#define _AKAISAINTERFACE_H_

#include "gbauicc.h"

// INCLUDES
#include <e32base.h>
#include <rmmcustomapi.h>

//Constants
const TUint8 KAKA_RES_LENGTH =  16;
const TUint8 KAKA_IK_LENGTH  = 16;
const TUint8 KAKA_CK_LENGTH  = 16;
const TUint8 KAKA_KEY_LENGTH = 16;
const TUint8 KAKA_RAND_LENGTH = 16;
const TUint8 KRESPONSE_2G_LENGTH = 16;

#define GBA_UICC_INTERFACE_IMPLE 0x20029F0F
const TUid KGBAUICCImpleUID = { GBA_UICC_INTERFACE_IMPLE };

// CLASS DECLARATION
class CAkaIsaInterface : public CBase, public MUICCInterface 
    {
    public:
        static MUICCInterface* NewL(); 
        virtual ~CAkaIsaInterface();
        // Inherited from CUICCInterface  
        void QueryIdentityL(TDes8& aIdentity);
        void QueryHomeNetworkDnL(TDes8& aHNDN);
        TBool QueryAuthenticationL(const TDesC8& aNonce, TDes8& aResponse, TDes8& aResync );
        void QueryKeyMaterialL(const TDesC8& aKeyMaterial, const TDesC8& aRand, const TDesC8& aIMPI, const TDesC8& aUTF8_NAF_ID, TDes8& aDerivedKey ); 
        TBool QueryKs(TDes8& aKS);
        // Check the GBA_U availability from smart card apps
        void QueryGBAUAvailabilityL( TBool& aGBAAvail );
        // Save B-TID and key lifttime from BSF to smart card 
        TInt UpdateGBADataL( const TDesC8& aBTID, const TDesC8& aLifetime );  
        TBool InterfaceIs2G(); 
        void Release(); 
        void NotifyCardChangeL();
        void QueryCardInterfaceL();
    
    private:
    
        CAkaIsaInterface();
        void ConstructL();
		//run aka
        TBool QueryAuthenticationGBAUL(const TDesC8& aNonce, TDes8& aResponse, TDes8& aResync );
        //run aka
        TBool QueryAuthentication3GL(const TDesC8& aNonce, TDes8& aResponse, TDes8& aResync );
        //run simulated aka for 2G
        void QueryAuthentication2GL( const TDesC8& aNonce, TDes8& aResponse ); 
    
    private:
        enum TCardInterface
           {
           ENoInterface = 0,
           E2GInterface,	
           E3GInterface,	// UICC with USIM not supporting Ks_NAF derivation
           EGBAUInterface	// UICC with USIM application supporting Ks_NAF derivation.
           };
        
    private:
        TBuf8<KAKA_RES_LENGTH>  iResBuf;
        TBuf8<KAKA_IK_LENGTH>   iIKBuf;
        TBuf8<KAKA_CK_LENGTH>   iCKBuf;
        TCardInterface          iCardInterface;
        RTelServer              iEtelServer;
        RMobilePhone            iPhone;
        RMmCustomAPI            iCustomAPI;
        
        }; 

#endif // _AKAISAINTERFACE_H_

//EOF
