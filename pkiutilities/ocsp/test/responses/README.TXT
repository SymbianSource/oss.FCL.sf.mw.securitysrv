This directory contains pre-computed OCSP responses which are used by the input
scripts for TOCSP.  Most have been hand-edited to contain particular errors, and
in some cases then re-signed using the 'resign' utility.

(* signifies resigning was used)

The following are used in the Error.txt input script:

response.000     Original response, upon which other non-trivial responses are based
response.001     'malformedRequest'
response.002     'internalError'
response.003     'tryLater'
response.004     ..invalid responseStatus
response.005     'sigRequired'
response.006     'unauthorised'
response.007     ..invalid responseStatus
response.008     ..invalid responseStatus (-ve)
response.009     invalid RSA signature
response.010     corrupt RSA signature data (#unused bits not 0)
response.011*    invalid hashAlgorithm in CertId
response.012*    corrupt issuerNameHash in CertId
response.013*    corrupt issuerKeyHash in CertId
response.014*    corrupt serialNumber in CertId
response.015     hash algorithm specified in signature doesn't match that
                  used (which is specified before the signature)
                  - produced using a modified version of resign.exe
response.016     Invalid response type (outside signed data portion)
response.017*    Inserted 'version' field with value v1 (shouldn't be there since this is
                  the default value, and defaults are skipped in DER)
response.018*    As 017, but with unrecognised version value '1'.
response.019*    producedAt before thisUpdate
response.020*    Added nextUpdate field - one month after thisUpdate
response.021*    From 020, set producedAt > nextUpdate
response.022*    From 020, set thisUpdate > nextUpdate (producedAt < nextUpdate)
response.023*    From 020, set thisUpdate > nextUpdate (producedAt between the two)
response.024*    From 020, set thisUpdate > nextUpdate (producedAt > thisUpdate)
response.025*    Issuer name corrupted (doesn't match signing cert subject field)
response.026-030 Corrupt length fields (* on 29)
response.031-035 Corrupt tag fields (* on 32, 34)

The following are used in XCert.txt:

response.n1      Testing Nonce on, incorrect value / unexpected nonce received
response.n2      Testing Nonce on, response with missing nonce

The following are used in XCert-local.txt, in addition to those used in XCert.txt:

XCert_00_Good_RSA_XCert.rsp
XCert_01_Revoked_RSA_XCert.rsp
XCert_02_Suspended_RSA_XCert.rsp
XCert_03_Unknown_RSA_XCert.rsp
XCert_04_Good_DSA_XCert.rsp
XCert_05_Revoked_DSA_XCert.rsp
XCert_06_Suspended_DSA_XCert.rsp
XCert_07_Unknown_DSA_XCert.rsp
XCert_08_All_RSA_XCert.rsp
XCert_09_All_DSA_XCert.rsp
XCert_10_Expired_Subject_and_Issuer.rsp
XCert_11_Expired_Signing.rsp
XCert_12_Expired_Subject.rsp
XCert_13_Unspecified_Date.rsp
XCert_14_Expired_subject_valid_time.rsp
XCert_15_Expired_unspecified.rsp
XCert_16_Expired_specified.rsp
XCert_20_Nonce_Off.rsp
XCert_22_BadSig_DSA_XCert.rsp
