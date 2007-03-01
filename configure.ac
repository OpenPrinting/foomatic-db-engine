dnl Process this file with autoconf to produce a configure script.
AC_INIT([footmatic-db-engine], 3.0.2)
AC_CONFIG_SRCDIR(foomatic-perl-data.c)

dnl If the user didn't specify the $sysconfdir on the command line, let it
dnl be /etc, not /usr/local/etc or /usr/etc
if [[ x$sysconfdir = 'x${prefix}/etc' ]]; then
  sysconfdir=/etc
fi

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl Checks for libraries.
dnl AC_PATH_XTRA
dnl LIBS="$LIBS"

dnl
dnl Test for libxml
dnl
xml_version=
AC_PATH_PROG(XML2_CONFIG, xml2-config)
if test ! -z "$XML2_CONFIG"; then
        xml_version=libxml2
        XML_CFLAGS=`$XML2_CONFIG --cflags`
        XML_LIBS=`$XML2_CONFIG --libs`
fi
if test -z "$xml_version"; then
        AC_PATH_PROG(XML_CONFIG, xml-config)
        if test ! -z "$XML_CONFIG"; then
                xml_version=libxml
                XML_CFLAGS=`$XML_CONFIG --cflags`
                XML_LIBS=`$XML_CONFIG --libs`
        fi
