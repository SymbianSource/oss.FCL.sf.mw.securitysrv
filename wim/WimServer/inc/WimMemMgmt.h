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
* Description:  WIM memomy management
*
*/



#ifndef CWIMMEMMGMT_H
#define CWIMMEMMGMT_H

//  INCLUDES
#include "Wimi.h"            //WIMI definitions
#include <e32base.h>


// DATA TYPES
enum TWimRefType
    {
    EWimiRefpt,
    EWimiRefListt,
    };

struct TWimiAllocRef
    {
    union 
        {
        WIMI_Ref_pt ref;
        WIMI_RefList_t refLst;
        };
    
    TWimRefType refType;
    };


// CLASS DECLARATION

/**
*  WIM memomy management.
*  
*  @since Series60 2.1
*/
class CWimMemMgmt : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimMemMgmt* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimMemMgmt();

    public: // New functions
        
        /**
        * Sets active memory reference to iWimRef member variable.
        * @param aRef active reference.
        */
        void SetWIMRef( WIMI_Ref_pt aRef );

        /**
        * Retrieves active reference.
        * @return Active reference.
        */
        WIMI_Ref_pt WimRef();

        /**
        * Creates and append a new WIMI_Alloc_Ref item to iRefs array.
        * @param aLst reference list where to add the new item.
        */
        void AppendWIMRefLstL( WIMI_RefList_t aLst );

        /**
        * Creates and appends a new WIMI_Alloc_Ref item to iRefs array.
        * @param aRef Reference to be added.
        */
        void AppendWIMRefL( WIMI_Ref_pt aRef );        
        
        /**
        * Removes reference from the list and releases allocated memory.
        * @param aRef Reference to be freed.
        */
        void FreeRef( WIMI_Ref_pt );

        /**
        * Frees the reference(s) pointed by a client.
        * @param aMessage Encapsulates a client request.
        */
        void FreeWIMRefs( const RMessage2& aMessage );

        /**
        * Frees the list of reference(s) pointed by a client.
        * @param aMessage Encapsulates a client request.
        */
        void FreeRefLst( const RMessage2& aMessage );

        /**
        * Checks if the reference is valid
        * @param aRef Reference to be checked.
        * @return TBool ETrue if aRef is valid.
        */
        TBool ValidateWIMRefL( WIMI_Ref_pt aRef );


    private:

        CWimMemMgmt();
        void ConstructL();
        void CleanUp();

    private:    // Data
        RPointerArray<TWimiAllocRef> iRefs;
        WIMI_Ref_pt iWimRef;
    };

#endif      // CWIMMEMMGMT_H
            
// End of File
