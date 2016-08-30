
/*
 * module         dump.c V1.2 - Created by Bill Harper @ Versatec 1986
 * last modified  15:19:45 8/11/89
 * current date   15:20:40 8/11/89
 *
 * 1.2  for QA round 4 of VAX/VGS
 *      no changes
 * 1.1  for QA round 3 of VAX/VGS
 *      base code
 */

#include "dump.h"

#define START 'S'
#define RAS1D '1'
#define RAS2D '2'
#define RAS3D '3'
#define VRF16 '4'
#define VRF32 '5'
#define VCGL  '6'
#define BKRAS '7'
#define VECTR '8'
#define ERROR 'E'

#define BSIZE 32766
#define MSIZE 80

static int vflag = 0, vms_spooled = 0;
static char version[] = "vgsdump V1.2 8/11/89",mesg[MSIZE];
static FILE *fd = NULL;

main(argc,argv)
int argc;
char **argv;
{
  int error_flag = 0, data_file = 0;
  char mode = 'S', *rc_file, *filename;
  FILE *F_TYPE();

  while ( --argc > 0 ) {
    argv++;
    if ((*argv)[0] == '-') {
      switch ((*argv)[1] ) {
        case 'F':
        case 'f':
          mode = 'F';
        break;
        case 'S':
        case 's':
          mode = 'S';
        break;
        case 'P':
        case 'p':
          mode = 'P';
          if ((*argv)[2] == '\0')
            rc_file = NULL;
          else
            rc_file = *argv;
        break;
        case 'V':
        case 'v':
          vflag++;
        break;
        default:
          error_flag = 1;
        break;
      }
    } else {
      data_file = 1;
      filename = *argv;
    }
  }

  if (data_file)
    if ((fd = F_OPEN(filename,"r")) == NULL) {
      printf("vgsdump: error opening %s\n",filename);
      exit(1);
    }

  if ((data_file == 0) || (fd == NULL) || (error_flag == 1)) {
    printf("usage: vgsdump data_file ");
    printf("[ -f | -p[parameter_file] | -s ] [ -v ]\n");
  } else {
    dump(filename,mode,rc_file);
    F_CLOSE(fd);
  }
}


static dump(filename,mode,rc_file)
char *filename, mode, *rc_file;
{
  char cmd,file_type();

  if (rc_file != NULL) rc_file += 2;
  cmd = file_type();
  header(filename,cmd,mode,rc_file);
  F_SEEK(fd,0L,0);

  switch(cmd) {
    case VCGL:
      vcgl(fd,mode,rc_file);
    break;
    case VRF16:
    case VRF32:
      vrf(fd,mode,rc_file);
    break;
    case RAS1D:
    case RAS2D:
    case RAS3D:
    case BKRAS:
    case VECTR:
      vds(fd,mode,rc_file);
    break;
    case ERROR:
      if ( vms_spooled ) raster(fd,mode,rc_file);
    break;
  }
}

static header(filename,cmd,mode,rc_file)
char *filename,cmd,mode,*rc_file;
{
  printf("dump of %s\n",filename);
  if ( vflag ) printf("version: %s\n",version);

  switch(cmd) {
    case VCGL:
      printf("data format: VCGL\n");
    break;
    case VRF16:
      printf("data format: VRF 16-bit\n");
    break;
    case VRF32:
      printf("data format: VRF 32-bit\n");
    break;
    case RAS1D:
      printf("data format: VDS One-Dimensional Compacted Raster\n");
    break;
    case RAS2D:
      printf("data format: VDS Two-Dimensional Compacted Raster\n");
    break;
    case RAS3D:
      printf("data format: VDS Optimized Compacted Raster\n");
    break;
    case BKRAS:
      printf("data format: VDS Blocked Raster\n");
    break;
    case VECTR:
      printf("data format: VDS Ordered Vector\n");
    break;
    case ERROR:
      if ( vms_spooled ) {
        printf("data format: VMS spooled raster\n");
      } else {
        printf("vgsdump: %s\n",mesg);
        exit(1);
      }
    break;
  }

  switch(mode) {
    case 'F':
      printf("output level: full dump\n");
    break;
    case 'P':
      printf("output level: partial dump\n");
    break;
    case 'S':
      printf("output level: short dump\n");
    break;
  }
  
  if (mode == 'P')
    if (rc_file != NULL)  
      printf("parameter file: %s\n",rc_file);
    else
      printf("parameter file: vgsdump.dat (default)\n");

  if (vms_spooled) printf("data type: VMS spooler control codes\n");
}


