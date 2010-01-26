/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
*
*/
#ifndef PK5RECOGNIZER_H_
#define PK5RECOGNIZER_H_

// INCLUDES
#include <apmrec.h>

// CLASS DECLARATION
/**
*  Mime recognizer for "application/pk5".
*  
*/
const TInt KPk5RecognizerImplementationUid = 0x2000FD89;
class CPk5Recognizer : public CApaDataRecognizerType
    {
    public: // Constructor

        /**
        * Constructor.
        */
        CPk5Recognizer();

		~CPk5Recognizer();
		static CPk5Recognizer* NewL();
		
    public:  // Functions from base classes

        /**
        * From CApaDataRecognizerType.
        * Preferred byte size of sample content.
        */
        TUint PreferredBufSize();

        /**
        * From CApaDataRecognizerType.
        * Return mime type which this class supports to recognize.
        *
        * @param aIndex index.
        * @return always text/plain.
        */
        TDataType SupportedDataTypeL(TInt aIndex) const;
        /**
        ** From CApaDataRecognizerType.
        */
        
    	static CApaDataRecognizerType* CreateRecognizerL();
    		
    private:  // Functions from base classes

        /*
        // Two phase contructor
        //
        */
        void ConstructL();
        /**
        * From CApaDataRecognizerType.
        * This is a main part of recognization.
        *
        * @param aName file name.
        * @param aBuffer sample content.
        */
        void DoRecognizeL(const TDesC& aName, const TDesC8& aBuffer);

    };

#endif /*PK5RECOGNIZER_H_*/
