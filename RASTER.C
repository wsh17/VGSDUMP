/*
 * module         raster.c V1.2
 * last modified  15:19:51 8/11/89
 * current date   15:20:41 8/11/89
 *
 * 1.2  for QA round 4 of VAX/VGS
 *      no changes
 * 1.1  for QA round 3 of VAX/VGS
 *      base code
 */

#include "dump.h"

#define VMS_PRINT 0
#define VMS_PLOT 1
#define VMS_REMOTE 2
#define NUM_VMS 3
#define BSIZE 32766

static char indent[] = "           ";
static int count[NUM_VMS],start_output[NUM_VMS],end_output[NUM_VMS];

raster(fd,mode,rc_file)
FILE *fd;
char mode,*rc_file;

#ifdef vms

{
  int full_dump = 0;

  switch(mode) {
    case 'F':
      full_dump = 1;
    break;
    case 'P':
      setup(rc_file);
    break;
    case 'S':
    break;
  }
  parser(fd,full_dump);
}

#define ALL_SIZ  83
#define OPT_SIZ   3
#define BUF_SIZ 240

static setup(rc_file)
char *rc_file;
{
  static char *menu_options[] = {
    "VMS_RASTER_PRINT",
    "VMS_RASTER_PLOT",
    "VMS_RASTER_REMOTE",
    "VDS_LOGICAL_BLOCK_HEADERS",
    "VDS_LEVEL_1_COMMANDS",
    "VDS_LEVEL_2_COMMANDS",
    "VRF_LOGICAL_BLOCK_HEADERS",
    "VRF_LEVEL_1_COMMANDS",
    "VRF_BEGIN_PARTITION_COMMANDS",
    "VRF_DRAW_COMMANDS",
    "VRF_MOVE_COMMANDS",
    "VRF_DEFINE_PEN_COMMANDS",
    "VRF_SET_PEN_COMMANDS",
    "VRF_DEFINE_PATTERN_COMMANDS",
    "VRF_DRAW_POLYGON_COMMANDS",
    "VRF_TEXT_STRING_COMMANDS",
    "VRF_SET_FONT_COMMANDS",
    "VRF_SET_FONT_BASE_LINE_COMMANDS",
    "VRF_SKIP_COMMANDS",
    "VRF_DRAW_CIRCLE_COMMANDS",
    "VRF_END_VRF_COMMANDS",
    "VRF_A_SUMMARY_AFTER_EACH_COLOR_PASS",
    "VCGL_INVALID",
    "VCGL_BEGIN_FRAME",
    "VCGL_END_FRAME",
    "VCGL_SKIP_DATA",
    "VCGL_SET_POSTPRINT",
    "VCGL_SET_PREPLOT_CONTROL",
    "VCGL_SET_POSTPLOT_CONTROL",
    "VCGL_SET_PAPER_CONTROL",
    "VCGL_SET_MULTIPLEXER",
    "VCGL_ALLOW_OPAQUE_OBJECTS",
    "VCGL_INITIALIZE_LEVELS",
    "VCGL_INITIALIZE_LEVEL_ORDER",
    "VCGL_END_INITIALIZATION",
    "VCGL_SET_CONTROLLER_OPTIONS",
    "VCGL_SET_PLOTTER_IMAGING",
    "VCGL_DEFINE_DITHER_COLOR",
    "VCGL_DEFINE_RGB_COLOR",
    "VCGL_DEFINE_PATTERN",
    "VCGL_DEFINE_RASTER_STAMP",
    "VCGL_DEFINE_FONT",
    "VCGL_DEFINE_MARKER_SET",
    "VCGL_DEFINE_DASH_STYLE",
    "VCGL_DEFINE_PATTERN_COVERAGES",
    "VCGL_DEFINE_PRIMARY_CONSTANTS",
    "VCGL_DEFINE_MONITOR_CONSTANTS",
    "VCGL_DEFINE_GAMMA_CORRECTION",
    "VCGL_SET_EDGE_VISIBILITY",
    "VCGL_SET_LINE_WIDTH",
    "VCGL_SET_LINE_COLOR",
    "VCGL_SET_DASH_WRAP_STYLE",
    "VCGL_SET_LINE_DASH_INDEX",
    "VCGL_SET_LINE_CAP_STYLE",
    "VCGL_SET_LINE_JOINT_STYLE",
    "VCGL_SET_FILL_COLOR",
    "VCGL_SET_FILL_STYLE",
    "VCGL_SET_LINE_RGB",
    "VCGL_SET_FILL_RGB",
    "VCGL_SET_MARKER_BASELINE",
    "VCGL_SET_TEXT_ORIENTATION",
    "VCGL_SET_TEXT_BASELINE",
    "VCGL_SET_TEXT_ALIGNMENT",
    "VCGL_SET_TEXT_PATH",
    "VCGL_SET_TEXT_SPACING",
    "VCGL_SET_FONT",
    "VCGL_SET_OPAQUE",
    "VCGL_SET_LEVEL",
    "VCGL_SET_CLIP_VIEWPORT",
    "VCGL_DRAW_POLYLINE",
    "VCGL_DRAW_TEXT",
    "VCGL_DRAW_POLYMARKER",
    "VCGL_DRAW_POLYGON",
    "VCGL_DRAW_POLYGON_SET",
    "VCGL_DRAW_RECTANGLE",
    "VCGL_DRAW_CIRCLE",
    "VCGL_DRAW_RASTER_STAMP",
    "VCGL_DRAW_CELL_ARRAY",
    "VCGL_DRAW_RGB_CELL_ARRAY",
    "VCGL_DRAW_RUN_LENGTH",
    "VCGL_DRAW_RGB_RUN_LENGTH",
    "VCGL_DRAW_CMY_RASTER",
    "VCGL_PLOT_LABEL"
  };

  int i,match,start,finish;
  FILE *fp;
  char buf[BUF_SIZ],cmd[BUF_SIZ];

  if ( rc_file == NULL )
    fp = fopen("vgsdump.dat","r");
  else
    fp = fopen(rc_file,"r");

  if (fp != NULL) {
    while (fgets(buf,BUF_SIZ,fp) != NULL) {
      if (sscanf(buf,"%s%d%d",cmd,&start,&finish) > 0 ) {
        if ( *cmd == '*' ) continue;
        match = 0;
        for ( i = 0 ; i < ALL_SIZ ; i++ ) {
          fold_upper(cmd);
          if (strcmp(cmd,menu_options[i]) == 0) {
            match = 1;
            if ( i < OPT_SIZ ) {
              start_output[i] = start;
              end_output[i] = finish;
            }
          }
        }
        if ( match == 0 )
          printf("vgsdump: %s: unknown option in parameter file\n",cmd);
      }
    }
    fclose(fp);
  } else {
    printf("vgsdump: error opening parameter file\n");
    exit(1);
  }
}