static char file_type()
{
  char type();

  return(type(START));
}

static char type(cmd)
char cmd;
{
  static jmp_buf env;
  static int mode;

  mode = cmd;

  if (mode == START)
    setjmp(env);
  else
    longjmp(env,0);

  if (mode == START) parser();

  return(mode);
} 

static get_frame()
{
  static int frame_sync[8] = {
    0x18, 0x01, 0x00, 0x00,
    0x18, 0x02, 0x00, 0x00
  };
  int i;

  for ( i = 0 ; i < 8 ; i++)
    if ( get_byte() != frame_sync[i] ) {
      sprintf(mesg,"expecting frame sync header");
      type(ERROR);
    }
}

static int get_header()
{
  int bytcnt;

  if ( get_byte() != 0x18 ) {
    sprintf(mesg,"expecting logical block header");
    type(ERROR);
  }

  switch(get_byte()) {
    case 0x01:
      if ( get_word() != 0 ) {
        sprintf(mesg,"expecting zero byte count with frame sync header");
        type(ERROR);
      }
      if ( get_word() != 0x1802 ) {
        sprintf(mesg,"expecting 0x1802 with frame sync header");
        type(ERROR);
      }
      if ( get_word() != 0 ) {
        sprintf(mesg,"expecting zero byte count with frame sync header");
        type(ERROR);
      }
      bytcnt = get_header();
    break;
    case 0x02:
      bytcnt = get_word();
      if ( bytcnt == 0xffff ) type(VCGL);
      if ( bytcnt > 32766 ) {
        sprintf(mesg,"expecting byte count <= 32766 with cont header");
        type(ERROR);
      }
    break;
    case 0x03:
      if ( get_word() != 0 ) {
        sprintf(mesg,"expecting zero byte count with end of frame header");
        type(ERROR);
      }
      bytcnt = -1;
    break;
    default:
      sprintf(mesg,"expecting logical block header");
      type(ERROR);
    break;
  }
  return(bytcnt);
}

static parser()
{
  int bytcnt = -1,knt = 0;
  int tmp;

  for (;;) {

    while ( bytcnt == -1 ) {
      get_frame();
      bytcnt = get_header();
    }
  
    if ( bytcnt > 0 ) {
      switch(knt&0xff) {
        case 0:
          knt = get_word();
          bytcnt -= 2;
        default:
          switch(knt&0xff00) {
            case 0x8000:
              type(VECTR);
            break; 
            case 0x8100:
              switch(knt&0x00ff) {
                case 0:
                  type(RAS1D);
                break;
                case 1:
                  type(RAS2D);
                break;
                case 2:
                  type(RAS3D);
                break;
              }    
            break;
            case 0x8200:
              type(BKRAS);
            break;
            case 0x8300:
              type(VRF16);
            break;
            case 0x8400: 
              type(VRF32);
            break;
            case 0xc500:
            case 0xc600:
            case 0xcc00:
            case 0xc000:
              tmp = MIN(knt&0xff,bytcnt);
              knt -= tmp; 
              bytcnt -= tmp;
              for (; tmp ; tmp--) get_byte();
            break;
            case 0xc200:
            case 0xc300:
            case 0xc400:
            break;
            default:
              sprintf(mesg,"expecting level one command");
              type(ERROR);
            break;
          } 
        break;
      }
    }
    if ( bytcnt == 0 ) bytcnt = get_header();
  }
}  

