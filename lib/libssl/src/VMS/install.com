$! INSTALL.COM -- Installs the files in a given directory tree
$!
$! Author: Richard Levitte <richard@levitte.org>
$! Time of creation: 23-MAY-1998 19:22
$!
$! P1	root of the directory tree
$!
$	IF P1 .EQS. ""
$	THEN
$	    WRITE SYS$OUTPUT "First argument missing."
$	    WRITE SYS$OUTPUT "Should be the directory where you want things installed."
$	    EXIT
$	ENDIF
$
$	ROOT = F$PARSE(P1,"[]A.;0",,,"SYNTAX_ONLY,NO_CONCEAL") - "A.;0"
$	ROOT_DEV = F$PARSE(ROOT,,,"DEVICE","SYNTAX_ONLY")
$	ROOT_DIR = F$PARSE(ROOT,,,"DIRECTORY","SYNTAX_ONLY") -
		   - "[000000." - "][" - "[" - "]"
$	ROOT = ROOT_DEV + "[" + ROOT_DIR
$
$	DEFINE/NOLOG WRK_SSLROOT 'ROOT'.] /TRANS=CONC
$	DEFINE/NOLOG WRK_SSLVLIB WRK_SSLROOT:[VAX_LIB]
$	DEFINE/NOLOG WRK_SSLALIB WRK_SSLROOT:[ALPHA_LIB]
$	DEFINE/NOLOG WRK_SSLINCLUDE WRK_SSLROOT:[INCLUDE]
$	DEFINE/NOLOG WRK_SSLVEXE WRK_SSLROOT:[VAX_EXE]
$	DEFINE/NOLOG WRK_SSLAEXE WRK_SSLROOT:[ALPHA_EXE]
$	DEFINE/NOLOG WRK_SSLCERTS WRK_SSLROOT:[CERTS]
$	DEFINE/NOLOG WRK_SSLPRIVATE WRK_SSLROOT:[PRIVATE]
$
$	IF F$PARSE("WRK_SSLROOT:[000000]") .EQS. "" THEN -
	   CREATE/DIR/LOG WRK_SSLROOT:[000000]
$	IF F$PARSE("WRK_SSLINCLUDE:") .EQS. "" THEN -
	   CREATE/DIR/LOG WRK_SSLINCLUDE:
$	IF F$PARSE("WRK_SSLROOT:[VMS]") .EQS. "" THEN -
	   CREATE/DIR/LOG WRK_SSLROOT:[VMS]
$
$	EXHEADER := vms_idhacks.h
$
$	COPY 'EXHEADER' WRK_SSLINCLUDE: /LOG
$
$	OPEN/WRITE SF WRK_SSLROOT:[VMS]OPENSSL_STARTUP.COM
$	WRITE SYS$OUTPUT "%OPEN-I-CREATED,  ",F$SEARCH("WRK_SSLROOT:[VMS]OPENSSL_STARTUP.COM")," created."
$	WRITE SF "$! Startup file for Openssl 0.9.2-RL 15-Mar-1999"
$	WRITE SF "$!"
$	WRITE SF "$! Do not edit this file, as it will be regenerated during next installation."
$	WRITE SF "$! Instead, add or change SSLROOT:[VMS]OPENSSL_SYSTARTUP.COM"
$	WRITE SF "$!"
$	WRITE SF "$! P1	a qualifier to DEFINE.  For example ""/SYSTEM"" to get the logical names"
$	WRITE SF "$!	defined in the system logical name table."
$	WRITE SF "$!"
$	WRITE SF "$	ARCH = ""VAX"""
$	WRITE SF "$	IF F$GETSYI(""CPU"") .GE. 128 THEN ARCH = ""ALPHA"""
$	WRITE SF "$	DEFINE/NOLOG'P1	SSLROOT		",ROOT,".] /TRANS=CONC"
$	WRITE SF "$	DEFINE/NOLOG'P1	SSLLIB		SSLROOT:['ARCH'_LIB]"
$	WRITE SF "$	DEFINE/NOLOG'P1	SSLINCLUDE	SSLROOT:[INCLUDE]"
$	WRITE SF "$	DEFINE/NOLOG'P1	SSLEXE		SSLROOT:['ARCH'_EXE]"
$	WRITE SF "$	DEFINE/NOLOG'P1	SSLCERTS	SSLROOT:[CERTS]"
$	WRITE SF "$	DEFINE/NOLOG'P1	SSLPRIVATE	SSLROOT:[PRIVATE]"
$	WRITE SF "$"
$	WRITE SF "$!	This is program can include <openssl/{foo}.h>"
$	WRITE SF "$	DEFINE/NOLOG'P1	OPENSSL		SSLINCLUDE:"
$	WRITE SF "$"
$	WRITE SF "$	IF F$SEARCH(""SSLROOT:[VMS]OPENSSL_SYSTARTUP.COM"") .NES."""" THEN -"
$	WRITE SF "	   @SSLROOT:[VMS]OPENSSL_SYSTARTUP.COM"
$	WRITE SF "$"
$	WRITE SF "$	EXIT"
$	CLOSE SF
$
$	COPY OPENSSL_UTILS.COM WRK_SSLROOT:[VMS]/LOG
$
$	EXIT
