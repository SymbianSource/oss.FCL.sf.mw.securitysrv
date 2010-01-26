/*
* Copyright (c) 2000, 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:    This file contains the headers of CCrBer and CCrBerSet classes.
*                 These classes are used to encode and decode BER object(s).
*
*/



#ifndef CR_BER
#define CR_BER

//  INCLUDES
#include <e32base.h>        // CBase
#include <asn1dec.h>       // ASN1 (Symbian)
#include "crdata.h"         // CCrData
#include "crdefs.h"         // Definitions

#include <bigint.h>

//  MACROS
#define LEN_BYTES_MAX    4  // Max amount of length bytes in BER encoded
                            // object. Probably this never changes, but
                            // you'll never know.
//  DATA TYPES
typedef TUint8 TBerTag;

//  CONSTANTS  
//////////////////////////////////////////////////////////////////////////
// BER types.
const TBerTag KBerEndOfContent  = 0x00;     // End of contents tag
const TBerTag KBerBoolean       = 0x01;     // Boolean tag
const TBerTag KBerInteger       = 0x02;     // Integer tag
const TBerTag KBerBitString     = 0x03;     // Bit string tag
const TBerTag KBerOctetString   = 0x04;     // Octet string tag
const TBerTag KBerNull          = 0x05;     // NULL tag
const TBerTag KBerOid           = 0x06;     // Object identifier tag
const TBerTag KBerNumS          = 0x12;	    // Numeric string		
const TBerTag KBerPrS           = 0x13;     // Printable string tag
const TBerTag KBerT61S          = 0x14;     // T61 string tag
const TBerTag KBerVideoS        = 0x15;	    // Video string tag				 
const TBerTag KBerIA5S          = 0x16;     // IA5 string tag
const TBerTag KBerUtc           = 0x17;     // UTC time tag
const TBerTag KBerGenTime       = 0x18;	    // Generalized Time tag					
const TBerTag KBerGraphS        = 0x19;	    // Graphics string tag			
const TBerTag KBerVisibleS      = 0x1A;     // Visible string					
const TBerTag KBerGeneralS      = 0x1B;     // Generalised string	
const TBerTag KBerBmpS          = 0x1E;     // Bmp string
const TBerTag KBerSeq           = 0x30;     // Sequence tag
const TBerTag KBerSet           = 0x31;     // Set tag

const TBerTag KBerUnknown       = 0xff;     // Unknown tag

const TBerTag KBerEncodedObject = 0xfe;     // Tag for ready-made
                                            // BER encoded objects

const TBerTag KBerLongLengthBit = 0x80;     // Long length bit
const TBerTag KBerConstructedBit= 0x20;     // Constructed bit
const TBerTag KBerConstructed   = 0x20;

const TBerTag KBerImplicit = 0x80;   // Implicit tag
const TBerTag KBerExplicit = 0x80;   // Explicit tag

const TBerTag KBerImplicitConstructed = 0xA0;   // Implicit constructed tag
const TBerTag KBerExplicitConstructed = 0xA0;   // Explicit constructed tag

const TBerTag KBerBooleanTrue   = 0xFF;     // Boolean true value   (DER)
const TBerTag KBerBooleanFalse  = 0x00;     // Boolean false value  (DER)
const TBerTag KBerNullContent   = 0x00;     // Null objects content

const TBerTag KBerBooleanLen    = 0x03;     // Boolean object length
const TBerTag KBerNullLen       = 0x02;     // Null object length
const TBerTag KBerShortLen      = 0x02;     // Tag + (short) length byte
const TBerTag KBerIndefiniteLen = 0x04;     // Indefinite length (xx 80 00 00)

const TInt KReadBufMax          = 256;      // Length of buffer used
								            // when reading from files.
const TUint KOpenAllLevels      = 255;      // OpenL(....) function
const TInt KOctetWidth          = 8;        
const TInt KObjectIDDot         = 1;        // object id encoding
const TInt KObjectIDFirstFactor = 4;        // object id encoding
const TInt KObjectIDSecondFactor = 10;      // object id encoding

const TInt KIntBufSize          = 6;        // Integer encoding buffer size 

// CLASS DECLARATION

