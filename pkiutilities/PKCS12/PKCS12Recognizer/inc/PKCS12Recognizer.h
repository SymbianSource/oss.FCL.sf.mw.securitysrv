/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Header of recognizer capable of recognizing mime type
*                application/x-pkcs12
*
*/



#ifndef PKCS12RECOGNIZER_H
#define PKCS12RECOGNIZER_H

//  INCLUDES
#include <apmrec.h>

// FORWARD DECLARATION
class MPKCS12;

// CLASS DECLARATION

/**
*  CPKCS12Recognizer class.
*  Recognizes mime-types
*  application/x-pkcs12
*/
class CPKCS12Recognizer : public CApaDataRecognizerType
    {
    public: 
        CPKCS12Recognizer();
        ~CPKCS12Recognizer();
    public: // from CApaDataRecognizerType        
        TUint PreferredBufSize();
        TDataType SupportedDataTypeL(TInt aIndex) const;
        
        //for ECOM
        static CApaDataRecognizerType* CreateRecognizerL();
    
    private: // from CApaDataRecognizerType
        void DoRecognizeL(const TDesC& aName, const TDesC8& aBuffer);
        
    private:
        MPKCS12* iPKCS12;   
    };

#endif // PKCS12RECOGNIZER_H

// End of File
