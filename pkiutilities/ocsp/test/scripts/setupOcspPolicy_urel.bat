@rem
@rem Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
@rem All rights reserved.
@rem This component and the accompanying materials are made available
@rem under the terms of "Eclipse Public License v1.0"
@rem which accompanies this distribution, and is available
@rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
@rem
@rem Initial Contributors:
@rem Nokia Corporation - initial contribution.
@rem
@rem Contributors:
@rem
@rem Description:
@rem

@ECHO Off
REM 

REM Copies the OCSP repository specified in %1 to the correct place
REM Called with the the repository file as the only argument

set NAME=%1

set FILE=\epoc32\winscw\c\system\tocsp\scripts\%1

echo Installing OCSP repository from %FILE%

IF %NAME% == donothing GOTO END

IF %NAME% == 2002b28b-new.txt GOTO NEWREPOSITORY

IF %NAME% == 2002b28b.txt GOTO OLDREPOSITORY


:NEWREPOSITORY
copy \epoc32\release\winscw\urel\z\private\10202be9\2002b28b.txt \epoc32\release\winscw\urel\z\private\10202be9\2002b28b-old.txt
copy %FILE% \epoc32\release\winscw\urel\z\private\10202be9\2002B28B.txt
GOTO END

:OLDREPOSITORY
copy \epoc32\release\winscw\urel\z\private\10202be9\2002b28b-old.txt \epoc32\release\winscw\urel\z\private\10202be9\2002b28b.txt
del \epoc32\release\winscw\urel\z\private\10202be9\2002b28b-old.txt



:END
cd \epoc32\winscw\c\twtlscert\scripts\batchfiles
perl certstorePlugins disable_all
perl certstorePlugins enable filecertstore.dll