/**
*  Class CCrBer is able to contain a BER encoded object of any type.
*
*  @lib crber.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CCrBer ): public CBase
    {
    /////////////////////////////////////////////
    // Constructors and destructors.
    private:
        CCrBer(TInt aLevel = 0);
        
        TAny ConstructL();
        
    public:
        IMPORT_C static CCrBer* NewLC(TInt aLevel = 0); 
        IMPORT_C static CCrBer* NewL(TInt aLevel = 0);
        IMPORT_C ~CCrBer();


        /////////////////////////////////////////////
        // Functions to open BER object.

        /**
        * Read next BER encoded object from the current position of
        * given CCrData to this object. Return the type of the BER
        * object, or KBerUnknown, if not of any known type. Only pointer
        * to aData is stored into this object, so Get* functions are
        * meaningful only if original data object is still existing,
        * when these functions are used.
        */
        IMPORT_C TBerTag Read(CCrData* aData);

        /**
        * Read type tag from current position in given CCrData.
        * Returns type tag or KBerUnknown, if not of any known type.
        */
        IMPORT_C static TBerTag ReadType(CCrData* aData);

        /**
        * Read length tags from current position in given CCrData.
        * Returns length. If length is indefinite, aIndefinite is set
        * to true, otherwise to false. In indefinite case length is 0.
        * Also sets amount of length bytes in aLenLen, if given.
        */
        IMPORT_C TUint ReadLen(
            CCrData* aData,
            TBool&   aIndefinite,
            TUint8*  aLenLen = 0);

        /**
        * Returns true if given BER tag is identified one.
        */
        IMPORT_C static TBool IsKnownType(TBerTag aTag);

        /**
        * Returns true if given BER tag is sequence, set,
        * implicit contructed, or explicit constructed tag.
        */
        IMPORT_C static TBool IsSeqOrSet(TBerTag aTag);
        
        /**
        * Finds next end-of-content (00 00) tag from given data.
        * Returns the distance between current place and the tag,
        * or 0, if not found. Moves data pointer to the next byte
        * _after_ the tag, i.e. two bytes longer than you might
        * except from the return value.
        */
        IMPORT_C TUint FindEndOfContent(CCrData* aData);

        /**
        * Open constructed encoding from this object.  
        * Parameters: aTarget; target for contentbytes.
        * Return Values:  Number of objects. 
        */
        IMPORT_C TInt OpenConstructedEncodingL(CCrData& aTarget);

        IMPORT_C TUint OpenConstructedEncodingWithTagL(
            CCrData* aData,
            CCrBer&  parentObj,
            HBufC8*  buf = 0);

        /////////////////////////////////////////////
        // Functions to get info about this BER object.
			
        // Return type of the object.
        IMPORT_C TBerTag Type();
    
        // Return value of the object (int or boolean object).
        // Only to encoding..
        IMPORT_C TInt Value();
        IMPORT_C RInteger GetBigInt();
        IMPORT_C TDesC8* ValuePtr();
        IMPORT_C TAny SetValue(TInt aInt);

        // Return amount of length bytes.
        IMPORT_C TInt LenLen();

        // Return begin of the whole object.
        IMPORT_C TInt ObjectBegin();

        // Return begin of the content.
        IMPORT_C TInt ContentBegin();

        // Return length of the content.
        IMPORT_C TInt ContentLen();

        // Return length of the whole object.
        IMPORT_C TInt ObjectLen();

        // Return true if this object is indefinite length, else false.
        IMPORT_C TBool Indefinite();

        // Return pointer to data object.
        IMPORT_C CCrData* Data();

        // Return whole BER object with tag and stuff. For implementation
        // reasons ObjectL returns only max 255 bytes of data.

        IMPORT_C TInt Object(HBufC8* aBuf);
        IMPORT_C TInt ObjectL(HBufC* aBuf);

        // Return content of this object. For implementation reasons
        // ContentL returns only max 255 bytes of data.
        IMPORT_C TInt Content(HBufC8* aBuf);
        IMPORT_C TInt ContentL(HBufC* aBuf);

        // Read aAmount of data starting from begin to the buffer.
        // Return amount of data read. For implementation reasons
        // BufferL can be used only to read max 255 bytes of data.
        static IMPORT_C TInt BufferL(
            HBufC8*   aBuf,
            CCrData*  aData,
            TUint     aBegin = 0,
            TUint     aAmount = KReadBufMax);
        static IMPORT_C TInt BufferL(
            HBufC*    aBuf,
            CCrData*  aData,
            TUint     aBegin = 0,
            TUint8    aAmount = 255);

        // Return nesting level of this object.
        IMPORT_C TUint Level();

        // Set nesting level of this object.
        IMPORT_C TUint SetLevel(TUint aLevel);
      
        ///////////////////////////////////////////
        // Get content 

        // Return value of integer object.
        // e.g. 21
		IMPORT_C TInt GetIntegerL();
		IMPORT_C RInteger GetLongIntegerL();

        // Return value of boolean object.
        // e.g. ETrue
        IMPORT_C TBool GetBooleanL();

		// Return content of object identifier object.
        // e.g. 1.2.3.4.5
        IMPORT_C HBufC* GetOidL();
		
        // Return content of octet string object.
        // e.g. 45 53 65 77 
        IMPORT_C HBufC8* GetOctetStringL();
        IMPORT_C TAny GetOctetStringL(CCrData& Trg);

        // Return content of content string object.
        IMPORT_C HBufC8* GetContentStringLC();
		
        // Return content of IA5 string object.
        // e.g. "hello"
        IMPORT_C HBufC* GetIA5StringL();
		
        // Return content of UTC time object.
        IMPORT_C TTime GetUTCTimeL();
		
        // Return content of printable string object.
        // e.g. "hello"
        IMPORT_C HBufC* GetPrintStringL();
		
        // Return content of teletext string object.
        IMPORT_C HBufC* GetTeletextStringL();
		
        // Return content of sequence object.
        // e.g. 02 01 01 01 01 FF 
        IMPORT_C HBufC8* GetSequenceL();
		
        // Return content of videotext string object.
        IMPORT_C HBufC* GetVideoStringL();
		
        // Return content of generalised string object.
        IMPORT_C HBufC* GetGeneralStringL();
		
        // Return content of visible characters string object.
        IMPORT_C HBufC* GetVisibleStringL();
		
        // Return content of a generalised graphical string obj.
        IMPORT_C HBufC* GetGraphicsStringL();
		
        // Basically, TIME+ century
        // Return content of a generalized time string obj.
        IMPORT_C TTime GetGeneralizedTimeL();
		
        // Return content of numeric string object
        // only for numbers. 
        IMPORT_C HBufC* GetNumericStringL();


        /////////////////////////////////////////////
        // Functions to modify info about this BER object.

        // Add given integer to content length of this object.
        IMPORT_C TAny AddToContentLen(TInt iLen);

        // Calculate object length from tag, length's length, and
        // content's length. Used to 'close' indefinite objects, meaning
        // that their end-of-contents tag is finally found, so their
        // length can be calculated.
        IMPORT_C TAny SetObjectLen();

        // Set object length to object length - content length. = Length of tag 
        // and lenbits (+lenlen).
        IMPORT_C TAny SetObjLenWithOutContent(TUint aContentLen);        
    
    public:
        // Encoding
        // These functions are used by CCrBerSet::Create*
        // Save type,length and value. 
        TAny CreateBool(TBool aBool);
        // Save type,length and value.
        TAny CreateInt(TInt aInt);
        TAny CreateLongInt(RInteger& aData);
        // Save type and value of object.
        TAny CreateNull();
        // Save type, length and pointer to string
        TAny CreateOIdL(TDesC8* aString);
        TAny CreateString(TBerTag aTag, TDesC8* aString);
        TAny CreateString(TBerTag aTag, CCrData* aData);

        // Save tag and value of constructed type.
        TAny CreateStart(TBerTag aTag, TBool aDefinite);
        // Save type.
        TAny CreateEnd();

        // This fuction can be used to create a BER object
        // from buffer, which already contains a full BER
        // encoded object.
        TAny CreateBEREncodedObject(TDesC8* aBuffer);
        
        
    public: // Data.
        // Maximum sixe of TUint.
        // static TUint iMaxUint;
        TUint iMaxUint;

        // Maximum sixe of TInt.
        //static TInt iMaxInt;
        TInt iMaxInt;

    private: // Data
        // Type.
        TBerTag iType;

        // iValue is used in encoding
        TInt iValue;
        // Pointer to value (encoding).
        TDesC8* iValuePtr;
        RInteger iInt;

        // Amount of length bytes.
        TUint8 iLenLen;

        // Indefinite length or not.
        TBool iIndefinite;

        // These indexes indicate places in the CCrData
        // object pointed by the member iData.
        TUint iObjectBegin;     // Place where whole BER object begins.
        TUint iContentBegin;    // Place where BER object's content begins.
        TUint iContentLen;      // Length of the BER object's content.
        TUint iObjectLen;       // Length of the whole BER object.

        // Pointer to the object containing the actual object.
		CCrData* iData;
		

        // Nesting level of this object. Not set in this object,
        // but given by caller.
        TInt iLevel;

        // End of content bytes.
        // static TUint8 iEOCBytes[2];
        TUint8 iEOCBytes[2];

    };