fi
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
dnl AC_CHECK_HEADERS(libxml/xmlmemory.h libxml/parser.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_CHECK_FUNCS(strdup strspn strstr xmlParseFile xmlCreatePushParserCtxt xmlParseChunk xmlFreeParserCtxt xmlFreeDoc xmlDocGetRootElement xmlNodeListGetString)

dnl Checks for programs/directories which are used by Foomtic

SPOOLSEARCHPATH=/var/spool:/usr/spool/:/usr/local/spool:/usr/local/var/spool:/var/local/spool:/var/local
LOGSEARCHPATH=/var/log:/usr/log:/usr/local/log:/usr/local/var/log:/var/local/log:/var/local
SYSCONFSEARCHPATH=/etc:/usr/etc:/usr/local/etc:/var/etc
SBINSEARCHPATH=/usr/sbin:/sbin:/usr/local/sbin:/etc/sbin
BINSEARCHPATH=/usr/bin:/bin:/usr/local/bin
DATASEARCHPATH=/usr/share:/usr/local/share:/usr/lib:/usr/local/lib:/opt
LIBSEARCHPATH=/usr/lib:/lib:/usr/local/lib:/usr/local/libexec:/opt
PTALPIPESSEARCHPATH=/var/run:/var:/var/ptal:/usr/var/run:/usr/var:/usr/var/ptal:/usr/local/var/run:/usr/local/var:/usr/local/var/ptal:/dev:/dev/ptal
MTINKPIPESSEARCHPATH=/var:/var/run:/usr/var/:/usr/var/run:/usr/local/var:/usr/local/var/run:/dev

# There is a bug in some versions of autoconf that causes
# bad expansion of paths in the AC_PATH_* macros.  This is very annoying

BSB=$BINSEARCHPATH:$SBINSEARCHPATH
SBB=$SBINSEARCHPATH:$BINSEARCHPATH
SYS_LIB_PATH=$SYSCONFSEARCHPATH:$LIBSEARCHPATH

dnl Get the operating system and version number...
uname=`uname`
uversion=`uname -r | sed -e '1,$s/[[^0-9]]//g'`
if test x$uname = xIRIX64; then
  uname="IRIX"
fi

if test x$uname = xIRIX; then
  SPOOLSEARCHPATH="/usr/freeware/var/spool:$SPOOLSEARCHPATH"
  LOGSEARCHPATH="/usr/freeware/var/log:$LOGSEARCHPATH"
  SYSCONFSEARCHPATH="/usr/freeware/etc:$SYSCONFSEARCHPATH"
  BINSEARCHPATH="/usr/freeware/bin:$BINSEARCHPATH"
  SBINSEARCHPATH="/usr/freeware/sbin:$BINSEARCHPATH"
  DATASEARCHPATH="/usr/freeware/share:$DATASEARCHPATH"
  LIBSEARCHPATH="/usr/freeware/lib32:/usr/freeware/lib:$LIBSEARCHPATH"
  PTALPIPESSEARCHPATH="/usr/freeware/var/run:$PTALPIPESSEARCHPATH"
  MTINKPIPESSEARCHPATH="/usr/freeware/var/run:$MTINKPIPESSEARCHPATH"
fi

v="cannot open";
AC_PATH_PROG(FILEUTIL,file,FILE_UTILITY_NOT_FOUND,$BSB)dnl
if test "$FILEUTIL" != "FILE_UTILITY_NOT_FOUND" ; then
	v=`${FILEUTIL} - </etc/passwd 2>/dev/null`;
fi
if expr "$v" : ".*cannot open.*" >/dev/null;  then
	echo "Please use a file utility that handles 'file - 'correctly";
	echo "You can obtain one from: ftp://ftp.astron.com/pub/file/";
	echo " or ftp://ftp.lprng.com/pub/LPRng/UNIXTOOLS/file/";
	echo "if necessary, use the FILEUTIL environment variable to specify the path"
	exit 1
fi

AC_PATH_PROG(CAT,cat,CAT_NOT_FOUND,$BSB)
AC_PATH_PROG(GS,gs,GHOSTSCRIPT_NOT_FOUND,$BSB)
AC_PATH_PROG(A2PS,a2ps,A2PS_NOT_FOUND,$BSB)
AC_PATH_PROG(WGET,wget,$BSB)
AC_PATH_PROG(CURL,curl,$BSB)
if test -z "$CURL" -a -z "$CURL" ; then
	AC_MSG_ERROR("cannot find wget and curl.  You need to install at least one");
fi
AC_PATH_PROG(PRINTF,printf,$BSB)dnl

dnl dislabel ghostscript check
AC_MSG_CHECKING(GhostScript check)
AC_ARG_ENABLE( gscheck,
[  --disable-gscheck      disable checking GhostScript for security flaw],
[
if test "$enableval" = "yes" ; then
	GSCHECK=yes;
else
	GSCHECK=no;
fi
],
[
	GSCHECK=no;
],
)
AC_MSG_RESULT($GSCHECK)
AC_SUBST(GSCHECK)

echo GS "$GS"
if test "$GS" != "GHOSTSCRIPT_NOT_FOUND" -a "$GSCHECK" != "no" ; then
  AC_MSG_CHECKING(GhostScript -dSAFER and -dPARANOIDSAFER option)
  if $GS -q -dBATCH -dNOPAUSE -dSAFER -dPARANOIDSAFER -sDEVICE=nullpage -sOutputFile=- $srcdir/gsQuit ; then
    if $GS -q -dBATCH -dNOPAUSE -dSAFER -dPARANOIDSAFER -sDEVICE=nullpage -sOutputFile=- $srcdir/gsTest 1>/dev/null 2>/dev/null ; then
  	  AC_MSG_WARN( [
WARNING: '($GS) -dSAFER -dPARANOIDSAFER ' allows files to be read
Please read README.GhostScriptSecurityProblem
and fix your distribution of GhostScript
OR use:
configure --disable-gscheck
] )
      AC_MSG_ERROR(GhostScript security hole found!)
    else
      AC_MSG_RESULT(no GhostScript security hole found)
    fi
  else
    AC_MSG_ERROR(Error with: $GS -q -dSAFER -dPARANOIDSAFER -sDEVICE=nullpage -sOutputFile=- )
  fi
fi


AC_MSG_CHECKING(how to redirect GhostScript output to fd 3)
if test "$DEVFD3" = "" ; then
	v=`( if test -d /dev/fd ; then echo 3 >/dev/fd/3; fi) 3>&1 1>&2`
	if test "$v" != "" ; then
		DEVFD3=/dev/fd/3
		DEVFD0=/dev/fd/0
	else
		DEVFD3="|$CAT >&3"
		DEVFD0=-
	fi
	if test "$GS" != "GHOSTSCRIPT_NOT_FOUND" -a "$GSCHECK" != "no" ; then
	   rm -f gs_out
	   gs -q -dBATCH -dNOPAUSE -dSAFER -dPARANOIDSAFER -sDEVICE=laserjet "-sOutputFile=$DEVFD3" ${srcdir}/gsPrint 3>gs_out
	   if test "!" -s gs_out ; then
		AC_MSG_ERROR("ERROR - gs cannot handle -sOutputFile=$OUT" )
	   fi
	fi
fi
AC_MSG_RESULT(using $DEVFD3)
AC_SUBST(DEVFD3)
AC_SUBST(DEVFD0)

dnl Check for Perl
AC_PATH_PROGS(PERL,perl,/usr/bin/perl,$BINSEARCHPATH)

dnl Check whether Perl libraries will be found when installed with current
dnl $prefix, if not, try to fall back to /usr/local at first, then to /usr
if test x$prefix = xNONE; then
  PREFIX=$ac_default_prefix
else
  PREFIX=$prefix
fi
echo -n Checking whether Perl will find libraries installed under $PREFIX...
if ( $PERL -V | egrep "^ *$PREFIX/lib(/perl[[0-9]]?|)(/site_perl|)/[[0-9]]" > /dev/null); then
  PERLPREFIX=$PREFIX
  echo " yes"
else
  echo " no"
  echo -n Checking whether Perl will find libraries installed under /usr/local...
  if ( $PERL -V | egrep "^ */usr/local/lib(/perl[[0-9]]?|)(/site_perl|)/[[0-9]]" > /dev/null); then
    PERLPREFIX=/usr/local
    echo " yes"
    echo Installing the Perl libraries with prefix /usr/local.
  else
    PERLPREFIX=/usr
    echo " no"
    echo Installing the Perl libraries with prefix /usr.
  fi
fi
AC_SUBST(PERLPREFIX)

dnl Perl installation directories

dnl This works only with autoconf 2.52 or newer
dnl PERL_INSTALLDIRS=site
dnl AC_ARG_VAR(PERL_INSTALLDIRS, [Where should the Perl libraries be installed (Default: site, Choices: perl, site)?])

dnl This works also with autoconf 2.13
PERL_INSTALLDIRS=site
AC_SUBST(PERL_INSTALLDIRS)

dnl Check for bourne shell
AC_PATH_PROGS(SH,bash sh,/bin/sh,$BINSEARCHPATH)

dnl Paths for LPD and LPRng (without magicfilter)
AC_PATH_DIRS(LPD_SPOOL,lpd,/var/spool/lpd,$SPOOLSEARCHPATH)
AC_PATH_PROGS(LPD_LOG,lp-errs,/var/log/lp-errs,$LOGSEARCHPATH)
AC_PATH_PROGS(LPD_BIN,lpd,/usr/sbin/lpd,$SBINSEARCHPATH)
AC_PATH_PROGS(PRINTCAP,printcap,/etc/printcap,$SYSCONFSEARCHPATH)
AC_PATH_PROGS(LPRNG_CONF,lpd.conf,/etc/lpd.conf,$SYSCONFSEARCHPATH)
AC_PATH_PROGS(LPD_LPR,lpr-lpd lpr-lprng lpr-lpr lpr,/usr/bin/lpr,$BINSEARCHPATH)
AC_PATH_PROGS(LPD_LPQ,lpq-lpd lpq-lprng lpq-lpr lpq,/usr/bin/lpq,$BINSEARCHPATH)
AC_PATH_PROGS(LPD_LPRM,lprm-lpd lprm-lprng lprm-lpr lprm,/usr/bin/lprm,$BINSEARCHPATH)
AC_PATH_PROGS(LPD_LPC,lpc-lpd lpc-lprng lpc-lpr lpc,/usr/sbin/lpc,$SBB)
AC_PATH_PROGS(LPRNG_CHECKPC,checkpc,/usr/sbin/checkpc,$SBINSEARCHPATH)

dnl Paths and other settings for CUPS
AC_PATH_DIRS(CUPS_ETC,cups,/etc/cups,$SYSCONFSEARCHPATH)
AC_PATH_PROGS(CUPS_ADMIN,lpadmin,/usr/sbin/lpadmin,$SBINSEARCHPATH)
AC_PATH_PROGS(CUPS_LPSTAT,lpstat,/usr/bin/lpstat,$BINSEARCHPATH)
AC_PATH_DIRS(CUPS_PPDS,cups/model,/usr/share/cups/model,$DATASEARCHPATH)
AC_PATH_DIRS(CUPS_FILTERS,cups/filter,/usr/lib/cups/filter,$DATASEARCHPATH)
AC_PATH_DIRS(CUPS_BACKENDS,cups/backend,/usr/lib/cups/backend,$DATASEARCHPATH)
AC_PATH_DIRS(LIB_CUPS,cups,/usr/lib/cups,$LIBSEARCHPATH)
AC_PATH_PROGS(CUPS_PCONF,cups/printers.conf,$CUPS_ETC/printers.conf,$SYSCONFSEARCHPATH)
AC_PATH_PROGS(CUPS_LPR,lpr-cups lpr,/usr/bin/lpr,$BINSEARCHPATH)
AC_PATH_PROGS(CUPS_LPQ,lpq-cups lpq,/usr/bin/lpq,$BINSEARCHPATH)
AC_PATH_PROGS(CUPS_LPRM,lprm-cups lprm,/usr/bin/lprm,$BINSEARCHPATH)
AC_PATH_PROGS(CUPS_LPC,lpc-cups lpc,/usr/sbin/lpc,$SBB)
AC_PATH_PROGS(CUPS_LP,lp-cups lp,/usr/bin/lp,$BINSEARCHPATH)
AC_PATH_PROGS(CUPS_CANCEL,cancel-cups cancel,/usr/bin/cancel,$BSB)
AC_PATH_PROGS(CUPS_ENABLE,enable,/usr/bin/enable,$BSB)
AC_PATH_PROGS(CUPS_DISABLE,disable,/usr/bin/disable,$BSB)
AC_PATH_PROGS(CUPS_ACCEPT,accept,/usr/sbin/accept,$SBB)
AC_PATH_PROGS(CUPS_REJECT,reject,/usr/sbin/reject,$SBB)
AC_PATH_PROGS(CUPS_LPMOVE,lpmove,/usr/sbin/lpmove,$SBB)
AC_PATH_PROGS(CUPS_LPOPTIONS,lpoptions,/usr/bin/lpoptions,$BINSEARCHPATH)
AC_PATH_PROGS(CUPS_LPINFO,lpinfo,/usr/sbin/lpinfo,$SBINSEARCHPATH)

dnl Paths for PDQ
AC_PATH_DIRS(PDQ_CONF,pdq,/usr/lib/pdq,$SYS_LIB_PATH)
AC_PATH_PROGS(PDQ_PRINTRC,pdq/printrc,$PDQ_CONF/printrc,$SYS_LIB_PATH)
dnl PDQ_FOOMATIC=$PDQ_CONF/drivers/foomatic
AC_PATH_PROGS(PDQ_PRINT,pdq lpr-pdq,/usr/bin/pdq,$BINSEARCHPATH)
dnl PDQ_JOBDIR=~/.printjobs

dnl Paths for PPR
AC_PATH_PROGS(PPR_PPRD,ppr/bin/pprd,/usr/lib/ppr/bin/pprd,$LIBSEARCHPATH)
AC_PATH_DIRS(PPR_INTERFACES,ppr/interfaces,/usr/lib/ppr/interfaces,$LIBSEARCHPATH)
AC_PATH_DIRS(PPR_PPDFILES,ppr/PPDFiles,/usr/share/ppr/PPDFiles,$DATASEARCHPATH)
AC_PATH_DIRS(PPR_ETC,ppr,/etc/ppr,$SYSCONFSEARCHPATH)
AC_PATH_PROGS(PPR_PPR,ppr lpr-ppr,/usr/bin/ppr,$BINSEARCHPATH)
AC_PATH_PROGS(PPR_PPAD,ppad,/usr/bin/ppad,$BSB)
AC_PATH_PROGS(PPR_PPOP,ppop,/usr/bin/ppop,$BSB)

dnl Paths for direct, spoolerless printing
dnl DIRECT_ETC=/etc/foomatic/direct
dnl DIRECT_CONFIG=$(DIRECT_ETC)/.config

dnl Other paths
AC_PATH_PROGS(NC,nc netcat,/usr/bin/nc,$BSB)
AC_PATH_PROGS(RLPR,rlpr,/usr/bin/rlpr,$BSB)
AC_PATH_PROGS(SMBCLIENT,smbclient,/usr/bin/smbclient,$BSB)
AC_PATH_PROGS(NPRINT,nprint,/usr/bin/nprint,$BSB)
AC_PATH_PROGS(PTAL_CONNECT,ptal-connect,/usr/bin/ptal-connect,$BSB)
AC_PATH_DIRS(PTAL_PIPES,ptal-printd,/var/run/ptal-printd,$PTALPIPESSEARCHPATH)
AC_PATH_DIRS(MTINK_PIPES,mtink,/var/mtink,$MTINKPIPESSEARCHPATH)
AC_PATH_PROGS(GZIP,gzip,/bin/gzip,$BSB)

dnl Path for Foomatic XML database (foomatic-db)
AC_PATH_DIRS(LIBDIR,foomatic/db/source,/usr/share/foomatic/db/source,$DATASEARCHPATH)
LIBDIR=`echo $LIBDIR | sed -e "s:/db/source$::"`
AC_PATH_DIRS(LOCAL_LIBDIR,db/source,../foomatic-db/db/source,.:./foomatic-db:./foomatic-db*:../foomatic-db:../foomatic-db*)
LOCAL_LIBDIR=`echo $LOCAL_LIBDIR | sed -e "s:/db/source$::"`

dnl Path for foomatic-rip (from foomatic-filters)
AC_PATH_PROGS(FOOMATIC_RIP,foomatic-rip,/usr/bin/foomatic-rip,$BSB)
AC_PATH_PROGS(LOCAL_FOOMATIC_RIP,foomatic-rip,../foomatic-filters/foomatic-rip,.:./foomatic-filters:./foomatic-filters*:../foomatic-filters:../foomatic-filters*)

dnl AC_OUTPUT(Makefile src/Makefile)
AC_OUTPUT(Makefile makeDefaults makeMan
foomatic-configure foomatic-printjob foomatic-kitload
foomatic-ppdfile foomatic-preferred-driver foomatic-cleanupdrivers
foomatic-getpjloptions foomatic-addpjloptions
foomatic-compiledb foomatic-fix-xml
foomatic-nonumericalids foomatic-replaceoldprinterids
foomatic-ppd-options foomatic-printermap-to-gimp-print-xml
foomatic-extract-text foomatic-searchprinter
)

dnl Finished
echo "Finished configuring."
echo "Type 'make' to build the package"
echo "then 'make install' to install it."
