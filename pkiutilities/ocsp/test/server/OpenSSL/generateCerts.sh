#!/bin/sh

# Generate certs for testing OCSP against OpenSSL implementation
#
# There are two CAs:
#   ca1 signs a responder cert which signs responses
#   ca2 signs responses with its ca cert

# Trash existing data
rm -rf ca1 ca2 certs tmp
mkdir ca1 ca2 certs tmp 

# ca1 ##########################################################################

# RSA keys, CA signed responder cert signed responses

# Create ca files
touch ca1/index.txt
echo "01" > ca1/serial
mkdir ca1/private
mkdir ca1/certs

# Generate root cert
openssl req -x509 -newkey rsa:1024 -keyout ca1/private/cakey.pem -out ca1/cacert.pem -subj "/O=Symbian/CN=CA Root Cert" -days 3650 -nodes 
openssl x509 -in ca1/cacert.pem -outform DER -out certs/ca1-root.der

# Generate ocsp responder cert 
openssl req -newkey rsa:1024 -keyout ca1/private/reskey.pem -out tmp/req.pem -subj "/O=Symbian/CN=CA OCSP Responder" -days 3650 -nodes
openssl ca -config openssl.config -name ca1 -in tmp/req.pem -batch -days 3650
openssl x509 -in ca1/certs/01.pem -outform DER -out certs/ca1-responder.der

# Generate entity cert 1
openssl req -newkey rsa:1024 -keyout tmp/key.pem -out tmp/req.pem -subj "/O=Symbian/CN=Entity Cert 1 (Good)" -days 3650 -nodes
openssl ca -config openssl.config -name ca1 -in tmp/req.pem -batch -days 3650
openssl x509 -in ca1/certs/02.pem -outform DER -out certs/ca1-entity1.der

# Generate entity cert 2 and revoke it
openssl req -newkey rsa:1024 -keyout tmp/key.pem -out tmp/req.pem -subj "/O=Symbian/CN=Entity Cert 2 (Revoked)" -days 3650 -nodes
openssl ca -config openssl.config -name ca1 -in tmp/req.pem -batch -days 3650
openssl x509 -in ca1/certs/03.pem -outform DER -out certs/ca1-entity2.der
openssl ca -config openssl.config -name ca1 -revoke ca1/certs/03.pem -crl_reason keyCompromise

# Generate entity cert 3 and then remove it from the ca
openssl req -newkey rsa:1024 -keyout tmp/key.pem -out tmp/req.pem -subj "/O=Symbian/CN=Entity Cert 3 (Unknown)" -days 3650 -nodes
openssl ca -config openssl.config -name ca1 -in tmp/req.pem -batch -days 3650
openssl x509 -in ca1/certs/04.pem -outform DER -out certs/ca1-entity3.der
mv ca1/index.txt tmp
head -3 tmp/index.txt > ca1/index.txt
rm ca1/certs/04.pem

# ca2 ##########################################################################

# DSA keys, CA cert signs responses

# Create ca files
touch ca2/index.txt
echo "01" > ca2/serial
mkdir ca2/private
mkdir ca2/certs

# Generate root cert
openssl req -x509 -newkey rsa:1024 -keyout ca2/private/cakey.pem -out ca2/cacert.pem -subj "/O=Symbian/CN=CA Root Cert" -days 3650 -nodes 
openssl x509 -in ca2/cacert.pem -outform DER -out certs/ca2-root.der

# Generate entity cert 1
openssl req -newkey rsa:1024 -keyout tmp/key.pem -out tmp/req.pem -subj "/O=Symbian/CN=Entity Cert 1 (Good)" -days 3650 -nodes
openssl ca -config openssl.config -name ca2 -in tmp/req.pem -batch -days 3650
openssl x509 -in ca2/certs/01.pem -outform DER -out certs/ca2-entity1.der

# Generate entity cert 2 and revoke it
openssl req -newkey rsa:1024 -keyout tmp/key.pem -out tmp/req.pem -subj "/O=Symbian/CN=Entity Cert 2 (Revoked)" -days 3650 -nodes
openssl ca -config openssl.config -name ca2 -in tmp/req.pem -batch -days 3650
openssl x509 -in ca2/certs/02.pem -outform DER -out certs/ca2-entity2.der
openssl ca -config openssl.config -name ca2 -revoke ca2/certs/02.pem -crl_reason keyCompromise

# Generate entity cert 3 and then remove it from the ca
openssl req -newkey rsa:1024 -keyout tmp/key.pem -out tmp/req.pem -subj "/O=Symbian/CN=Entity Cert 3 (Unknown)" -days 3650 -nodes
openssl ca -config openssl.config -name ca2 -in tmp/req.pem -batch -days 3650
openssl x509 -in ca2/certs/03.pem -outform DER -out certs/ca2-entity3.der
mv ca2/index.txt tmp
head -2 tmp/index.txt > ca2/index.txt
rm ca2/certs/03.pem

# To use DSA instead of RSA, first generate DSA parameters:
#   openssl dsaparam -out tmp/dsaparam.pem 1024
# And use this in the newkey options
#   openssl req -x509 -newkey dsa:tmp/dsaparam.pem ...

# Tidy
rm -rf tmp