/**
*  Class CCrBerSet 
*  CCrBerSet contains set of CCrBer objects.
*
*  @lib crber.lib
*  @since Series 60 3.0
*/
class CCrBerSet : public CArrayPtrSeg<CCrBer>
    {
    /////////////////////////////////////////////
    // Constructors and destructors.
    private:
        CCrBerSet(TInt aGranularity); 
        TAny ConstructL();
         
        
    public:
        IMPORT_C static CCrBerSet* NewLC(TInt aGranularity);
        IMPORT_C static CCrBerSet* NewL(TInt aGranularity);
        IMPORT_C ~CCrBerSet();

       
    /////////////////////////////////////////////
    // Functions.
        
        // Opens next BER encoded object from the current position of
        // given CCrData to this object set. Opens also all nested BER
        // objects as deep as aRecursionLevel tells. If it is zero, one
        // object is read, if it is one, all objects at first level are
        // read, if it is 255, all objects at 255 first levels are
        // extracted, and if it is 256, all objects at any level are
        // extracted. Returns the amount of extracted objects.
        // The data pointer of CCrData points right after the
        // opened BER object after this function is finished.
        IMPORT_C TUint OpenL(
            CCrData* aData,
            TUint8   aRecursionLevel = 1);

        // Appends given new object into this set. Updates all previous
        // items in this set, which have indefinite length, with the
        // length of the new object.
        IMPORT_C TAny AppendAndUpdateL(CCrBer *aBerObject);

        // Updates all previous items in this set, which have
        // indefinite length, with the length of the new object.
        IMPORT_C TAny Update(CCrBer *aBerObject);

        // Finds last open indefinite length sequence
        // or set from this set at given level and closes it.
        IMPORT_C TAny CloseLastSeqOrSet(TUint aLevel);
        

    ////////////////////////////////////////////
    // Encoding functions

        // Function creates Ber boolean object (CCrBer object).
        // Write a object to CCrData (file) by Flush(..)  
        // (ETrue or EFalse) (Tag 01)
        IMPORT_C TAny CreateBoolL(TBool aValue);

        //   Function creates Ber integer object (CCrBer object).
        // Write a object to CCrData (file) by Flush(..)  Parameter is either
        // a positive whole number, or a negative whole number, or zero.
        IMPORT_C TAny CreateIntL(TInt aValue);
        IMPORT_C TAny CreateLongIntL(RInteger& aValue);

        // Function creates Ber octet string object.
        // The OCTET STRING  is an arbitrarily long binary value.
        IMPORT_C TAny CreateOctetL(TDesC8& aString);
        IMPORT_C TAny CreateOctetL(CCrData* aData);


        // Function creates Ber null object (CCrBer object).
        // (0x05 0x00).
        IMPORT_C TAny CreateNullL();

        // Function creates Ber object identifier object.
        // type tag = (0x06)
        // e.g. berSet->AppendObjectId(_L8("1.2.3.4.5")
        IMPORT_C TAny CreateOIdL(TDesC8& aString);

        // Function creates Ber Numeric string object (CCrBer object).
        // The NumericString is defined to only contain the characters 0-9 
        // and space. 
        IMPORT_C TAny CreateNumericL(TDesC8& aString);

        // Function creates Ber Printable string object (CCrBer object)
        // The PrintableString is defined to only contain
        // the characters A-Z, a-z, 0-9, space,
        // and the punctuation characters ()-+=:',./?.
        IMPORT_C TAny CreatePrintableL(TDesC8& aString);

        // Function creates Ber T61 string object (CCrBer object).
        // The TeletexString is a string, containing characters
        // according to the T.61 character set. 
        IMPORT_C TAny CreateT61L(TDesC8& aString);

        // Function creates Ber Video text object (CCrBer object) 
        // type tag = (0x15). 
        IMPORT_C TAny CreateVideoTexL(TDesC8& aString);

        // Function creates Ber IA5 string object (CCrBer object).
        // IA5 (International Alphabet 5)
        // is equivalent to US-ASCII.
        IMPORT_C TAny CreateIA5L(TDesC8& aString);

        // Function creates Ber UTC time object (CCrBer object).
        // Note this value only represents years using two digits.
        // e.g. berSet->AppendUTCTime(_L8("980801000000Z"));
        IMPORT_C TAny CreateUTCL(TDesC8& aString);

        // Function creates Ber generalised time object 
        // (CCrBer object). type = (0x18). Unlike UTCTime
        // it represents years using 4 digits.
        IMPORT_C TAny CreateGenTimeL(TDesC8& aString);

        // Function creates Ber graphical string object (CCrBer object)
        // type byte = (0x19).
        IMPORT_C TAny CreateGraphicalL(TDesC8& aString);

        // Function creates Ber visible string object (CCrBer object)
        // type = (0x1A).
        IMPORT_C TAny CreateVisibleL(TDesC8& aString);

        // Function creates Ber general string object (CCrBer object)
        // type = (0x1B).       
        IMPORT_C TAny CreateGeneralL(TDesC8& aString);

        // Append start of sequence (30 xx, xx = length)
        // True --> DefiniteLen, False --> IndefiniteLength. 
        IMPORT_C TAny CreateSeqStartL(TBool aDefinite);

        // Append start of set (31 xx, xx = length)
        // True --> DefiniteLen, False --> IndefiniteLength. 
        IMPORT_C TAny CreateSetStartL(TBool aDefinite);

        // Append start of the constructed.
        IMPORT_C TAny CreateConstructedStartL(TBerTag aTag, TBool aDefinite);

        // Append end to last constructed (e.g. seq)
        // type (definite or indefinite).
        IMPORT_C TAny CreateEndL();

        // This fuction can be used to create a BER object
        // from buffer, which already contains a full BER
        // encoded object.
        IMPORT_C TAny CreateBEREncodedObjectL(TDesC8& aBuffer);

        // Function writes all created crber objects (which are 
        // AppendL to set) to file. Function also fix all definite
        // lengths. Call this function then crberSet is ready 
        // (all berObjects are delived).
        IMPORT_C TInt FlushL(CCrData* aTarget);

    public:

        // Encode and write object identifier.
        TInt AppendObjectIdL(const TDesC8& aString,TBool aOnlyLen = EFalse);

    private:
        // These functions are used by CCrBerSet::Flush(...)

        // Write constructed type start.
        TInt AppendConstructedL(TBerTag aTag, TUint aLength = 0);
        
        // close indefinite length. (= add 0x00 0x00) 
        TInt CloseIndefinite(); 
        
        // Encode and write boolean object. 
        TInt AppendBoolL(TBool aBool);   
        
        // Encode and write integer object.
        TInt AppendIntL(TInt aData); 
        TInt AppendLongIntL(RInteger& aData);
        
        // Encode and write null object. (0x05 0x00)
        TInt AppendNull();
        
        // Encode and write string to CCrData (file).
        TInt AppendStringL(TBerTag aTag, const TDesC8& aString);
        TInt AppendStringL(TBerTag aTag, CCrData* const aData);

        // Write BER encoded object to CCrData (file).
        // Note that this function doesn't add any tags
        // etc. it trusts that the given object already
        // is a whole BER encoded object.
        TInt AppendBerEncodedObject(const TDesC8& aString);    
        
    private: // Data

        // Target file for objects (encoding). 
        CCrData* iTarget;
        
        // Nesting level of object. (encoding).
        TInt iLevel;
        
        // Max level which is used in encoding. (Flush(..))
        TInt iMaxLevel;
    };

#endif CR_BER
