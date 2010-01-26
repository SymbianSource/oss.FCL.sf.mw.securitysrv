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
* Description:  class M3GPPAuthenticationCallback definition
*
*/


#ifndef	__M3GPPAUTHENTICATIONCALLBACK_H__
#define	__M3GPPAUTHENTICATIONCALLBACK_H__ 

// System includes
#include <uri8.h>
#include <stringpool.h>
#include <http/rhttpsession.h>

/** 
	@public
	@class			M3GPPAuthenticationCallback

	The mixin that needs to be implemented if 3gpp digest AKA authentication is to be
	supported. This supplies the username and password and nonce when they are
	needed for authentication.
	To use, subclass off this class and implement GetCredentialsL. To install
	the class into a session, call InstallAuthenticationL.
 */

class M3GPPAuthenticationCallback
	{
 public:
	/** Gets some credentials. Note that the first 3 parameters are
		informational and many clients may not have a use for
		them. Clients may want to display some or all of these
		parameters to the user, so that the user knows who they are
		giving their credentials to, and how securely they will be
		transmitted.
		@param aURI The URI being requested
		@param aRealm The realm being requested
		@param aNonce The value of the nonce field: used to transport RAND/AUTN vectors
		@param aAuthenticationType The type of authentication (basic or digest)
		@param aAlgorithm The algorithm used in the Digest procedure eg MD5
		@param aUsername The username is returned in this parameter. 
		The caller will close the returned string.
		@param aPassword The password is returned in this parameter
		The caller will close the returned string.
		@param aResync The AUTS - authentication resyncronization value is returned in this parameter
		The caller will close the returned string. If aResyncRequired is false this variable will not be initialized.
		@param aResyncRequired will be set to true, if call caused the uicc to request resynchronization
		@return True if credentials have been obtained. 
		@exception Anything. If this function leaves the transaction will be 
		failed. 
	*/
	//##ModelId=3B1E52A5015D
	virtual TBool GetCredentialsL(const TUriC8& aURI, RString aRealm, RString aNonce, 
								  RString aAlgorithm,
								  RString& aUsername, 
								  RString& aPassword,
								  RString& aResync,
								  TBool& aResyncRequired) = 0;

	};



#endif //	__M3GPPAUTHENTICATIONCALLBACK_H__
                                                                 

//EOF
