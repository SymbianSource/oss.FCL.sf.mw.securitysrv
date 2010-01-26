
This directory contains all the source code and associated data for testing the
OCSP module against external OCSP servers.

The executable is called TOCSP.EXE.  It takes the name of a command script as a
parameter, followed by the name of the log file.  If the third paramteter "OOM"
is specified, it runs out of memory tests on the script.

This command file must be unicode.  Commands for this file are described in
command.cpp

Several command files are given, for testing against different servers:

scripts\XCert.txt

  Tests against the XCert OCSP server running internally within Symbian
  (lon-ocsp01) on http://10.35.2.47:445/ocsp.xuda

scripts\XCert-local.txt

  Tests against logged responses from the XCert OCSP server.  These tests
  include almost all of the functionality in the XCert.txt script, but can be
  run without a network connection.  The responses were produced by running the
  XCert-logging.txt script.

scripts\OpenSSL-local.txt

  Tests against logged requests and responses from an OpenSSL OCSP server.  The
  responses were produced by running the OpenSSL-logging.txt script in
  conjunction with a local OpenSSL server, run using the scripts in
  server/OpenSSL.

scripts\Error.txt

  Tests against a set of pre-computed erroneous OCSP responses, that have been
  hand-produced (signatures for these come from the 'resign' utility in a
  subdirectory of this directory)

scripts\Valicert.txt

  These tests are no longer run.  They used to use a Valicert OCSP server
  running internally within Symbian (cam-security.intra) on 10.178.2.2:133

scripts\CertCo.txt

  These tests are no longer run.  They use to a CertCo OCSP server, hosted by
  CertCo, on 208.222.33.18:80/ocspv1

These files are copied to Z:\system\tocsp\scripts\. on the EPOC device as part
of 'abld test export'.  This also copies across all the appropriate test data.

TOCSP sets CACerts.dat itself - you don't need to set it, and the existing one
gets trashed.
