
This directory contains the files for a small EPOC console app called RESIGN.EXE

This app takes filenames of an OCSP response and of a private .key as command line
parameters.

The purpose of the app is to resign the response using that key.  This can be used
to re-sign OCSP responses that have been edited, so that the signatures are valid.
This is used to generate erroneous OCSP responses for error-testing our OCSP client.