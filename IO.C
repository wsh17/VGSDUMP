
/*
 * module         io.c V1.2
 * last modified  15:19:49 8/11/89
 * current date   15:20:41 8/11/89
 *
 * 1.2  for QA round 4 of VAX/VGS
 *      no changes
 * 1.1  for QA round 3 of VAX/VGS
 *      base code
 */

#include "dump.h"

#ifdef vms
#define OPEN(x,y,z) open(x,y,z,"rfm=var,ctx=rec")
#else
#define OPEN(x,y,z) open(x,y,z)
#endif

static int fd;

FILE *openf(filename,mode)
char *filename,*mode;
{
  fd = OPEN(filename,O_RDONLY,0644);
 
  if ( fd == -1 ) 
    return((FILE *) NULL);
  else
    return((FILE *) 1);
}

int readf(ptr,item,nitem,fp)
char *ptr;
int item,nitem;
FILE *fp;
{
  int cc;

  cc = read(fd,ptr,item*nitem);
  if ( cc == -1 ) cc = 0;
  return(cc);
}

closef(fp)
FILE *fp;
{
   close(fd);
}

seekf(fp,offset,pos)
FILE *fp;
long offset;
int pos;
{
  lseek(fd,offset,pos);
}