static fold_upper(buf)
char *buf;
{
  int i;

  for ( i = 0 ; i < strlen(buf) ; i++ )
    if ((buf[i] >= 'a') && (buf[i] <= 'z' ))
      buf[i] += 'A' -'a';
}

static parser(fd,full_dump)
FILE *fd;
int full_dump;
{
  char buf[BSIZE];
  int cc,record = 0,offset = 0;

  for (;;) {
    cc = get_buf(fd,buf);
    record++;

    switch(buf[0]&0xff) {
      case 0x11:
        count[VMS_PRINT]++;
        if ( full_dump || ck(VMS_PRINT) ) {
          printf("\nbyte 0x%X, record %d\n",offset,record);
          printf("%s%-9dTRANSPARENT PRINT\n",indent,count[VMS_PRINT]);
          printf("%srecord length = %d\n",indent,cc);
          print_pass(buf,cc);
        }
      break;
      case 0x12:
        count[VMS_PLOT]++;
        if ( full_dump || ck(VMS_PLOT) ) {
          printf("\nbyte 0x%X, record %d\n",offset,record);
          printf("%s%-9dTRANSPARENT PLOT\n",indent,count[VMS_PLOT]);
          printf("%srecord length = %d\n",indent,cc);
        }
      break;
      case 0x13:
        count[VMS_REMOTE]++;
        if ( full_dump || ck(VMS_REMOTE) ) {
          printf("\nbyte 0x%X, record %d\n",offset,record);
          printf("%s%-9dREMOTE FUNCTION\n",indent,count[VMS_REMOTE]);
          printf("%srecord length = %d\n",indent,cc);
          remote(buf[1]&0xff);
        }
      break;
      case 0x91:
        count[VMS_PRINT]++;
        if ( full_dump || ck(VMS_PRINT) ) {
          printf("\nbyte 0x%X, record %d\n",offset,record);
          printf("%s%-9dTRANSPARENT PRINT\n",indent,count[VMS_PRINT]);
          printf("%srecord length = %d\n",indent,cc);
          printf("%sswap bytes\n",indent);
          print_pass(buf,cc);
        }
      break;
      case 0x92:
        count[VMS_PLOT]++;
        if ( full_dump || ck(VMS_PLOT) ) {
          printf("\nbyte 0x%X, record %d\n",offset,record);
          printf("%s%-9dTRANSPARENT PLOT\n",indent,count[VMS_PLOT]);
          printf("%srecord length = %d\n",indent,cc);
          printf("%sswap bytes\n",indent);
        }
      break;
      case 0x93:
        count[VMS_REMOTE]++;
        if ( full_dump || ck(VMS_REMOTE) ) {
          printf("\nbyte 0x%X, record %d\n",offset,record);
          printf("%s%-9dREMOTE FUNCTION\n",indent,count[VMS_REMOTE]);
          printf("%srecord length = %d\n",indent,cc);
          printf("%sswap bytes\n",indent);
          remote(buf[1]&0xff);
        }
      break;
      default:
        printf("\nbyte 0x%X, record %d\n",offset,record);
        printf("%sWARNING: expecting VMS spooler control code\n",indent);
        printf("%srecord length = %d\n",indent,cc);
      break;
    }
    offset += cc;
  }
}

