
/*
 * module         dump.h V1.2
 * last modified  15:19:48 8/11/89
 * current date   15:20:40 8/11/89
 *
 * 1.2  for QA round 4 of VAX/VGS
 *      no changes
 * 1.1  for QA round 3 of VAX/VGS
 *      base code
 */

/*****************************************************************************
  Macro definitions.
*****************************************************************************/

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define unix
/*****************************************************************************
 system dependent include files
   standard i/o package
   setjmp/longjmp non-local goto
*****************************************************************************/

#if defined(vms)
#include stdio
#include unixio
#include file
#include setjmp
#endif

#if defined(unix) || defined(XENIX)
#include <stdio.h>
#include <sys\types.h>
/* #include <sys\file.h> */
#include <fcntl.h>
#include <setjmp.h>
#endif

#if defined(ibm)
#include <stdefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#endif

/*****************************************************************************
 system dependent i/o routines
   fopen,fread,fclose,fseek - standard i/o package, proposed ansi C standard
   openf,readf,closef,seekf - unix i/o package, ieee posix 1003.1 standard
*****************************************************************************/

#if defined(vms)
#define F_OPEN openf
#define F_READ readf
#define F_CLOSE closef
#define F_SEEK seekf
#define F_TYPE openf
#endif

#if defined(unix) || defined(XENIX)
#define F_OPEN openf
#define F_READ readf
#define F_CLOSE closef
#define F_SEEK seekf
#define F_TYPE openf
#endif

#if defined(ibm)
#define F_OPEN(x,y) fopen(x,"rb, recfm=f, lrecl=132")
#define F_READ fread
#define F_CLOSE fclose
#define F_SEEK fseek
#define F_TYPE fopen
#endif

/*****************************************************************************
 system dependent variable argument list routines
   varargs - current unix standard
   stdarg - proposed ansi c standard
*****************************************************************************/

#if defined(vms)
#include varargs
#define VARARGS
#endif

#if defined(unix) || defined(XENIX)
#include <varargs.h>
#define VARARGS
#endif

#if defined(ibm)
#include <stdarg.h>
#define STDARG
#endif

/*****************************************************************************
 system dependent variable argument print routines
   vfprintf - int vfprintf(FILE *stream, char *fmt, va_list *args)
   doprnt - _doprnt(char *fmt, va_list *args, FILE *stream)
*****************************************************************************/

#if defined(vms) || defined(ibm)
#define VFPRINTF(x,y,z) vfprintf(x,y,z)
#endif

#if defined(sun) || defined(ultrix) || defined(XENIX)
#define VFPRINTF(x,y,z) vfprintf(x,y,z)
#endif

#if defined(bsd4_3)
#define VFPRINTF(x,y,z) _doprnt(y,z,x)
#endif

