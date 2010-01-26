/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   The header file of DevCertKeyEncryptor
*
*/



#ifndef __DEVCERTKEYENCRYPTOR_H__
#define __DEVCERTKEYENCRYPTOR_H__

#include <e32base.h>
#include <keyencryptor.h>

/**
 *  class CDevCertKeyEncryptor
 *
 *  Encrypt key before putting into keystore 
 *  by using key encryptor plugin
 *
 *  @lib
 *  @since S60 v3.2
 */
class CDevCertKeyEncryptor : public CBase
    {
    public:

      static CDevCertKeyEncryptor* NewL();
      
      static CDevCertKeyEncryptor* NewLC();
      
      virtual ~CDevCertKeyEncryptor();
      
      MKeyEncryptor* CreateImplementationL();
      
      TBool IsPluginExistL();
  
    private:
    
      CDevCertKeyEncryptor();
      
      void ConstructL();
    
    private:
      
      TUid iImplementationUID;
      
      TUid iDtorIDKey;
      
      MKeyEncryptor* iKeyEncryptor;
    };

#endif  //  __DEVCERTKEYENCRYPTOR_H__

//EOF