static print_pass(buf,len)
char *buf;
int len;
{
  int next = 2, tmp = 0, tmp_local, tmp_cr;

  while ( next < len ) {
    if ( tmp == 0 ) {
      tmp = ((buf[next]&0xff) << 8) + (buf[next+1]&0xff);
      next += 2;
      tmp_local = 1;
    } else {
      switch(tmp) {
        case 0x9B42: /* ESC B */
          tmp_cr = 0;
          tmp_local = ((buf[next]&0xff) << 8) + (buf[next+1]&0xff);
          next += 2;
          printf("%sremote label, ",indent);
          printf("byte count = %d\n",tmp_local);
          while (tmp_local) {
            if ( (tmp_cr%10) == 0 ) printf("%s",indent);
            printf("0x%X ",buf[next]);
            next++;
            tmp_local--;
            tmp_cr++;
            if ( (tmp_cr%10) == 0 ) printf("\n");
          } 
          tmp = 0;
          if ( (tmp_cr%10) != 0 ) printf("\n");
        break;
        case 0x9B43: /* ESC C */
          printf("%sremote clear ",indent);
          printf("buffer command\n"); 
          next += 2;
          tmp = 0;
        break;
        case 0x9B45: /* ESC E */
          printf("%sremote line ",indent);
          printf("enhance command\n"); 
          next += 2;
          tmp = 0;
        break;
        case 0x9B46: /* ESC F */
          tmp_local = ((buf[next]&0xff) << 8) + (buf[next+1]&0xff);
          next += 2;
          printf("%sremote font load, ",indent);
          printf("byte count = %d\n",tmp_local);
          next += tmp_local;
        break;
        case 0x9B47: /* ESC G */
          printf("%sremote set ",indent);
          printf("page mode command\n"); 
          next += 2;
          tmp = 0;
        break;
        case 0x9B48: /* ESC H */
          switch(tmp_local) {
            case 1:
              next += 2;
              tmp_local++;
            break;
            case 2:
              tmp_local = ((buf[next]&0xff) << 8) + (buf[next+1]&0xff);
              printf("%sremote set ",indent);
              printf("character height command\n"); 
              printf("height = %d scans\n",tmp_local);
              tmp = 0;
            break;
          }
        break;
        case 0x9B49: /* ESC I */
          printf("%sremote inverse ",indent);
          printf("image command\n"); 
          next += 2;
          tmp = 0;
        break;
        case 0x9B4B: /* ESC K */
          printf("%sremote cut ",indent);
          printf("immediate command\n"); 
          next += 2;
          tmp = 0;
        break;
        case 0x9B4D: /* ESC M */
          printf("%sremote mirror ",indent);
          printf("image command\n"); 
          next += 2;
          tmp = 0;
        break;
        case 0x9B50: /* ESC P */
          switch(tmp_local) {
            case 1:
            case 2:
              next += 2;
              tmp_local++;
            break;
            case 3:
              tmp_local = buf[next]&0xff;
              next++;
              tmp = 0;
              printf("%scolor plotter preamble command\n",indent);
              switch(tmp_local&0x0c) {
                case 0x00:
                  printf("%ssingle pass plot\n",indent);
                break;
                case 0x04:
                  printf("%sfirst pass of multi-pass ",indent);
                  printf("color plot\n");
                break;
                case 0x08:
                  printf("%sintermediate or last pass for ",indent);
                  printf("multi-pass color plot\n");
                break;
                case 0x0c:
                  printf("%sillegal - bits (2-3) set\n",indent);
                break;
              }
              switch(tmp_local&0x30) {
                case 0x00:
                  printf("%snot last pass\n",indent);
                break;
                default:
                  printf("%slast pass\n",indent);
                break; 
              }
              switch(tmp_local&0x40) {
                case 0x00:
                  printf("%suse color plotting width\n",indent);
                break;
                case 0x40:
                  printf("%suse full plotting width\n",indent);
                break;
              }
              switch(tmp_local&0x80) {
                case 0x00:
                  printf("%sdo not merge tick marks\n",indent);
                break;
                case 0x80:
                  printf("%smerge tick marks\n",indent);
                break;
              }
              switch(tmp_local&0x03) {
                case 0x00:
                  printf("%sblack pass\n",indent);
                break;
                case 0x01:
                  printf("%scyan pass\n",indent);
                break;
                case 0x02:
                  printf("%smagenta pass\n",indent);
                break;
                case 0x03:
                  printf("%syellow pass\n",indent);
                break;
              }
            break;
          }
        break;
        case 0x9B52: /* ESC R */
          switch(tmp_local) {
            case 1:
              next += 2;
              tmp_local++;
            break;
            case 2:
              tmp_local = ((buf[next]&0xff) << 8) + (buf[next+1]&0xff);
              tmp = 0;
              printf("%sremote rdt ",indent);
              printf("command: "); 
              switch(tmp_local) {
                case 0:
                  printf("half density\n");
                break;
                case 1:
                  printf("1 X 1 enlargement\n");
                break;
                case 2:
                  printf("2 X 2 enlargement\n");
                break;
                case 3:
                  printf("3 X 3 enlargement\n");
                break;
                case 4:
                  printf("4 X 2 enlargement\n");  
                break;
                default:
                  printf("invalid data word\n");
                break;
              }
            break;
          }
        break;     
        case 0x9B53: /* ESC S */
          switch(tmp_local) {
            case 1:
              next += 2;
              tmp_local++;
            break;
            case 2:
              tmp_local = ((buf[next]&0xff) << 8) + (buf[next+1]&0xff);
              tmp = 0;
              printf("%sremote speed of ",indent);
              switch(tmp_local) {
                case 0:
                  printf("0.125 ips\n");
                break;
                case 1:
                  printf("0.25 ips\n");
                break;
                case 2:
                  printf("0.5 ips\n");
                break;
                case 3:
                  printf("0.75 ips\n");
                break;
                case 4:
                  printf("1.0 ips\n");
                break;
                case 5:
                  printf("1.25 ips\n");
                break;
                case 6:
                  printf("1.50 ips\n");
                break;
                case 7:
                  printf("1.75 ips\n");
                break;
                case 8:
                  printf("2.0 ips\n");
                break;
                case 0xff:
                  printf("full speed\n");
                break;
              }
            break;
          }
        break;
        case 0x9B54: /* ESC T */
          switch(tmp_local) {
            case 1:
              next += 2;
              tmp_local++;
            break;
            case 2:
              tmp_local = ((buf[next]&0xff) << 8) + (buf[next+1]&0xff);
              tmp = 0;
              printf("%sremote select ",indent);
              printf("plot mode command, scan lines = ");
              printf("%d\n",tmp_local);
            break;
          }
        break;
        case 0x9B57: /* ESC W */
          printf("%sremote color plotter ",indent);
          printf("rewind command\n"); 
          next += 2;
          tmp = 0;
        break;
        case 0x9B3C: /* ESC < */
          printf("%sremote enable ",indent);
          printf("ff and eot\n"); 
          next += 2;
          tmp = 0;
        break;
        case 0x9B3E: /* ESC > */
          printf("%sremote disable ",indent);
          printf("ff and eot\n"); 
          next += 2;
          tmp = 0;
        break;
        default:
          printf("%sunknown print pass all remote command\n",indent);
          next = len;
        break;
      }
    }
  }
}