static int get_byte()
{
  static char buf[BSIZE];
  static int next = -1, length = -1,swap_flag = 0;
  static int unlocked = 1, swap_correct;

  if ( next == length ) {
    if ((length = F_READ(buf,1,BSIZE,fd)) < 1) {
      sprintf(mesg,"unexpected read error");
      type(ERROR);
    }
    if (unlocked) {
      unlocked = 0;
      switch( ((buf[0]&0xff) << 8) + (buf[1]&0xff) ) {

#ifdef vms

        case 0x0113:
        case 0x0213:
        case 0x0313:
        case 0x0413:
        case 0x0513:
        case 0x0193:
        case 0x0293:
        case 0x0393:
        case 0x0493:
        case 0x0593:
          if ( length == 2 ) {
            swap_correct = 1;
            printf("warning: incorrect byte swapping in data format\n");
            vms_spooled++;
          }
        break;

        case 0x1301:
        case 0x1302:
        case 0x1303:
        case 0x1304:
        case 0x1305:
        case 0x9301:
        case 0x9302:
        case 0x9303:
        case 0x9304:
        case 0x9305:
          if ( length == 2 ) vms_spooled++;
        break;

        case 0x0011:
        case 0x0012:
        case 0x0091:
        case 0x0092:
          vms_spooled++;
          swap_correct = 1;
          printf("warning: incorrect byte swapping in data format\n");
        break;

        case 0x1100:
        case 0x1200:
        case 0x9100:
        case 0x9200:
          vms_spooled++;
        break;
#endif
        case 0x0118:
          swap_correct = 1;
          printf("warning: incorrect byte swapping in data format\n");
        break;
        default:
          swap_correct = 0;
        break;
      }
    }
    next = 0;
    if ( vms_spooled ) {
      if ( swap_correct == 0 ) {
        switch( ((buf[0]&0xff) << 8) + (buf[1]&0xff) ) {
          case 0x9100:
            swap_flag = 1;
            next += 2;
          break;
          case 0x1100:
            swap_flag = 0;
            next += 2;
          break;
          case 0x1301:
          case 0x1302:
          case 0x1303:
          case 0x1304:
          case 0x1305:
          case 0x9301:
          case 0x9302:
          case 0x9303:
          case 0x9304:
          case 0x9305:
            if ( length == 2 ) next += 2;
          break;
          case 0x1200:
            swap_flag = 0;
            next += 2;
          break;
          case 0x9200:
            swap_flag = 1;
            next += 2;
          break;
          default:
          break;
        }
      } else {
        switch( ((buf[0]&0xff) << 8) + (buf[1]&0xff) ) {
          case 0x0091:
            swap_flag = 1;
            next += 2;
          break;
          case 0x0011:
            swap_flag = 0;
            next += 2;
          break;
          case 0x0113:
          case 0x0213:
          case 0x0313:
          case 0x0413:
          case 0x0513:
          case 0x0193:
          case 0x0293:
          case 0x0393:
          case 0x0493:
          case 0x0593:
            if ( length == 2 ) next += 2;
          break;
          case 0x0012:
            swap_flag = 0;
            next += 2;
          break;
          case 0x0092:
            swap_flag = 1;
            next += 2;
          break;
          default:
          break;
        }
      }
    }
    if ( swap_flag || swap_correct )
      if ( swap_flag != swap_correct) swap(buf,length); 
  }
  return((buf[next++])&0xff);
}

static int get_word()
{
  int tmp;

  tmp = get_byte();
  tmp = (tmp << 8) + get_byte();
  return(tmp);
}

static swap(buf,cc)
char *buf;
int cc;
{
  int i;
  char tmp;

  for ( i = 0 ; i < cc ; i += 2 ) {
    tmp = buf[i];
    buf[i] = buf[i+1];
    buf[i+1] = tmp;
  }
}
