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


// -----------------------------------------------------------------------------
// Inline implementation of CWimCertPckgBuf<T> class
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// CWimCertPckg<T>::CWimCertPckg()
// C++ constructor.
// -----------------------------------------------------------------------------
template <class T>
inline CWimCertPckg<T>::CWimCertPckg( const T& aRef ) : iPckg( aRef )
    {
    }

// -----------------------------------------------------------------------------
// CWimCertPckg<T>::~CWimCertPckg()
// Destructor.
// -----------------------------------------------------------------------------
template <class T>
inline CWimCertPckg<T>::~CWimCertPckg()
    {
    }

// -----------------------------------------------------------------------------
// CWimCertPckgBuf<T>::Pckg()
// return TPckg<T>* Pointer to TPckg<T> typed member
// -----------------------------------------------------------------------------
template <class T>
inline TPckg<T>* CWimCertPckg<T>::Pckg()
    {
    return &iPckg; 
    }


// -----------------------------------------------------------------------------
// Inline implementation of CWimCertPckgBuf<T> class
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// CWimCertPckgBuf<T>::CWimCertPckgBuf()
// C++ default constructor.
// -----------------------------------------------------------------------------
template <class T>
inline CWimCertPckgBuf<T>::CWimCertPckgBuf() : iPckgBuf()
    {
    }

// -----------------------------------------------------------------------------
// CWimCertPckgBuf<T>::CWimCertPckgBuf( const T& aRef ) 
// C++ constructor.
// -----------------------------------------------------------------------------
template <class T>
inline CWimCertPckgBuf<T>::CWimCertPckgBuf( const T& aRef ) : iPckgBuf( aRef )
    {
    }


// -----------------------------------------------------------------------------
// CWimCertPckgBuf<T>::~CWimCertPckgBuf() 
// Destructor.
// -----------------------------------------------------------------------------
template <class T>
inline CWimCertPckgBuf<T>::~CWimCertPckgBuf()
    {
    }

// -----------------------------------------------------------------------------
// CWimCertPckgBuf<T>::PckgBuf()
// -----------------------------------------------------------------------------
template <class T>
inline TPckgBuf<T>* CWimCertPckgBuf<T>::PckgBuf()
    {
    return &iPckgBuf; 
    }




//  End of File  