static remote(cmd)
int cmd;
{
  switch(cmd) {
    case 1:
      printf("%sremote line terminate\n",indent);
    break;
    case 2:
      printf("%sremote buffer clear\n",indent);
    break;
    case 3:
      printf("%sremote reset\n",indent);
    break;
    case 4:
      printf("%sremote form feed\n",indent);
    break;
    case 5:
      printf("%sremote end of transmission\n",indent);
    break;
    default:
      printf("%sinvalid remote function\n",indent);
    break;
  }
}

static summary()
{
  printf("\nSummary\n");
  printf(" %9d REMOTE FUNCTION\n",count[VMS_REMOTE]);
  printf(" %9d TRANSPARENT PLOT\n",count[VMS_PLOT]);
  printf(" %9d TRANSPARENT PRINT\n",count[VMS_PRINT]);
  exit();
}

static int get_buf(fd,buf)
FILE *fd;
char *buf;
{
  int cc;

  if ((cc = F_READ(buf,1,BSIZE,fd)) < 1 ) summary();

  switch(((buf[0]&0xff) << 8) + (buf[1]&0xff)) {
    case 0x0091:
      swap(buf,2);
    break;
    case 0x9100:
      swap(buf,cc);
      swap(buf,2);
    break;
  }
  return(cc);
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

static int ck(option)
int option;
{
  return((start_output[option] <= count[option]) &&
         (count[option] <= end_output[option]));
}

#else

{
}

#endif
