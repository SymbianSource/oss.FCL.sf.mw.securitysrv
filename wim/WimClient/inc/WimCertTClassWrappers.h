/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  These wrappers are used when we need to fetch/insert 
*               certificate data asyncronously.
*
*/


#ifndef WIMCERTCLASSWRAPPERS_H
#define WIMCERTCLASSWRAPPERS_H

//  INCLUDES
#include <e32base.h>

// CLASS DECLARATION

/**
*  Class which wraps a TPckg -buffer. 
*
*  @lib WimClient
*  @since Series60 2.1
*/
template <class T>
class CWimCertPckg : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * C++ constructor.
        */
        inline CWimCertPckg( const T& aRef );
        
        /**
        * Destructor.
        */
        inline virtual ~CWimCertPckg();

    public: // New functions
        
        /**
        * @return TPckg<T>* Pointer to TPckg<T> typed member
        */
        inline TPckg<T>* Pckg();

    private:    // Data
        // Wrapped TPckg<T>
        TPckg<T>    iPckg;
    };


/**
*  Class which wraps a TPckgBuf -buffer. 
*
*  @WimClient
*  @since Series60 2.1
*/
template <class T>
class CWimCertPckgBuf : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * C++ default constructor.
        */
        inline CWimCertPckgBuf();

        /**
        * C++ constructor.
        */
        inline CWimCertPckgBuf( const T& aRef );
        
        /**
        * Destructor.
        */
        inline virtual ~CWimCertPckgBuf();


    public: // New functions
        
        /**
        * @return TPckgBuf<T>* Pointer to TPckgBuf<T> typed member
        */
        inline TPckgBuf<T>* PckgBuf();

    private:    // Data
        // Wrapped TPckgBuf<T>
        TPckgBuf<T>    iPckgBuf;
        
    };

#include "WimCertTClassWrappers.inl"

#endif //WIMICERTCLASSWRAPPERS_H         
            

