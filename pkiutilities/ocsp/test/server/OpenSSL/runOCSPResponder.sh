#!/bin/sh

# Run OpenSSL OCSP responder

# CA1, normal, 5 responses
openssl ocsp -index ca1/index.txt -CA ca1/cacert.pem -rsigner ca1/certs/01.pem -rkey ca1/private/reskey.pem -port 100 -nrequest 5

# CA2, normal, 5 responses
openssl ocsp -index ca2/index.txt -CA ca2/cacert.pem -rsigner ca2/cacert.pem -rkey ca2/private/cakey.pem -port 100 -nrequest 5

# CA1, no certs in response, 1 response
openssl ocsp -index ca1/index.txt -CA ca1/cacert.pem -rsigner ca1/certs/01.pem -rkey ca1/private/reskey.pem -port 100 -resp_no_certs -nrequest 1

# CA2, no certs in response, 1 response
openssl ocsp -index ca2/index.txt -CA ca2/cacert.pem -rsigner ca2/cacert.pem -rkey ca2/private/cakey.pem -port 100 -resp_no_certs -nrequest 1

# CA1, responder cert idenified by id, 1 response
openssl ocsp -index ca1/index.txt -CA ca1/cacert.pem -rsigner ca1/certs/01.pem -rkey ca1/private/reskey.pem -port 100 -resp_key_id -nrequest 1

# CA2, responder cert idenified by id, 1 response
openssl ocsp -index ca2/index.txt -CA ca2/cacert.pem -rsigner ca2/cacert.pem -rkey ca2/private/cakey.pem -port 100 -resp_key_id -nrequest 1
