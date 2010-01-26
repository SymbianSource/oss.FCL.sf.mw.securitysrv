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
* Description:  
*
*/


#ifndef DEVENCSTARTERPROPERTYOBSERVER_H
#define DEVENCSTARTERPROPERTYOBSERVER_H

// INCLUDES
#include <e32base.h>
#include <e32property.h>

// CLASS DECLARATION

/**
 * MDevEncStarterPropertyChangeObserver
 * Abstract interface for handling property change events.
 **/

class MDevEncStarterPropertyChangeObserver
    {
    public:
        /**
         * This is a callback function which is called when a property value is changed.
         * @param aObserver a refference to the observer interface implementer
         * @param aCategory UID of Publish And Subscribe category
         * @param aKey subkey to specify the category event; to be used with Publish And Subscribe
         * @param aValue the new value
         * Event enumerations and uid:s of Publish And Subscribe can be found from PSVariables.h
         **/
        virtual void HandlePropertyChangeL( const TUid& aCategory,
                                            const TUint aKey,
                                            const TInt aValue ) = 0;

        /**
         * This is a callback function which is called when a P&S components returns an error
         * @param aCategory UID of Publish And Subscribe category
         * @param aKey subkey to specify the category event
         * @param aError an error code
         **/
        virtual void HandlePropertyChangeErrorL( const TUid& aCategory,
                                                 const TUint aKey,
                                                 TInt aError ) = 0;
    };

// INCLUDES

// CONSTANTS

// CLASS DECLARATION

/**
 * Observer class that observes changes of Property values and propogates them further.
 * The class defines a handle to a property, a single data value representing
 * an item of state information.
 *
 **/
class CDevEncStarterPropertyObserver : public CActive
	{
public:  // Constructors and destructor

    /**
     * Two-phased constructor.
     * @param aObserver a reference to the observer interface implementer
     * @param aCategory UID of Publish And Subscribe category
     * @param aKey subkey to specify the category event; to be used with Publish And Subscribe
     * Event enumerations and uid:s of Publish And Subscribe can be found from PSVariables.h
     **/
    static CDevEncStarterPropertyObserver* NewL( MDevEncStarterPropertyChangeObserver& aObserver,
                                            const TUid& aCategory,
                                            const TUint aKey );

    /**
     * Destructor.
     **/
    ~CDevEncStarterPropertyObserver();

    /**
     * Updates a value reference in correspondence with the current Property value
     * @param handle to a value which will be updated
     * @return KErrNone if operation was successful
     **/
	void GetValue( TInt& aValue ) const;

 private:
    /**
     * C++ default constructor overload.
     * Two-phased constructor.
     * @param aObserver a reference to the observer interface implementer
     * @param aCategory UID of Publish And Subscribe category
     * @param aKey subkey to specify the category event; to be used with Publish And Subscribe
     * Event enumerations and uid:s of Publish And Subscribe can be found from PSVariables.h
     **/
	CDevEncStarterPropertyObserver( MDevEncStarterPropertyChangeObserver& aObserver,
	                           const TUid& aCategory,
	                           const TUint aKey );

    /**
     * By default Symbian 2nd phase constructor is private.
     **/
    void ConstructL();

 protected:  // Functions from base classes
    /**
     * From CActive
     * Handles an active object’s request completion event
     **/
    void RunL();

    /**
     * From CActive,
     * Cancels and outstanding request
     **/
    void DoCancel();

 private:  // Data
 	// Value of a subscribed category property
 	TInt iValue;

    // handle to Publish And Subscribe component
    RProperty iProperty;

    // Observer to be notified when particular Property value has changed
    MDevEncStarterPropertyChangeObserver& iObserver;

    // UID of Publish And Subscribe category
    TUid iCategory;

    // subkey to be used with Publish And Subscribe
    TUint iKey;

	};

#endif  // CPROPERTYOBSERVER_H


// End of File
