/*
 * module         vcgl.c V1.2
 * last modified  15:19:54 8/11/89
 * current date   15:20:41 8/11/89
 *
 * 1.2  for QA round 4 of VAX/VGS
 *      no changes
 * 1.1  for QA round 3 of VAX/VGS
 *      base code
 */

#include "dump.h"

/* pdefs.h   --  Defines Indices for counting each VCGL command        */
/* (C) Copyright 1988 by Versatec, Inc.  All Rights Reserved.          */

#define  P_INVALID                    1
#define  P_BEGIN_FRAME                2
#define  P_END_FRAME                  3
#define  P_SKIP_DATA                  4
#define  P_SET_POSTPRINT              5
#define  P_SET_PREPLOT                6
#define  P_SET_POSTPLOT               7
#define  P_SET_PAPER_CONTROL          8
#define  P_SET_MULTIPLEXER            9
#define  P_ALLOW_OPAQUE              10
#define  P_INITIALIZE_LEVELS         11
#define  P_LEVEL_ORDER               12
#define  P_END_INIT                  13
#define  P_SET_CNTRL_OPTIONS         14
#define  P_SET_PLOTTER_IMAGING       15
#define  P_DEFINE_DITHER             16
#define  P_DEFINE_RGB                17
#define  P_DEFINE_PATTERN            18
#define  P_DEFINE_RASTER_STAMP       19
#define  P_DEFINE_FONT               20
#define  P_DEFINE_MARKER             21
#define  P_DEFINE_DASH               22
#define  P_DEFINE_COVERAGES          23
#define  P_DEFINE_PRIMARY_CONSTANTS  24
#define  P_DEFINE_MONITOR_CONSTANTS  25
#define  P_DEFINE_GAMMA_CORRECTION   26
#define  P_SET_EDGE_VISIBILITY       27
#define  P_SET_LINE_WIDTH            28
#define  P_SET_LINE_COLOR            29
#define  P_SET_DASH_WRAP_STYLE       30
#define  P_SET_LINE_DASH_INDEX       31
#define  P_SET_LINE_CAP_STYLE        32
#define  P_SET_LINE_JOINT_STYLE      33
#define  P_SET_FILL_COLOR            34
#define  P_SET_FILL_STYLE            35
#define  P_SET_LINE_RGB              36
#define  P_SET_FILL_RGB              37
#define  P_SET_MARKER_BASELINE       38
#define  P_SET_TEXT_ORIENTATION      39
#define  P_SET_TEXT_BASELINE         40
#define  P_SET_TEXT_ALIGNMENT        41
#define  P_SET_TEXT_PATH             42
#define  P_SET_TEXT_SPACING          43
#define  P_SET_FONT                  44
#define  P_SET_OPAQUE                45
#define  P_SET_LEVEL                 46
#define  P_SET_CLIP_VIEWPORT         47
#define  P_DRAW_POLYLINE             48
#define  P_DRAW_TEXT                 49
#define  P_DRAW_POLYMARKER           50
#define  P_DRAW_POLYGON              51
#define  P_DRAW_POLYGON_SET          52
#define  P_DRAW_RECTANGLE            53
#define  P_DRAW_CIRCLE               54
#define  P_DRAW_RASTER_STAMP         55
#define  P_DRAW_CELL_ARRAY           56
#define  P_DRAW_RGB_CELL_ARRAY       57
#define  P_DRAW_RUN_LENGTH           58
#define  P_DRAW_RGB_RUN_LENGTH       59
#define  P_DRAW_CMY_RASTER           60
#define  P_PLOT_LABEL                61

/* extern.h   -- Defines variables and functions used between routines       */
/* (C) Copyright 1988 by Versatec, Inc.  All Rights Reserved.                */

#define  DEBUG_LEVEL                  0  /* 1 = take input from OUTPUT.VCGL  */
                                         /* 5 = full debug output            */
#define  TRUE                         1
#define  FALSE                        0
#define  MAX_ERRORS                   5  /* Abort processing after count     */
#define  NUM_VCGL_COMMANDS          256  /* Array size of 0xff               */
#define  VCGL_DUMP_RC   ".vcgl_dump_rc"
#define  NAMESIZE                    80
#define  BYTES_DUMPED               100  /* Number of data bytes dumped by   */
                                         /* dump_data in vcgl_ascii          */

static short           type;             /* Index for X,Y,Parm resolution    */
static unsigned short  last_vcgl_cmd;    /* Current VCGL command             */
static long            val_words;        /* Number of words in current cmd   */
static long            err_count;        /* Counts number of errors          */
static short           odd_byte_flag;    /* Flags that an odd byte remains   */
static short           max_priority_level;
                                         /* Set by Initialize Levels cmd     */
static short           eof_found;        /* Input end of file flagword       */
static long            count[NUM_VCGL_COMMANDS];
static long            count_old[NUM_VCGL_COMMANDS];
                                         /* Counts VCGL commands by type     */
static long            start_output[NUM_VCGL_COMMANDS];
                                         /* Start outputting at this count   */
static long            end_output[NUM_VCGL_COMMANDS];
                                         /* End outputting at this count     */
static short           end_frame_found;  /* End Frame found flagword         */
static short           frame_sync_found; /* Frame Sync found flagword        */
static short           debug_level;      /* Debug level for debug output     */
static char            sbuf[NAMESIZE];   /* Used to hold byte offset string  */
static short           full_output;      /* Flag word for full output msgs   */
static long            byte_offset;      /* Byte offset into input file      */
static long            cmd_byte_offset;  /* Byte offset to current vcgl cmd  */
static short           print_sum;        /* Logical print summary flag       */

/* Function Prototypes Follow */
void            get_init();
short           get_short();
long            get_long();
long            get_val();
unsigned short  get_cmd();
char            get_byte();
long            get_x_value();
long            get_y_value();
long            get_rel_value();

dump_data();

/* ************************************************************************* */
/* vcgl_dump.c  -- VCGL interpretive dump utility                            */
/*                                                                           */
/* The code was written by:                                                  */
/*                           Brian Jones                                     */
/*                           VerDak Graphics Software & Systems              */
/*                           versatc!brian                                   */
/*                           October 25, 1988                                */
/*                                                                           */
/* Copyright c1988, Versatec Inc., a Xerox Company, Santa Clara California.  */
/*                                                                           */
/* ************************************************************************* */
#define  _IN_MAIN
static FILE *fp;
char *getenv();

/* sccs_dump[] = vcgl_dump.c 1.14 12/9/88 */

/* This is a utility that reads VCGL data from the standard input stream     */
/* and interpretively dumps it to the standard output stream.                */
/* It can be used in conjunction with the unix utility more to search        */
/* for certain VCGL commands.  Byte offsets are given for each command       */
/* Commands can be filtered by setting the start counts and end counts       */
/* in a .vcgl_dump_rc file in the user's home directory.                     */

#define ALL_SIZ  83
#define OPT_SIZ  61
#define BUF_SIZ 240

vcgl(ifile,mode,rc_file)
FILE *ifile;
char mode,*rc_file;
{
  int i,match,start,finish;            /* for option-matching              */
  char buf[BUF_SIZ],cmd[BUF_SIZ];      /* buffers for getting a line text  */

  static char *menu_options[] = {
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
    "VCGL_PLOT_LABEL",
    "VDS_LOGICAL_BLOCK_HEADERS",
    "VDS_LEVEL_1_COMMANDS",
    "VDS_LEVEL_2_COMMANDS",
    "VMS_RASTER_PRINT",
    "VMS_RASTER_PLOT",
    "VMS_RASTER_REMOTE",
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
    "VRF_A_SUMMARY_AFTER_EACH_COLOR_PASS"
};

  debug_level = DEBUG_LEVEL;
  full_output = (mode == 'S' ? FALSE : TRUE );
  get_init(ifile);              /* Initialize Input Variables */

  if (mode == 'P') {
    for ( i = 0 ; i < NUM_VCGL_COMMANDS ; i++ ) end_output[i] = 0L;

    if (rc_file == NULL)
      fp = fopen("vgsdump.dat","r");
    else
      fp = fopen(rc_file, "r");

    if ( fp == NULL ) {
      printf("vgsdump: error opening parameter file\n");
      exit(1);
    }

    while (fgets(buf,BUF_SIZ,fp) != NULL) {
      if (sscanf(buf,"%s%d%d",cmd,&start,&finish) > 0 ) {
        if ( *cmd == '*' ) continue;
        match = 0;
        for ( i = 0 ; i < ALL_SIZ ; i++ ) {
          fold_upper(cmd);
          if (strcmp(cmd,menu_options[i]) == 0) {
            match = 1;
            if ( i < OPT_SIZ ) {
              start_output[i+1] = start;
              end_output[i+1] = finish;
            }
          }
        }
        if ( match == 0 )
          printf("vgsdump: %s: unknown option in parameter file\n",cmd);
      }
    }
    fclose(fp);
  }

  /* Pass Control to the VCGL Switch Function */

  vcglsw();                     /* Begin Switching on VCGL Data */
}

static summary()
{
  int i;
  static int plot_number = 1;

  for ( i = 0 ; i < NUM_VCGL_COMMANDS ; i++ ) count[i] -= count_old[i];

  printf("\n");
  printf("Summary for Plot %d\n",plot_number++);
  printf(" %9d ALLOW OPAQUE OBJECTS\n", count[P_ALLOW_OPAQUE]);
  printf(" %9d BEGIN FRAME\n", count[P_BEGIN_FRAME]);
  printf(" %9d DEFINE DASH STYLE\n", count[P_DEFINE_DASH]);
  printf(" %9d DEFINE DITHER COLOR\n", count[P_DEFINE_DITHER]);
  printf(" %9d DEFINE FONT\n", count[P_DEFINE_FONT]);
  printf(" %9d DEFINE GAMMA CORRECTION\n", count[P_DEFINE_GAMMA_CORRECTION]);
  printf(" %9d DEFINE MARKER SET\n", count[P_DEFINE_MARKER]);
  printf(" %9d DEFINE MONITOR CONSTANTS\n", count[P_DEFINE_MONITOR_CONSTANTS]);
  printf(" %9d DEFINE PATTERN\n", count[P_DEFINE_PATTERN]);
  printf(" %9d DEFINE PATTERN COVERAGES\n", count[P_DEFINE_COVERAGES]);
  printf(" %9d DEFINE PRIMARY CONSTANTS\n", count[P_DEFINE_PRIMARY_CONSTANTS]);
  printf(" %9d DEFINE RASTER STAMP\n", count[P_DEFINE_RASTER_STAMP]);
  printf(" %9d DEFINE RGB COLOR\n", count[P_DEFINE_RGB]);
  printf(" %9d DRAW CELL ARRAY\n", count[P_DRAW_CELL_ARRAY]);
  printf(" %9d DRAW CIRCLE\n", count[P_DRAW_CIRCLE]);
  printf(" %9d DRAW CMY RASTER\n", count[P_DRAW_CMY_RASTER]);
  printf(" %9d DRAW POLYGON\n", count[P_DRAW_POLYGON]);
  printf(" %9d DRAW POLYGON SET\n", count[P_DRAW_POLYGON_SET]);
  printf(" %9d DRAW POLYLINE\n", count[P_DRAW_POLYLINE]);
  printf(" %9d DRAW POLYMARKER\n", count[P_DRAW_POLYMARKER]);
  printf(" %9d DRAW RASTER STAMP\n", count[P_DRAW_RASTER_STAMP]);
  printf(" %9d DRAW RECTANGLE\n", count[P_DRAW_RECTANGLE]);
  printf(" %9d DRAW RGB CELL ARRAY\n", count[P_DRAW_RGB_CELL_ARRAY]);
  printf(" %9d DRAW RGB RUN LENGTH\n", count[P_DRAW_RGB_RUN_LENGTH]);
  printf(" %9d DRAW RUN LENGTH\n", count[P_DRAW_RUN_LENGTH]);
  printf(" %9d DRAW TEXT\n", count[P_DRAW_TEXT]);
  printf(" %9d END FRAME\n", count[P_END_FRAME]);
  printf(" %9d END INITIALIZATION\n", count[P_END_INIT]);
  printf(" %9d INITIALIZE LEVEL ORDER\n", count[P_LEVEL_ORDER]);
  printf(" %9d INITIALIZE LEVELS\n", count[P_INITIALIZE_LEVELS]);
  printf(" %9d INVALID\n", count[P_INVALID]);
  printf(" %9d PLOT LABEL\n", count[P_PLOT_LABEL]);
  printf(" %9d SET CLIP VIEWPORT\n", count[P_SET_CLIP_VIEWPORT]);
  printf(" %9d SET CONTROLLER OPTIONS\n", count[P_SET_CNTRL_OPTIONS]);
  printf(" %9d SET DASH WRAP STYLE\n", count[P_SET_DASH_WRAP_STYLE]);
  printf(" %9d SET EDGE VISIBILITY\n", count[P_SET_EDGE_VISIBILITY]);
  printf(" %9d SET FILL COLOR\n", count[P_SET_FILL_COLOR]);
  printf(" %9d SET FILL RGB\n", count[P_SET_FILL_RGB]);
  printf(" %9d SET FILL STYLE\n", count[P_SET_FILL_STYLE]);
  printf(" %9d SET FONT\n", count[P_SET_FONT]);
  printf(" %9d SET LEVEL\n", count[P_SET_LEVEL]);
  printf(" %9d SET LINE CAP STYLE\n", count[P_SET_LINE_CAP_STYLE]);
  printf(" %9d SET LINE COLOR\n", count[P_SET_LINE_COLOR]);
  printf(" %9d SET LINE DASH INDEX\n", count[P_SET_LINE_DASH_INDEX]);
  printf(" %9d SET LINE JOINT STYLE\n", count[P_SET_LINE_JOINT_STYLE]);
  printf(" %9d SET LINE RGB\n", count[P_SET_LINE_RGB]);
  printf(" %9d SET LINE WIDTH\n", count[P_SET_LINE_WIDTH]);
  printf(" %9d SET MARKER BASELINE\n", count[P_SET_MARKER_BASELINE]);
  printf(" %9d SET MULTIPLEXER\n", count[P_SET_MULTIPLEXER]);
  printf(" %9d SET OPAQUE\n", count[P_SET_OPAQUE]);
  printf(" %9d SET PAPER CONTROL\n", count[P_SET_PAPER_CONTROL]);
  printf(" %9d SET PLOTTER IMAGING\n", count[P_SET_PLOTTER_IMAGING]);
  printf(" %9d SET POSTPLOT CONTROL\n", count[P_SET_POSTPLOT]);
  printf(" %9d SET POSTPRINT\n", count[P_SET_POSTPRINT]);
  printf(" %9d SET PREPLOT CONTROL\n", count[P_SET_PREPLOT]);
  printf(" %9d SET TEXT ALIGNMENT\n", count[P_SET_TEXT_ALIGNMENT]);
  printf(" %9d SET TEXT BASELINE\n", count[P_SET_TEXT_BASELINE]);
  printf(" %9d SET TEXT ORIENTATION\n", count[P_SET_TEXT_ORIENTATION]);
  printf(" %9d SET TEXT PATH\n", count[P_SET_TEXT_PATH]);
  printf(" %9d SET TEXT SPACING\n", count[P_SET_TEXT_SPACING]);
  printf(" %9d SKIP DATA\n", count[P_SKIP_DATA]);
  if (err_count)
    printf("*** WARNING - %d WARNINGS were found\n\n", err_count);

  for ( i = 1 ; i < NUM_VCGL_COMMANDS ; i++ ) {
    count[i] += count_old[i];
    count_old[i] = count[i];
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

#undef OPT_SIZ
#undef BUF_SIZ

/* ************************************************************************* */
/* vcglsw.c  -- This file contains the main vcgl switch statement            */
/*              it is planned that this function (vcglsw) will be used       */
/*              with several utilities:                                      */
/*                                      1.  vcgl intrepretive dump utility   */
/*                                      2.  vcgl to vmi translator           */
/* The code was written by:                                                  */
/*                           Brian Jones                                     */
/*                           VerDak Graphics Software & Systems              */
/*                           versatc!brian                                   */
/*                           October 25, 1988                                */
/*                                                                           */
/* Copyright c1988, Versatec Inc., a Xerox Company, Santa Clara California.  */
/*                                                                           */
/* ************************************************************************* */

/* sccs_vcglsw[] = vcglsw.c 1.11 12/9/88 */

static vcglsw()
{
  unsigned short vcgl_cmd;

  for(;;) {
    if (frame_sync_found == 0) {
      type = 0;                          /* Used to return X,Y,dx,dy, below */
      find_frame_sync();
    }
    switch (vcgl_cmd = get_cmd()) {
    case 0x00:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x01:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x02:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x03:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x04:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x05:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x06:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x07:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x08:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x09:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x0a:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x0b:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x0c:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x0d:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x0e:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x0f:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x10:                           /* Skip Data                       */
      skip_data();
      break;
    case 0x11:                           /* Set Postprint                   */
      set_postprint();
      break;
    case 0x12:                           /* Set Preplot Control             */
      set_preplot_control();
      break;
    case 0x13:                           /* Set Postplot Control            */
      set_postplot_control();
      break;
    case 0x14:                           /* Plot Label                      */
      plot_label();
      break;
    case 0x15:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x16:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x17:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x18:                           /* Possible Frame Sync             */
      fsync_look();
      break;
    case 0x19:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x1a:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x1b:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x1c:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x1d:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x1e:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x1f:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x20:                           /* Allow Opaque Object             */
      allow_opaque();
      break;
    case 0x21:                           /* Init Levels                     */
      init_levels();
      break;
    case 0x22:                           /* Init Level Order                */
      level_order();
      break;
    case 0x23:                           /* End Init                        */
      end_init();
      break;
    case 0x24:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x25:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x26:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x27:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x28:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x29:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x2a:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x2b:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x2c:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x2d:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x2e:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x2f:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x30:                           /* Define Dither Color             */
      define_dither();
      break;
    case 0x31:                           /* Define RGB Color                */
      define_RGB();
      break;
    case 0x32:                           /* Define Pattern                  */
      define_pattern();
      break;
    case 0x33:                           /* Define Raster Stamp             */
      define_raster_stamp();
      break;
    case 0x34:                           /* Define Font                     */
      define_font();
      break;
    case 0x35:                           /* Define Marker Set               */
      define_marker();
      break;
    case 0x36:                           /* Define Dash Style               */
      define_dash();
      break;
    case 0x37:                           /* Define Pattern Coverages        */
      define_coverages();
      break;
    case 0x38:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x39:                           /* Set Edge Visibility             */
      set_edge_visibility();
      break;
    case 0x3a:                           /* Define Primary Constants        */
      define_primary_constants();
      break;
    case 0x3b:                           /* Define Gamma Correction         */
      define_gamma_correction();
      break;
    case 0x3c:                           /* Define Monitor Constants        */
      define_monitor_constants();
      break;
    case 0x3d:                           /* Set Line RGB                    */
      set_line_RGB();
      break;
    case 0x3e:                           /* Set Fill RGB                    */
      set_fill_RGB();
      break;
    case 0x3f:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x40:                           /* Set Line Width                  */
      set_line_width();
      break;
    case 0x41:                           /* Set Line Color                  */
      set_line_color();
      break;
    case 0x42:                           /* Set Dash Wrap Style             */
      set_dash_wrap_style();
      break;
    case 0x43:                           /* Set Line Dash Index             */
      set_line_dash_index();
      break;
    case 0x44:                           /* Set Line Cap Style              */
      set_line_cap_style();
      break;
    case 0x45:                           /* Set Line Joint Style            */
      set_line_joint_style();
      break;
    case 0x47:                           /* Set Fill Color                  */
      set_fill_color();
      break;
    case 0x48:                           /* Set Fill Style                  */
      set_fill_style();
      break;
    case 0x49:                           /* Set Marker Baseline             */
      set_marker_baseline();
      break;
    case 0x4a:                           /* Set Text Orientation            */
      set_text_orientation();
      break;
    case 0x4b:                           /* Set Text Baseline               */
      set_text_baseline();
      break;
    case 0x4c:                           /* Set Text Alignment              */
      set_text_alignment();
      break;
    case 0x4d:                           /* Set Text Path                   */
      set_text_path();
      break;
    case 0x4e:                           /* Set Text Spacing                */
      set_text_spacing();
      break;
    case 0x4f:                           /* Set Font                        */
      set_font();
      break;
    case 0x50:                           /* Set Opaque                      */
      set_opaque();
      break;
    case 0x51:                           /* Set Level                       */
      set_level();
      break;
    case 0x52:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x53:                           /* Set Clip Viewport               */
      set_clip_viewport();
      break;
    case 0x54:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x55:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x56:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x57:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x58:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x59:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x5a:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x5b:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x5c:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x5d:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x5e:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x5f:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x60:                           /* Draw Polyline 16 X, 16 Y, 8 rel */
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x61:                           /* Draw Polyline 32 X, 16 Y, 8 rel */
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x62:                           /* Draw Polyline 16 X, 32 Y, 8 rel */
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x63:                           /* Draw Polyline 32 X, 32 Y, 8 rel */
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x64:                           /* Draw Polyline 16 X, 16 Y, 16 rel*/
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x65:                           /* Draw Polyline 32 X, 16 Y, 16 rel*/
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x66:                           /* Draw Polyline 16 X, 32 Y, 16 rel*/
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x67:                           /* Draw Polyline 32 X, 32 Y, 16 rel*/
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x68:                           /* Draw Polyline 16 X, 16 Y, 32 rel*/
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x69:                           /* Draw Polyline 32 X, 16 Y, 32 rel*/
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x6a:                           /* Draw Polyline 16 X, 32 Y, 32 rel*/
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;
    case 0x6b:                           /* Draw Polyline 32 X, 32 Y, 32 rel*/
      type = vcgl_cmd - 0x60;
      draw_polyline();
      break;

    case 0x6c:                           /* Draw Text 16 X, 16 Y, 8         */
      type = vcgl_cmd - 0x6c;
      draw_text();
      break;
    case 0x6d:                           /* Draw Text 32 X, 16 Y, 8         */
      type = vcgl_cmd - 0x6c;
      draw_text();
      break;
    case 0x6e:                           /* Draw Text 16 X, 32 Y, 8         */
      type = vcgl_cmd - 0x6c;
      draw_text();
      break;
    case 0x6f:                           /* Draw Text 32 X, 32 Y, 8         */
      type = vcgl_cmd - 0x6c;
      draw_text();
      break;
    case 0x70:                           /* Draw Text 16 X, 16 Y, 16        */
      type = vcgl_cmd - 0x6c;
      draw_text();
      break;
    case 0x71:                           /* Draw Text 32 X, 16 Y, 16        */
      type = vcgl_cmd - 0x6c;
      draw_text();
      break;
    case 0x72:                           /* Draw Text 16 X, 32 Y, 16        */
      type = vcgl_cmd - 0x6c;
      draw_text();
      break;
    case 0x73:                           /* Draw Text 32 X, 32 Y, 16        */
      type = vcgl_cmd - 0x6c;
      draw_text();
      break;
    case 0x74:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x75:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x76:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x77:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0x78:                           /* Draw Polygon 16 X, 16 Y, 8 rel  */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x79:                           /* Draw Polygon 32 X, 16 Y, 8 rel  */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x7a:                           /* Draw Polygon 16 X, 32 Y, 8 rel  */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x7b:                           /* Draw Polygon 32 X, 32 Y, 8 rel  */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x7c:                           /* Draw Polygon 16 X, 16 Y, 16 rel */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x7d:                           /* Draw Polygon 32 X, 16 Y, 16 rel */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x7e:                           /* Draw Polygon 16 X, 32 Y, 16 rel */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x7f:                           /* Draw Polygon 32 X, 32 Y, 16 rel */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x80:                           /* Draw Polygon 16 X, 16 Y, 32 rel */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x81:                           /* Draw Polygon 32 X, 16 Y, 32 rel */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x82:                           /* Draw Polygon 16 X, 32 Y, 32 rel */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x83:                           /* Draw Polygon 32 X, 32 Y, 32 rel */
      type = vcgl_cmd - 0x78;
      draw_polygon();
      break;
    case 0x84:                           /* Draw Polygon Set 16, 16, 8 rel  */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x85:                           /* Draw Polygon Set 32, 16, 8 rel  */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x86:                           /* Draw Polygon Set 16, 32, 8 rel  */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x87:                           /* Draw Polygon Set 32, 32, 8 rel  */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x88:                           /* Draw Polygon Set 16, 16, 16 rel */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x89:                           /* Draw Polygon Set 32, 16, 16 rel */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x8a:                           /* Draw Polygon Set 16, 32, 16 rel */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x8b:                           /* Draw Polygon Set 32, 32, 16 rel */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x8c:                           /* Draw Polygon Set 16, 16, 32 rel */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x8d:                           /* Draw Polygon Set 32, 16, 32 rel */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x8e:                           /* Draw Polygon Set 16, 32, 32 rel */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x8f:                           /* Draw Polygon Set 32, 32, 32 rel */
      type = vcgl_cmd - 0x84;
      draw_polygon_set();
      break;
    case 0x90:                           /* Draw Rectangle 16, 16, 8 rel    */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x91:                           /* Draw Rectangle 32, 16, 8 rel    */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x92:                           /* Draw Rectangle 16, 32, 8 rel    */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x93:                           /* Draw Rectangle 32, 32, 8 rel    */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x94:                           /* Draw Rectangle 16, 16, 16 rel   */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x95:                           /* Draw Rectangle 32, 16, 16 rel   */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x96:                           /* Draw Rectangle 16, 32, 16 rel   */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x97:                           /* Draw Rectangle 32, 32, 16 rel   */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x98:                           /* Draw Rectangle 16, 16, 32 rel   */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x99:                           /* Draw Rectangle 32, 16, 32 rel   */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x9a:                           /* Draw Rectangle 16, 32, 32 rel   */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x9b:                           /* Draw Rectangle 32, 32, 32 rel   */
      type = vcgl_cmd - 0x90;
      draw_rectangle();
      break;
    case 0x9c:                           /* Draw Circle 16 X, 16 Y          */
      type = vcgl_cmd - 0x9c;
      draw_circle();
      break;
    case 0x9d:                           /* Draw Circle 32 X, 16 Y          */
      type = vcgl_cmd - 0x9c;
      draw_circle();
      break;
    case 0x9e:                           /* Draw Circle 16 X, 32 Y          */
      type = vcgl_cmd - 0x9c;
      draw_circle();
      break;
    case 0x9f:                           /* Draw Circle 32 X, 32 Y          */
      type = vcgl_cmd - 0x9c;
      draw_circle();
      break;
    case 0xa0:                           /* Draw Raster Stamp 16 X, 16 Y    */
      type = vcgl_cmd - 0xa0;
      draw_raster_stamp();
      break;
    case 0xa1:                           /* Draw Raster Stamp 32 X, 16 Y    */
      type = vcgl_cmd - 0xa0;
      draw_raster_stamp();
      break;
    case 0xa2:                           /* Draw Raster Stamp 16 X, 32 Y    */
      type = vcgl_cmd - 0xa0;
      draw_raster_stamp();
      break;
    case 0xa3:                           /* Draw Raster Stamp 32 X, 32 Y    */
      type = vcgl_cmd - 0xa0;
      draw_raster_stamp();
      break;
    case 0xa4:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xa5:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xa6:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xa7:                           /* Draw Cell Array                 */
      draw_cell_array();
      break;
    case 0xa8:                           /* Draw RGB Cell Array             */
      draw_RGB_cell_array();
      break;
    case 0xaa:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xab:                           /* Draw Run Length                 */
      draw_run_length();
      break;
    case 0xac:                           /* Draw RGB Run Length             */
      draw_RGB_run_length();
      break;
    case 0xad:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xae:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xaf:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xb0:                           /* Draw CMY Raster                 */
      draw_CMY_raster();
      break;
    case 0xb1:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xb2:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xb3:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xb4:                           /* Draw Polymarker 16,16,8         */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xb5:                           /* Draw Polymarker 32,16,8         */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xb6:                           /* Draw Polymarker 16,32,8         */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xb7:                           /* Draw Polymarker 32,32,8         */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xb8:                           /* Draw Polymarker 16,16,16        */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xb9:                           /* Draw Polymarker 32,16,16        */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xba:                           /* Draw Polymarker 16,32,16        */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xbb:                           /* Draw Polymarker 32,32,16        */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xbc:                           /* Draw Polymarker 16,16,32        */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xbd:                           /* Draw Polymarker 32,16,32        */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xbe:                           /* Draw Polymarker 16,32,32        */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xbf:                           /* Draw Polymarker 32,32,32        */
      type = vcgl_cmd - 0xb4;
      draw_polymarker();
      break;
    case 0xc0:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xc1:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xc2:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xc3:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xc4:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xc5:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xc6:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xc7:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xc8:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xc9:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xca:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xcb:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xcc:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xcd:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xce:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xcf:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd0:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd1:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd2:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd3:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd4:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd5:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd6:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd7:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd8:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xd9:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xda:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xdb:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xdc:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xdd:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xde:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xdf:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe0:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe1:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe2:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe3:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe4:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe5:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe6:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe7:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe8:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xe9:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xea:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xeb:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xec:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xed:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xee:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xef:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xf0:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xf1:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xf2:                           /* Begin Frame                     */
      begin_frame();
      break;
    case 0xf3:                           /* End Frame                       */
      end_frame();
      break;
    case 0xf4:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xf5:                           /* Set Cntrl options               */
      set_cntrl_options();
      break;
    case 0xf6:                           /* Set Plotter Imaging             */
      set_plotter_imaging();
      break;
    case 0xf7:                           /* Set Paper Control               */
      set_paper_control();
      break;
    case 0xf8:                           /* Set Multiplexer                 */
      set_multiplexer();
      break;
    case 0xf9:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xfa:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xfb:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xfc:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xfd:                           /* -------<<<I N V A L I D>>>----- */
      invalid();
      break;
    case 0xfe:                           /* << INVALID - Very Long Cmd >>   */
      invalid();
      break;
    case 0xff:                           /* -- << INVALID - Long Cmd >>>  - */
      invalid();
      break;
    default:                             /* Should Never Get Here           */
      program_bug(vcgl_cmd);
      break;
    }
  }
}
/* ************************************************************************* */
/* vcgl_ascii.c -- Ascii Interpretive Dump for VCGL data                     */
/*                 These functions called by vcglsw.c                        */
/*                                                                           */
/* The code was written by:                                                  */
/*                           Brian Jones                                     */
/*                           VerDak Graphics Software Systems                */
/*                           versatc!brian                                   */
/*                           October 25, 1988                                */
/*                                                                           */
/* Copyright c1988, Versatec Inc., a Xerox Company, Santa Clara California.  */
/*                                                                           */
/* ************************************************************************* */

#define _IN_ASCII

/* sccs_ascii[] = {"vcgl_ascii.c  1.16  12/9/88 */

static short cmd_output;                 /* Flag to printing current command */

/* ************************************************************************* */
/* Output Check Function                                                     */
/* Is it OK to print out a message?                                          */
/* An array of counts are maintained which specify for each VCGL            */
/* command when to begin and end printing interpretive dump                  */
/* information.  This routine sets full_output true if it's now              */
/* time to output that information based on the input parameter.             */
/* ************************************************************************* */

static output_ck(cmd_type)
short cmd_type;
{
  cmd_output = FALSE;
  count[cmd_type]++;
  if (count[cmd_type] >= start_output[cmd_type] &&
      count[cmd_type] <= end_output[cmd_type]) {
    foutput("%s", sbuf);
    if ((cmd_output = full_output) == TRUE)
      printf("%s", sbuf);
  }
}


/* ************************************************************************* */
/*  foutput & woutput routines -- Output messages & warnings                 */
/*  There are two versions of these routines ... one for                     */
/*  Sun/SCO Xenix and another for ANSII C                                    */
/*  This is needed because of the different methods for handling             */
/*  a variable number of arguments.                                          */
/*                                                                           */
/*  foutput -- Used to output normal messages if the global                  */
/*             variable -- full_output is set to true                        */
/*             foutput has the same parameters as printf                     */
/*                                                                           */
/*  woutput -- Used to output warning messages                               */
/*             woutput has the same parameters as printf                     */
/* ************************************************************************* */

#ifdef STDARG

static foutput(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  if (cmd_output)
    VFPRINTF(stdout, fmt, args);
  va_end(args);
}

static woutput(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  VFPRINTF(stdout, fmt, args);
  if (!cmd_output)
    fprintf(stdout, "   Last VCGL Command = %X  %s\n", last_vcgl_cmd, sbuf);
  va_end(args);
}

#endif
#ifdef VARARGS

static foutput(va_alist)
va_dcl
{
  char *fmt;
  va_list args;

  va_start(args);
  fmt = va_arg(args, char *);
  if (cmd_output)
    VFPRINTF(stdout, fmt, args);
  va_end(args);
}

static woutput(va_alist)
va_dcl
{
  char *fmt;
  va_list args;

  va_start(args);
  fmt = va_arg(args, char *);
  VFPRINTF(stdout, fmt, args);
  if (!cmd_output)
    fprintf(stdout, "   Last VCGL Command = %X\n", last_vcgl_cmd);
  va_end(args);
}

#endif


/* ************************************************************************ */
/*    Hexidecimal with ascii formatted output -- Basically stolen           */
/*    from code written by Ray Salas in /usr/local/src/hd.c                 */
/* ************************************************************************ */

#define MAX_LINE 80
static char out_str[MAX_LINE];           /* output line string              */

/* init_line  -  Initializes the output line                                */

static init_line(out_line)
char *out_line;
{
  int index;

  for (index = 0; index < MAX_LINE; index++)
    out_line[index] = ' ';
  out_line[51] = '*';
  out_line[70] = '*';
  out_line[MAX_LINE-2] = '\n';
  out_line[MAX_LINE-1] = '\0';
}

/* fmthex - Format a string of characters corresponding to hex number       */

static fmthex(line_ptr, in_val, in_ofs, in_size)
char *line_ptr;                        /* pointer destination line string */
long in_val;                           /* value to be converted           */
int in_ofs;                            /* buffer offset at which hex      */
                                       /* string is placed                */
int in_size;                           /* number of characters in hex     */
                                       /* string field                    */
{
  int index;
  long temp;
  char hex_char;

  temp = in_val;
  for (index = 0; index < in_size; index++) {
    if ((temp & 0x0F) >= 10)
      hex_char = (char) ((temp & 0x0F) - 10 + 'A');
    else
      hex_char = (char) ((temp & 0x0F) + '0');
    line_ptr[(in_ofs + in_size - 1) - index] = hex_char;
    temp >>= 4;
  }
}

/* dump_block  -  dumps the specified block of memory */

static dump_block(buf_ptr, num_bytes, out_line)
char *buf_ptr;                         /* pointer to data buffer           */
long num_bytes;                        /* number of bytes in block         */
char *out_line;                        /* pointer to output line           */
{
  long index;
  int hex_ofs;
  int char_ofs;
  char curr_char;

  /* initialize loop index */
  index = 0;

  while (index < num_bytes) {
    if (index % 16 == 0) {
      /* initialize line */
      init_line(out_line);

      /* dump memory address for line */
      /* initialize offsets into line */
      hex_ofs = 11;
      char_ofs = 53;
    }
    /* set value for current character */
    curr_char = buf_ptr[index];

    /* dump hexadecimal value of current character */
    fmthex(out_line, (long) curr_char, hex_ofs, 2);

    if ((curr_char < 32)
        || (curr_char > 126)) {
      /* character is unprintable--store dot in ASCII field */
      out_line[char_ofs++] = '.';
    } else {
      /* store character in ASCII field */
      out_line[char_ofs++] = curr_char;
    }

    /* increment character index */
    index++;

    if (index % 2 == 0) {
      /* at word boundary--insert space */
      hex_ofs += 3;
    } else {
      /* continue to next hexadecimal position */
      hex_ofs += 2;
    }

    if (index % 16 == 0) {
      /* line format complete--dump output line */
      foutput("%s", out_line);
    }
  }

  if (index % 16 != 0) {
    /* partial line pending--dump output line */
    foutput("%s", out_line);
  }
}


/* dump_data function -- Dump Input Data in Hex Format */

static dump_data()
{
  long i, val_bytes;
  char *bybufptr,*saveptr,*malloc();

  val_bytes = val_words * 2 + odd_byte_flag;
  if (val_bytes > BYTES_DUMPED)
    val_bytes = BYTES_DUMPED;
  if (val_bytes <= 0) {
    foutput("           No Data\n");
    return;
  }
  if ((bybufptr = malloc((unsigned int) val_bytes))
      != NULL) {
    saveptr = bybufptr;
    i = val_bytes;
    while (i-- > 0)
      *bybufptr++ = get_byte(TRUE);
    dump_block(saveptr, val_bytes, out_str);
    free(saveptr);
  } else {
    woutput("WARNING: malloc(%ld) exceeded system limits\n", val_bytes);
    err_count++;
  }
  if (val_words != 0)
    foutput("\n                                      . . .\n\n");
  while (val_words > 0)
    get_short();                /* Discard Skipped Data */
  if (odd_byte_flag)
    get_byte(TRUE);
}



/****************************************************************************/
/* Program Bug                                                              */
/****************************************************************************/

static program_bug(v)
unsigned short v;
{
  woutput("WARNING: PROGRAM BUG, vcgl_cmd = %u\n", v);
  err_count++;
  foutput("    last vcgl_cmd = 0x%X\n", last_vcgl_cmd);
  if ( print_sum ) summary();
  exit(1);
}

/* ************************************************************************* */
/* Invalid VCGL Command  --      Error Processing Function                   */
/* ************************************************************************* */

static invalid()
{
  woutput("WARNING: %d Invalid vcgl_cmd = 0x%X\n",
          count[P_INVALID], last_vcgl_cmd);
  err_count++;
}

/* ****************************************************************** */
/* Find Frame Sync VCGL Command Processor                             */
/* ****************************************************************** */

/* This function is used to find the initial Frame Sync which         */
/* is before the Begin Frame VCGL command.                            */

static char frame_sync[] = {
  0x18, 0x01, 0x00, 0x00, 0x18, 0x02, 0x00, 0x00,
  0x18, 0x02, 0xff, 0xff, 0x18, 0x01, 0x00, 0x00,
  0x18, 0x02, 0x00, 0x00, 0x18, 0x02, 0xff, 0xfe,
  0x18, 0x01, 0x00, 0x00, 0x18, 0x02, 0x00, 0x00,
  0x18, 0x02, 0xff, 0xfd, 0x18, 0x01, 0x00, 0x00,
  0x18, 0x02, 0x00, 0x00, 0x18, 0x02, 0xff, 0xfc,
  0x18, 0x01, 0x00, 0x00, 0x18, 0x02, 0x00, 0x00,
  0x18, 0x02, 0xff, 0xfb, 0x18, 0x01, 0x00, 0x00,
  0x18, 0x02, 0x00, 0x00, 0x18, 0x02
};

static find_frame_sync()
{
  short state;
  char c;
  char i;

  state = 0;
  while (!frame_sync_found) {
    switch (state) {
      /* Case 0 get's our byte alignment set properly */
    case 0:
      if ((c = get_byte(FALSE)) == 0x18) {
        if (debug_level > 5)
          foutput("Got Char state 0 0x%X \n", c);
        state = 1;
        break;
      } else if (debug_level > 5)
        foutput("Got char state 0 %d (%c)\n", c);
      break;

      /* Check word against array */
    default:
      if ((i = get_byte(FALSE)) == frame_sync[state]) {
        if (debug_level > 5)
          foutput("Got Char - state %d 0x%X\n", state, c);
        if ((state++) == 69) {
          /* Complete Frame Sync Found */
          print_sum = 1;
          frame_sync_found = 1;
          if (debug_level > 1)
            foutput("\nFRAME SYNC FOUND\n\n");
          val_words = 0;
          return;
        } else
          /* Still in frame sync array, not done yet */
          break;
      } else {
        /* Faulted frame sync array */
        if (i == 0x18)
          /* If possible new plot */
          state = 1;
        else
          /* Else back to transparent mode */
          state = 0;
        break;
      }
    }
  }
  foutput("FRAME SYNC NOT FOUND\n");
  exit(1);
}

/* ************************************************************************* */
/* Begin Frame VCGL Command Processor                                        */
/* ************************************************************************* */

static begin_frame()
{
  char i;

  val_words = get_val();        /* Get count of words to skip */
  output_ck(P_BEGIN_FRAME);
  foutput("%6d BEGIN FRAME,                 ccnn = %X%02X (%ld)\n",
          count[P_BEGIN_FRAME], last_vcgl_cmd, val_words, val_words);
  foutput("           Version:       %d\n", get_short());
  foutput("           Plotter Model: ");
  while (val_words > 0)
    if ( (i = get_byte(TRUE)) != 0 ) foutput("%c",i);
  if (odd_byte_flag)
    if ( (i = get_byte(TRUE)) != 0 ) foutput("%c",i);
  foutput("\n");
}

/* ************************************************************************* */
/* Look for Frame Sync Before the End FRame VCGL Command Processor           */
/* ************************************************************************* */

/* An 0x18 was found, this is either an illegal VCGL command, or the         */
/* first byte of the frame sync preceeding the End Frame VCGL command       */
/* The following code attempts to find out which.                            */

static fsync_look()
{
  short state;
  char i;
  short j;

  state = 1;
  while (state <= 69) {
    /* Check word against array */
    if ((i = get_byte(FALSE)) == frame_sync[state]) {
      if (debug_level > 5)
        foutput("Got Char - state %d 0x%X\n", state, i);
      state++;
    } else {
      woutput("WARNING: Illegal VCGL Command\n");
      err_count++;
      foutput("    Thought it was a frame sync before\n");
      foutput("    The End Frame...Got Byte Sequence:\n");
      for (j = 0; j < state; j++)
        foutput("   0x%0X\n", frame_sync[j]);
      foutput("   0x%0X\n", i);
      return;
    }
  }
  /* Complete Frame Sync Found */
  if (debug_level > 1)
    foutput("FRAME SYNC FOUND\n\n");
  frame_sync_found = 2;         /* Flag for End Frame Test */
  val_words = 0;                /* No command words remain */
}


/* ****************************************************************** */
/* End Frame VCGL Command Processor                                   */
/* ****************************************************************** */

static end_frame()
{
  short i;

  if (frame_sync_found != 2) {
    woutput("WARNING: No Frame Sync Before End Frame\n");
    err_count++;
  }
  end_frame_found = 1;
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: End Frame Word Count = %d, expected 5\n", val_words);
    err_count++;
  }
  output_ck(P_END_FRAME);
  foutput("%6d END FRAME,                   ccnn = %X%02X (%ld)\n",
          count[P_END_FRAME], last_vcgl_cmd, val_words, val_words);
  if ((i = get_short()) <= 1)
    foutput("           Plot a Single Copy   (%d)\n", i);
  else
    foutput("           Plot %d Copies\n", i);

  summary();
  print_sum = 0;
  frame_sync_found = 0;
  end_frame_found = 0;
  err_count = 0;
}

/* ************************************************************************* */
/* Skip Data VCGL Command Processor                                          */
/* ************************************************************************* */

static skip_data()
{
  val_words = get_val();        /* Get count of words to skip */
  output_ck(P_SKIP_DATA);
  foutput("%6d SKIP DATA,                   ccnn = %X%02X (%ld)\n",
          count[P_SKIP_DATA], last_vcgl_cmd, val_words, val_words);
  foutput("           Skipping %ld Words\n", val_words);
  while (val_words > 0)
    get_short();                /* Discard Skipped Data */
}

/* ************************************************************************* */
/* set postprint VCGL Command Processor                                      */
/* ************************************************************************* */

static set_postprint()
{
  val_words = get_val();        /* Get count of words to skip */
  output_ck(P_SET_POSTPRINT);
  foutput("%6d SET POSTPRINT,               ccnn = %X%02X (%ld)\n",
          count[P_SET_POSTPRINT], last_vcgl_cmd, val_words, val_words);
  dump_data();
}


/* ************************************************************************ */
/* set preplot VCGL Command Processor                                       */
/* ************************************************************************ */

static set_preplot_control()
{
  val_words = get_val();        /* Get count of words to skip */
  output_ck(P_SET_POSTPLOT);
  foutput("%6d SET PREPLOT CONTROL,         ccnn = %X%02X (%ld)\n",
          count[P_SET_PREPLOT], last_vcgl_cmd, val_words, val_words);
  dump_data();
}


/* ************************************************************************ */
/* set Postplot control VCGL Command Processor                              */
/* ************************************************************************ */

static set_postplot_control()
{
  val_words = get_val();        /* Get count of words to skip */
  output_ck(P_SET_POSTPLOT);
  foutput("%6d SET POSTPLOT CONTROL,        ccnn = %X%02X (%ld)\n",
          count[P_SET_POSTPLOT], last_vcgl_cmd, val_words, val_words);
  dump_data();
}

/* ************************************************************************ */
/* set Plot Label VCGL Command Processor                                    */
/* ************************************************************************ */

static plot_label()
{
  val_words = get_val();        /* Get count of words to skip */
  output_ck(P_PLOT_LABEL);
  foutput("%6d PLOT LABEL,                  ccnn = %X%02X (%ld)\n",
          count[P_PLOT_LABEL], last_vcgl_cmd, val_words, val_words);
  foutput("           Number of Characters = %d\n", get_short());
  dump_data();
}

/* ************************************************************************ */
/* set Paper Control VCGL Command Processor                                 */
/* ************************************************************************ */

static set_paper_control()
{
  short i;

  val_words = get_val();        /* Get count of words to skip */
  if (val_words != 5) {
    woutput("WARNING: Set Paper Control Command Word Count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_PAPER_CONTROL);
  foutput("%6d SET PAPER CONTROL,           ccnn = %X%02X (%ld)\n",
          count[P_SET_PAPER_CONTROL], last_vcgl_cmd, val_words, val_words);
  switch (i = get_short()) {

  case -8739:
    foutput("           Unspecified Space Before Plot\n");
    break;
  case -2:
    foutput("           Two Form Feeds Before plot\n");
    break;
  case -1:
    foutput("           One Form Feed Before plot\n");
    break;
  default:
    foutput("           Space %d Scan Lines Before plot\n", i);
    break;
  }

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified Space Between Copies\n");
    break;
  case -1:
    foutput("           One Form Feed Between Copies\n");
    break;
  default:
    foutput("           Space %d Scan Lines Between Copies\n", i);
    break;
  }

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified Space After Plot\n");
    break;
  case -2:
    foutput("           Two Form Feeds After plot\n");
    break;
  case -1:
    foutput("           One Form Feed After plot\n");
    break;
  default:
    foutput("           Space %d Scan Lines After plot\n", i);
    break;
  }

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified Paper Cut\n");
    break;
  case -1:
    foutput("           No paper cutter installed\n");
    break;
  case 0:
    foutput("           Do not cut\n");
    break;
  case 1:
    foutput("           Cut plot, cut copies individually\n");
    break;
  case 2:
    foutput("           Cut plot, cut copies as a whole\n");
    break;
  case 3:
    foutput("           Cut before plot (before first copy)\n");
    break;
  case 4:
    foutput("           Cut after plot (after last copy\n");
    break;
  default:
    woutput("WARNING: unknown paper cut value = %d\n", i);
    err_count++;
    break;

  }

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified Paper Cut\n");
    break;
  case -1:
    foutput("           No Paper Cutter Installed\n");
    break;
  case 0:
    foutput("           Do not cut (suppress all cuts)\n");
    break;
  case 1:
    foutput("           Cut plot, cut copies individually\n");
    break;
  case 2:
    foutput("           Cut plot, cut copies as a whole\n");
    break;
  case 3:
    foutput("           Cut before plot (before first copy)\n");
    break;
  case 4:
    foutput("           Cut after plot (after last copy)\n");
    break;
  }

  switch (i = get_short()) {
  case 0:
    foutput("           Do not use page mode\n");
    break;
  case 1:
    foutput("           Use page mode\n");
    break;
  default:
    woutput("WARNING: unknown page mode = %d\n", i);
    err_count++;
    break;
  }
}


/* ************************************************************************ */
/* set multiplexer VCGL Command Processor                                   */
/* ************************************************************************ */

static set_multiplexer()
{
  short i;
  short j;

  val_words = get_val();        /* Get count of words to skip */
  output_ck(P_SET_MULTIPLEXER);
  foutput("%6d SET MULTIPLEXER,             ccnn = %X%02X (%ld)\n",
          count[P_SET_MULTIPLEXER], last_vcgl_cmd, val_words, val_words);
  switch (i = get_short()) {
    case 0:
      foutput("           Use Currently Selected Multiplexer Port \n");
    break;
    default:
      for (j = 1; j < 17; j++) {
        if (i & 1) foutput("           Use Multiplexer Port %d\n", j);
        i >>= 1;
      }
    break;
  }
}


/* ************************************************************************ */
/* allow opaque VCGL Command Processor                                      */
/* ************************************************************************ */

static allow_opaque()
{
  if ((val_words = get_val()) != 0) {
    woutput("WARNING - Allow Opaque word count = %d\n",val_words);
    err_count++;
  }
  output_ck(P_ALLOW_OPAQUE);
  foutput("%6d ALLOW OPAQUE OBJECTS,        ccnn = %X%02X (%ld)\n",
          count[P_ALLOW_OPAQUE], last_vcgl_cmd, val_words, val_words);
}

/* ************************************************************************ */
/* Initialize Levels VCGL Command Processor                                 */
/* ************************************************************************ */

static init_levels()
{
  val_words = get_val();
  output_ck(P_INITIALIZE_LEVELS);
  foutput("%6d INITIALIZE LEVELS,           ccnn = %X%02X (%ld)\n",
          count[P_INITIALIZE_LEVELS], last_vcgl_cmd, val_words, val_words);
  max_priority_level = get_short();
  foutput("           Maximum Priority Level = %d\n",
          max_priority_level);
}

/* ************************************************************************ */
/* Initialize Level Order VCGL Command Processor                            */
/* ************************************************************************ */

static level_order()
{
  short i;
  short j;

  val_words = get_val();
  output_ck(P_LEVEL_ORDER);
  foutput("%6d INITIALIZE LEVEL ORDER,      ccnn = %X%02X (%ld)\n",
          count[P_LEVEL_ORDER], last_vcgl_cmd, val_words, val_words);
  foutput("\n           Priority (1 = Highest)    Level Number\n");
  foutput("           ----------------------    ------------\n");
  i = 1;
  while (val_words > 0) {
    if ((j = get_short()) <= max_priority_level)
      foutput("                %4d                    %4d\n", i, j);
    else {
      woutput("WARNING: level %d is greater than Max Priority Level\n",j);
      err_count++;
    }
    i++;
  }
  foutput("\n");
}

/* ************************************************************************* */
/* End Initialization VCGL Command Processor                                 */
/* ************************************************************************* */

static end_init()
{
  if ((val_words = get_val()) != 0) {
    woutput("WARNING: End Init word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_END_INIT);
  foutput("%6d END INITIALIZATION,          ccnn = %X%02X (%ld)\n",
          count[P_END_INIT], last_vcgl_cmd, val_words, val_words);
}

/* ************************************************************************ */
/* Set Controller Options VCGL Command Processor                            */
/* ************************************************************************ */

static set_cntrl_options()
{
  short i;

  if ((val_words = get_val()) != 4) {
    woutput("WARNING: Set Controller Options word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_CNTRL_OPTIONS);
  foutput("%6d SET CONTROLLER OPTIONS,      ccnn = %X%02X (%ld)\n",
          count[P_SET_CNTRL_OPTIONS], last_vcgl_cmd, val_words, val_words);

  switch (i = get_short()) {

  case -8739:
    foutput("           Unspecified Disk Control\n");
    break;
  case 0:
    foutput("           Don't spool raster to disk\n");
    break;
  case 1:
    foutput("           Spool raster to disk\n");
    break;
  case 2:
    foutput("           Spool all passes to disk\n");
    break;
  default:
    woutput("WARNING: Unknown Disk control = %d\n", i);
    err_count++;
    break;
  }

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified Speed Control\n");
    break;
  case -3:
    foutput("           Autospeed mode (Controller Automatically");
    foutput(" Determines Speed\n");
    break;
  case -2:
    foutput("           Do not output speed control\n");
    break;
  case -1:
    foutput("           Output speed control of full speed\n");
    break;
  default:
    foutput("           Output speed control of %d\n", i);
    break;
  }

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified pre-clipped flag\n");
    break;
  case 0:
    foutput("           Data not pre-clipped\n");
    break;
  default:
    foutput("           Data pre-clipped\n");
    break;
  }

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified Statistics\n");
    break;
  case 0:
    foutput("           Disable plotting statistics\n");
    break;
  default:
    foutput("           Enable plotting statistics\n");
    break;
  }
}


/* ************************************************************************* */
/* Set Plotter Imaging VCGL Command Processor                                */
/* ************************************************************************* */

static set_plotter_imaging()
{
  short i;

  if ((val_words = get_val()) != 5) {
    woutput("WARNING - Set Plotter Imaging word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_PLOTTER_IMAGING);
  foutput("%6d SET PLOTTER IMAGING,         ccnn = %X%02X (%ld)\n",
          count[P_SET_PLOTTER_IMAGING], last_vcgl_cmd, val_words, val_words);

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified Line Enhance\n");
    break;
  case 0:
    foutput("           Do not use line enhance\n");
    break;
  case 1:
    foutput("           Use line enhance\n");
    break;
  default:
    woutput("WARNING: unknown line enhance value = %d\n", i);
    err_count++;
    break;
  }

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified Mirror Image\n");
    break;
  case 0:
    foutput("           Do not use Mirror Image\n");
    break;
  case 1:
    foutput("           Enable Mirror Image\n");
    break;
  default:
    woutput("WARNING: unknown mirror image value = %d\n", i);
    err_count++;
    break;
  }

  switch (i = get_short()) {
  case -8739:
    foutput("           Unspecified Inverse Image\n");
    break;
  case 0:
    foutput("           Do not use Inverse Image\n");
    break;
  case 1:
    foutput("           Use Inverse Image\n");
    break;
  default:
    woutput("WARNING: unknown inverse image value = %d\n");
    err_count++;
    break;
  }

  switch (i = get_short()) {
  case -1:
    foutput("           Do not use dot density\n");
    break;
  case 0:
    foutput("           Use half density\n");
    break;
  default:
    foutput("           Use dot density of %d\n", i);
    break;
  }

  switch ( i = get_short()) {
  case 0:
    foutput("           No merge tick marks\n");
    break;
  default:
    foutput("           Merge tick marks\n");
    break;
  }
}


/* ************************************************************************* */
/* Define Dither VCGL Command Processor                                      */
/* ************************************************************************* */

static define_dither()
{
  short i;

  if ((val_words = get_val()) != 5) {
    woutput("WARNING: Define Dither word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_DEFINE_DITHER);
  foutput("%6d DEFINE DITHER COLOR,         ccnn = %X%02X (%ld)\n",
          count[P_DEFINE_DITHER], last_vcgl_cmd, val_words, val_words);
  foutput("           Color Index           = %d\n", get_short());
  if ((i = get_short()) != -1)
    foutput("           Black   Pattern Index = %d\n", i);
  else
    foutput("           Black   Pattern Index = Null Pattern\n");
  if ((i = get_short()) != -1)
    foutput("           Cyan    Pattern Index = %d\n", i);
  else
    foutput("           Cyan    Pattern Index = Null Pattern\n");
  if ((i = get_short()) != -1)
    foutput("           Magenta Pattern Index = %d\n", i);
  else
    foutput("           Magenta Pattern Index = Null Pattern\n");
  if ((i = get_short()) != -1)
    foutput("           Yellow  Pattern Index = %d\n", i);
  else
    foutput("           Yellow  Pattern Index = Null Pattern\n");
}

/* ************************************************************************ */
/* Define RGB Color VCGL Command Processor                                  */
/* ************************************************************************ */

static define_RGB()
{
  if ((val_words = get_val()) != 3) {
    woutput("WARNING - Define RGB word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_DEFINE_RGB);
  foutput("%6d DEFINE RGB COLOR,            ccnn = %X%02X (%ld)\n",
          count[P_DEFINE_RGB], last_vcgl_cmd, val_words, val_words);
  foutput("           Color Index = %d\n", get_short());
  foutput("           Red   Intensity = %d\n",((unsigned char)get_byte(TRUE)));
  foutput("           Green Intensity = %d\n",((unsigned char)get_byte(TRUE)));
  foutput("           Blue  Intensity = %d\n",((unsigned char)get_byte(TRUE)));
  get_byte(FALSE);                   /* Discard Extra Byte */
}


/* ************************************************************************* */
/* Define Pattern VCGL Command Processor                                     */
/* ************************************************************************* */

static define_pattern()
{
  unsigned short i;

  val_words = get_val();        /* Get number of words */
  output_ck(P_DEFINE_PATTERN);
  foutput("%6d DEFINE PATTERN,              ccnn = %X%02X (%ld)\n",
          count[P_DEFINE_PATTERN], last_vcgl_cmd, val_words, val_words);
  foutput("           Pattern Index  = %d\n", get_short());
  foutput("           Pattern Height = %d\n", get_short());
  foutput("           Pattern Width  = %d\n", get_short());
  while (val_words > 0) {
    i = get_short();
    foutput("           Pattern value  = 0x%04X\n", i);
  }
}

/* ************************************************************************* */
/* Define Raster Stamp VCGL Command Processor                                */
/* ************************************************************************* */

static define_raster_stamp()
{
  val_words = get_val();        /* Get number of words */
  output_ck(P_DEFINE_RASTER_STAMP);
  foutput("%6d DEFINE RASTER STAMP,         ccnn = %X%02X (%ld)\n",
          count[P_DEFINE_RASTER_STAMP], last_vcgl_cmd, val_words, val_words);
  foutput("           Stamp Index  = %d\n", get_short());
  foutput("           Stamp Height = %d\n", get_short());
  foutput("           Stamp Width  = %d\n", get_short());
  dump_data();
}

/* ************************************************************************ */
/* Define Font VCGL Command Processor                                       */
/* ************************************************************************ */

static define_font()
{
  short i;
  short j;
  short k;
  short sym_count;
  short prop_flag;
  short const_count;
  short s;

  val_words = get_val();        /* Get number of words */
  output_ck(P_DEFINE_FONT);
  foutput("%6d DEFINE FONT,                 ccnn = %X%02X (%ld)\n",
          count[P_DEFINE_FONT], last_vcgl_cmd, val_words, val_words);
  if ((i = get_short()) < 31) {
    woutput("WARNING - Font index = %d\n", i);
    err_count++;
  } else {
    foutput("           Font Index  = %d\n", i);
    sym_count = get_short();
    foutput("           Count of symbols defined = %d\n", sym_count);
    foutput("           Maximum symbol index used = %d\n", get_short());
    foutput("           Maximum grid height used  = %d\n", get_short());
    foutput("           Maximum grid width  used  = %d\n", get_short());
    if ((prop_flag = get_short()) == 0)
      foutput("           Fixed Character Spacing\n");
    else
      foutput("           Proportional Character Spacing\n");
    foutput("                Bottomline   alignment = %d\n", get_short());
    foutput("                Baseline     alignment = %d\n", get_short());
    foutput("                Halfline     alignment = %d\n", get_short());
    foutput("                Capline      alignment = %d\n", get_short());
    foutput("                Topline      alignment = %d\n", get_short());
    foutput("                Left Extent  alignment = %d\n", get_short());
    foutput("                Left Body    alignment = %d\n", get_short());
    foutput("                Center       alignment = %d\n", get_short());
    foutput("                Right Body   alignment = %d\n", get_short());
    foutput("                Right Extent alignment = %d\n", get_short());
    while (val_words > 0) {
      foutput("           Symbol Index = %d\n", get_short());
      const_count = get_short();
      foutput("           Count of constructs = %d\n", const_count);
      if (prop_flag != 0) {
        foutput("           Left Extent Alignment = %d\n", get_short());
        foutput("           Left Body Alignment   = %d\n", get_short());
        foutput("           Center      Alignment = %d\n", get_short());
        foutput("           Right Body Alignment  = %d\n", get_short());
        foutput("           Right Extent Alignment= %d\n", get_short());
      }
      while (const_count) {
        switch (k = get_short()) {
        case 1:
          foutput("           Polyline\n");
          break;
        case 2:
          foutput("           Polygon\n");
          break;
        default:
          woutput("WARNING: unknown Symbol Construct = %d\n", k);
          err_count++;
          break;
        }
        j = get_short();        /* Get number of words in construct */
        foutput("           Number of words in construct = %d\n",
                j);
        if ((j & 1) != 0) {
          woutput("WARNING: Odd number of words in Symbol Definition\n");
          err_count++;
        }
        j = j >> 1;             /* In number of coord pairs */
        while (j) {
          s = get_short();
          foutput("           X,Y coordinate = (%d,%d)\n", s, get_short());
          j--;
        }
        const_count--;
      }
    }
  }
}



/* ************************************************************************ */
/* Define Marker Set VCGL Command Processor                                 */
/* ************************************************************************ */

static define_marker()
{
  short j;
  short k;
  short mrk_count;
  short const_count;
  short s;

  val_words = get_val();        /* Get number of words */
  output_ck(P_DEFINE_MARKER);
  foutput("%6d DEFINE MARKER SET,           ccnn = %X%02X (%ld)\n",
          count[P_DEFINE_MARKER], last_vcgl_cmd, val_words, val_words);
  mrk_count = get_short();
  foutput("           Count of markers defined = %d\n", mrk_count);
  foutput("           Maximum markers index used = %d\n", get_short());
  foutput("           Maximum grid height used  = %d\n", get_short());
  foutput("           Maximum grid width  used  = %d\n", get_short());
  foutput("           Marker  grid size         = %d\n", get_short());
  while (val_words > 0) {
    foutput("           Marker Index = %d\n", get_short());
    const_count = get_short();
    foutput("           Count of constructs = %d\n", const_count);
    foutput("           Halfline alignment coord = %d\n", get_short());
    foutput("           Center alignment coord   = %d\n", get_short());
    while (const_count) {
      switch (k = get_short()) {
      case 1:
        foutput("           Polyline\n");
        break;
      case 2:
        foutput("           Polygon\n");
        break;
      default:
        woutput("WARNING: unknown Marker Construct = %d\n", k);
        err_count++;
        break;
      }
      j = get_short();          /* Get number of words in construct */
      foutput("           Number of words in construct = %d\n", j);
      if ((j & 1) != 0) {
        woutput("WARNING: Odd number of words in Marker Definition\n");
        err_count++;
      }
      j = j >> 1;               /* In number of coord pairs */
      while (j) {
        s = get_short();
        foutput("           X,Y coordinate = (%d,%d)\n", s, get_short());
        j--;
      }
      const_count--;
    }
  }
}


/* ************************************************************************ */
/* Define Dash VCGL Command Processor                                       */
/* ************************************************************************ */

static define_dash()
{
  short j;

  j = 0;
  val_words = get_val();        /* Get number of words */
  output_ck(P_DEFINE_DASH);
  foutput("%6d DEFINE DASH STYLE,           ccnn = %X%02X (%ld)\n",
          count[P_DEFINE_DASH], last_vcgl_cmd, val_words, val_words);
  foutput("           Dash Index  = %d\n", get_short());
  while (val_words > 0) {
    if (j == 0)
      foutput("           Length of dash  = %d\n", get_short());
    else
      foutput("           Length of space = %d\n", get_short());
    j = (j + 1) & 1;
  }
}



/****************************************************************************/
/* Define Pattern Coverage VCGL Command Processor                           */
/****************************************************************************/

static define_coverages()
{
  short j;

  j = 1;
  val_words = get_val();        /* Get number of words */
  output_ck(P_DEFINE_COVERAGES);
  foutput("%6d DEFINE PATTERN COVERAGES,    ccnn = %X%02X (%ld)\n",
          count[P_DEFINE_COVERAGES], last_vcgl_cmd, val_words, val_words);
  foutput("           Primary Flags   = 0x%04X\n", get_short());
  while (val_words > 0) {
    if ((j & 1) == 1)
      foutput("           Pattern Index %4d = %4d,", j, get_short());
    else
      foutput("      Area  = %4d\n", ((unsigned short) get_short()));
    j++;
  }
}



/* ************************************************************************ */
/* Define Primary Constants VCGL Command Processor                          */
/* ************************************************************************ */

static define_primary_constants()
{
  if ((val_words = get_val()) != 0x18) {
    woutput("WARNING: define primary constants word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_DEFINE_PRIMARY_CONSTANTS);
  foutput("%6d DEFINE PRIMARY CONSTANTS,    ccnn = %X%02X (%ld)\n",
     count[P_DEFINE_PRIMARY_CONSTANTS], last_vcgl_cmd, val_words, val_words);
  foutput("           black x   = %d\n", ((unsigned short) get_short()));
  foutput("           black y   = %d\n", ((unsigned short) get_short()));
  foutput("           black z   = %d\n", ((unsigned short) get_short()));
  foutput("           cyan  x   = %d\n", ((unsigned short) get_short()));
  foutput("           cyan  y   = %d\n", ((unsigned short) get_short()));
  foutput("           cyan  z   = %d\n", ((unsigned short) get_short()));
  foutput("           magenta x = %d\n", ((unsigned short) get_short()));
  foutput("           magenta y = %d\n", ((unsigned short) get_short()));
  foutput("           magenta z = %d\n", ((unsigned short) get_short()));
  foutput("           yellow x  = %d\n", ((unsigned short) get_short()));
  foutput("           yellow y  = %d\n", ((unsigned short) get_short()));
  foutput("           yellow z  = %d\n", ((unsigned short) get_short()));
  foutput("           red    x  = %d\n", ((unsigned short) get_short()));
  foutput("           red    y  = %d\n", ((unsigned short) get_short()));
  foutput("           red    z  = %d\n", ((unsigned short) get_short()));
  foutput("           green  x  = %d\n", ((unsigned short) get_short()));
  foutput("           green  y  = %d\n", ((unsigned short) get_short()));
  foutput("           green  z  = %d\n", ((unsigned short) get_short()));
  foutput("           blue   x  = %d\n", ((unsigned short) get_short()));
  foutput("           blue   y  = %d\n", ((unsigned short) get_short()));
  foutput("           blue   z  = %d\n", ((unsigned short) get_short()));
  foutput("           white  x  = %d\n", ((unsigned short) get_short()));
  foutput("           white  y  = %d\n", ((unsigned short) get_short()));
  foutput("           white  z  = %d\n", ((unsigned short) get_short()));
}


/* ************************************************************************ */
/* Define Monitor Constants VCGL Command Processor                          */
/* ************************************************************************ */

static define_monitor_constants()
{
  if ((val_words = get_val()) != 0x0c) {
    woutput("WARNING: define monitor constants word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_DEFINE_MONITOR_CONSTANTS);
  foutput("%6d DEFINE MONITOR CONSTANTS,    ccnn = %X%02X (%ld)\n",
     count[P_DEFINE_MONITOR_CONSTANTS], last_vcgl_cmd, val_words, val_words);
  foutput("           red    x  = %d\n", ((unsigned short) get_short()));
  foutput("           red    y  = %d\n", ((unsigned short) get_short()));
  foutput("           red    z  = %d\n", ((unsigned short) get_short()));
  foutput("           green  x  = %d\n", ((unsigned short) get_short()));
  foutput("           green  y  = %d\n", ((unsigned short) get_short()));
  foutput("           green  z  = %d\n", ((unsigned short) get_short()));
  foutput("           blue   x  = %d\n", ((unsigned short) get_short()));
  foutput("           blue   y  = %d\n", ((unsigned short) get_short()));
  foutput("           blue   z  = %d\n", ((unsigned short) get_short()));
  foutput("           white  x  = %d\n", ((unsigned short) get_short()));
  foutput("           white  y  = %d\n", ((unsigned short) get_short()));
  foutput("           white  z  = %d\n", ((unsigned short) get_short()));
}

/* ************************************************************************ */
/* Define Gamma Correction VCGL Command Processor                           */
/* ************************************************************************ */

static define_gamma_correction()
{
  if ((val_words = get_val()) != 3) {
    woutput("WARNING: define gamma correction word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_DEFINE_GAMMA_CORRECTION);
  foutput("%6d DEFINE GAMMA CORRECTION,     ccnn = %X%02X (%ld)\n",
      count[P_DEFINE_GAMMA_CORRECTION], last_vcgl_cmd, val_words, val_words);
  foutput("           red   gamma = %d\n", get_short());
  foutput("           green gamma = %d\n", get_short());
  foutput("           blue  gamma = %d\n", get_short());
}

/****************************************************************************/
/* Set Edge Visibility VCGL Command Processor                               */
/****************************************************************************/

static set_edge_visibility()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set edge visibility word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_EDGE_VISIBILITY);
  foutput("%6d SET EDGE VISIBILITY,         ccnn = %X%02X (%ld)\n",
          count[P_SET_EDGE_VISIBILITY], last_vcgl_cmd, val_words, val_words);
  if (get_short() == 0)
    foutput("           Outlines of filled areas NOT drawn\n");
  else
    foutput("           Outlines of filled areas WILL BE drawn\n");
}


/* ************************************************************************* */
/* Set Line Width VCGL Command Processor                                     */
/* ************************************************************************* */

static set_line_width()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set line width word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_LINE_WIDTH);
  foutput("%6d SET LINE WIDTH,              ccnn = %X%02X (%ld)\n",
          count[P_SET_LINE_WIDTH], last_vcgl_cmd, val_words, val_words);
  foutput("           Width = %d\n", get_short());
}

/* ************************************************************************ */
/* Set Line Color VCGL Command Processor                                    */
/* ************************************************************************ */

static set_line_color()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set line color word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_LINE_COLOR);
  foutput("%6d SET LINE COLOR,              ccnn = %X%02X (%ld)\n",
          count[P_SET_LINE_COLOR], last_vcgl_cmd, val_words, val_words);
  foutput("           Color = %d\n", get_short());
}

/* ************************************************************************* */
/* Set Dash Wrap Style VCGL Command Processor                               */
/* ************************************************************************* */

static set_dash_wrap_style()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set dash wrap style word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_DASH_WRAP_STYLE);
  foutput("%6d SET DASH WRAP STYLE,         ccnn = %X%02X (%ld)\n",
          count[P_SET_DASH_WRAP_STYLE], last_vcgl_cmd, val_words, val_words);
  if (get_short())
    foutput("           Wrap Around Dash Style\n");
  else
    foutput("           Normal Wrap Style\n");
}


/* ************************************************************************ */
/* Set Line Dash Index VCGL Command Processor                               */
/* ************************************************************************ */

static set_line_dash_index()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set line dash index word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_LINE_DASH_INDEX);
  foutput("%6d SET LINE DASH INDEX,         ccnn = %X%02X (%ld)\n",
          count[P_SET_LINE_DASH_INDEX], last_vcgl_cmd, val_words, val_words);
  foutput("           Line Dash Index = %d\n", get_short());
}

/* ************************************************************************ */
/* Set Line Cap Style VCGL Command Processor                                */
/* ************************************************************************ */

static set_line_cap_style()
{
  short i;

  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set line cap style word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_LINE_CAP_STYLE);
  foutput("%6d SET LINE CAP STYLE,          ccnn = %X%02X (%ld)\n",
          count[P_SET_LINE_CAP_STYLE], last_vcgl_cmd, val_words, val_words);
  switch (i = get_short()) {
  case 0:
    foutput("           Line Cap Style = Butt\n");
    break;
  case 1:
    foutput("           Line Cap Style = Projecting\n");
    break;
  case 2:
    foutput("           Line Cap Style = Rounded\n");
    break;
  default:
    woutput("WARNING: Unknown Line Cap Style = %d\n", i);
    err_count++;
    break;
  }
}

/* ************************************************************************* */
/* Set Line Joint Style VCGL Command Processor                               */
/* ************************************************************************* */

static set_line_joint_style()
{
  short i;

  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set line joint style word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_LINE_JOINT_STYLE);
  foutput("%6d SET LINE JOINT STYLE,        ccnn = %X%02X (%ld)\n",
          count[P_SET_LINE_JOINT_STYLE], last_vcgl_cmd, val_words, val_words);
  switch (i = get_short()) {
  case 0:
    foutput("           Line Joint Style = Unjoined\n");
    break;
  case 1:
    foutput("           Line Joint Style = Beveled\n");
    break;
  case 2:
    foutput("           Line Joint Style = Mitered\n");
    break;
  case 3:
    foutput("           Line Joint Style = Rounded\n");
    break;
  default:
    woutput("WARNING: Unknown Line Joint Style = %d\n", i);
    err_count++;
    break;
  }
}



/* ************************************************************************* */
/* Set Fill Color VCGL Command Processor                                     */
/* ************************************************************************* */

static set_fill_color()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set fill color word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_FILL_COLOR);
  foutput("%6d SET FILL COLOR,              ccnn = %X%02X (%ld)\n",
          count[P_SET_FILL_COLOR], last_vcgl_cmd, val_words, val_words);
  foutput("           Fill color = %d\n", get_short());
}



/* ************************************************************************* */
/* Set Fill Style VCGL Command Processor                                     */
/* ************************************************************************* */

static set_fill_style()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set fill style word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_FILL_STYLE);
  foutput("%6d SET FILL STYLE,              ccnn = %X%02X (%ld)\n",
          count[P_SET_FILL_STYLE], last_vcgl_cmd, val_words, val_words);
  switch (get_short()) {
  case 0:
    foutput("           Fill not drawn\n");
    break;
  case 1:
    foutput("           Parity fill\n");
    break;
  case 2:
    foutput("           Non-Zero Winding fill\n");
    break;
  }
}

/* ************************************************************************ */
/* Set Line RGB VCGL Command Processor                                      */
/* ************************************************************************ */

static set_line_RGB()
{
  if ((val_words = get_val()) != 2) {
    woutput("WARNING: set line RGB word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_LINE_RGB);
  foutput("%6d SET LINE RGB,                ccnn = %X%02X (%ld)\n",
          count[P_SET_LINE_RGB], last_vcgl_cmd, val_words, val_words);
  foutput("           red, green intensities (rrgg) = 0x%04X\n", get_short());
  foutput("           blue intensities (bb00) = 0x%04X\n", get_short());
}


/* ************************************************************************* */
/* Set FILL RGB VCGL Command Processor                                       */
/* ************************************************************************* */

static set_fill_RGB()
{
  if ((val_words = get_val()) != 2) {
    woutput("WARNING: set fill RGB word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_FILL_RGB);
  foutput("%6d SET FILL RGB,                ccnn = %X%02X (%ld)\n",
          count[P_SET_FILL_RGB], last_vcgl_cmd, val_words, val_words);
  foutput("           red, green intensities (rrgg) = 0x%04X\n", get_short());
  foutput("           blue intensities (bb00) = 0x%04X\n", get_short());
}



/****************************************************************************/
/* Set MARKER BASELINE VCGL Command Processor                               */
/****************************************************************************/

static set_marker_baseline()
{
  if ((val_words = get_val()) != 2) {
    woutput("WARNING: set marker baseline word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_MARKER_BASELINE);
  foutput("%6d SET MARKER BASELINE,         ccnn = %X%02X (%ld)\n",
          count[P_SET_MARKER_BASELINE], last_vcgl_cmd, val_words, val_words);
  foutput("           X baseline component = %d\n", get_short());
  foutput("           Y baseline component = %d\n", get_short());
}


/* ************************************************************************* */
/* Set TEXT ORIENTATION VCGL Command Processor                               */
/* ************************************************************************* */

static set_text_orientation()
{
  short s;

  if ((val_words = get_val()) != 4) {
    woutput("WARNING - set text orientation word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_TEXT_ORIENTATION);
  foutput("%6d SET TEXT ORIENTATION,        ccnn = %X%02X (%ld)\n",
          count[P_SET_TEXT_ORIENTATION], last_vcgl_cmd, val_words, val_words);
  s = get_short();
  foutput("           Height vector = %d, %d\n", s, get_short());
  s = get_short();
  foutput("           Width  vector = %d, %d\n", s, get_short());
}



/* ************************************************************************* */
/* Set TEXT BASELINE VCGL Command Processor                                  */
/* ************************************************************************* */

static set_text_baseline()
{
  if ((val_words = get_val()) != 2) {
    woutput("WARNING: set text baseline word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_TEXT_BASELINE);
  foutput("%6d SET TEXT BASELINE,           ccnn = %X%02X (%ld)\n",
          count[P_SET_TEXT_BASELINE], last_vcgl_cmd, val_words, val_words);
  foutput("           X baseline component = %d\n", get_short());
  foutput("           Y baseline component = %d\n", get_short());
}


/* ************************************************************************ */
/* Set TEXT ALIGNMENT VCGL Command Processor                                */
/* ************************************************************************ */

static set_text_alignment()
{
  short i;

  if ((val_words = get_val()) != 1) {
    woutput("WARNING - set text alignment word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_TEXT_ALIGNMENT);
  foutput("%6d SET TEXT ALIGNMENT,          ccnn = %X%02X (%ld)\n",
          count[P_SET_TEXT_ALIGNMENT], last_vcgl_cmd, val_words, val_words);
  i = get_short();
  switch (i & 0xff00) {
  case 0x0000:
    foutput("           Align to Character Bottomline\n");
    break;
  case 0x0100:
    foutput("           Align to Character Baseline\n");
    break;
  case 0x0200:
    foutput("           Align to Character Halfline\n");
    break;
  case 0x0300:
    foutput("           Align to Character Capline\n");
    break;
  case 0x0400:
    foutput("           Align to Character Topline\n");
    break;
  }
  switch (i & 0xff) {
  case 0:
    foutput("           Align to Character left extent\n");
    break;
  case 1:
    foutput("           Align to Character left body\n");
    break;
  case 2:
    foutput("           Align to Character center\n");
    break;
  case 3:
    foutput("           Align to Character right body\n");
    break;
  case 4:
    foutput("           Align to Character right extent\n");
    break;
  }
}


/* ************************************************************************* */
/* Set Text Path VCGL Command Processor                                      */
/* ************************************************************************* */

static set_text_path()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set text path word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_TEXT_PATH);
  foutput("%6d SET TEXT PATH,               ccnn = %X%02X (%ld)\n",
          count[P_SET_TEXT_PATH], last_vcgl_cmd, val_words, val_words);
  switch (get_short()) {
  case 0:
    foutput("           Text Path = RIGHT\n");
    break;
  case 1:
    foutput("           Text Path = LEFT\n");
    break;
  case 2:
    foutput("           Text Path = UP\n");
    break;
  case 3:
    foutput("           Text Path = DOWN\n");
    break;
  }
}


/****************************************************************************/
/* Set Text Spacing VCGL Command Processor                                  */
/****************************************************************************/

static set_text_spacing()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set text spacing word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_TEXT_SPACING);
  foutput("%6d SET TEXT SPACING,            ccnn = %X%02X (%ld)\n",
          count[P_SET_TEXT_SPACING], last_vcgl_cmd, val_words, val_words);
  foutput("           Spacing = %d\n", get_short());
}


/* ************************************************************************* */
/* Set FONT VCGL Command Processor                                           */
/* ************************************************************************* */

static set_font()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set font word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_FONT);
  foutput("%6d SET FONT,                    ccnn = %X%02X (%ld)\n",
          count[P_SET_FONT], last_vcgl_cmd, val_words, val_words);
  foutput("           Font Index = %d\n", get_short());
}


/* ************************************************************************* */
/* Set OPAQUE VCGL Command Processor                                         */
/* ************************************************************************* */

static set_opaque()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set opaque word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_OPAQUE);
  foutput("%6d SET OPAQUE,                  ccnn = %X%02X (%ld)\n",
          count[P_SET_OPAQUE], last_vcgl_cmd, val_words, val_words);
  if (get_short() == 0)
    foutput("           Opaque Objects are OFF\n");
  else
    foutput("           Opaque Objects are ON\n");
}

/* ************************************************************************* */
/* Set LEVEL VCGL Command Processor                                          */
/* ************************************************************************* */

static set_level()
{
  if ((val_words = get_val()) != 1) {
    woutput("WARNING: set level word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_LEVEL);
  foutput("%6d SET LEVEL,                   ccnn = %X%02X (%ld)\n",
          count[P_SET_LEVEL], last_vcgl_cmd, val_words, val_words);
  foutput("           Set current level to %d\n", get_short());
}


/* ************************************************************************* */
/* Set CLIP VIEWPORT VCGL Command Processor                                  */
/* ************************************************************************* */

static set_clip_viewport()
{
  if ((val_words = get_val()) != 8) {
    woutput("WARNING: set clip viewport word count = %d\n", val_words);
    err_count++;
  }
  output_ck(P_SET_CLIP_VIEWPORT);
  foutput("%6d SET CLIP VIEWPORT,           ccnn = %X%02X (%ld)\n",
          count[P_SET_CLIP_VIEWPORT], last_vcgl_cmd, val_words, val_words);
  foutput("           Minimum X boundary = %6ld\n", get_long());
  foutput("           Minimum Y boundary = %6ld\n", get_long());
  foutput("           Maximum X boundary = %6ld\n", get_long());
  foutput("           Maximum Y boundary = %6ld\n", get_long());
}


/****************************************************************************/
/* Draw POLYLINE VCGL Command Processor                                     */
/****************************************************************************/

static draw_polyline()
{
  long x2;
  long y2;
  long dx;
  long dy;

  val_words = get_val();
  output_ck(P_DRAW_POLYLINE);
  foutput("%6d DRAW POLYLINE,               ccnn = %X%02X (%ld)\n",
          count[P_DRAW_POLYLINE], last_vcgl_cmd, val_words, val_words);
  x2 = get_x_value();
  y2 = get_y_value();
  foutput("           Starting X,Y     = %6ld, %6ld\n", x2, y2);
  while (val_words > 0) {
    dx = get_rel_value();
    dy = get_rel_value();
    x2 = x2 + dx;
    y2 = y2 + dy;
    foutput("           Delta X,Y        = ");
    foutput("%6ld, %6ld  (Abs X,Y = %6ld,%6ld)\n", dx, dy, x2, y2);
  }
}

/* ************************************************************************* */
/* Draw TEXT VCGL Command Processor                                          */
/* ************************************************************************* */

static draw_text()
{
  long x1;
  long y1;
  short i;
  char  c;
  short txt_count;

  val_words = get_val();
  output_ck(P_DRAW_TEXT);
  foutput("%6d DRAW TEXT,                   ccnn = %X%02X (%ld)\n",
          count[P_DRAW_TEXT], last_vcgl_cmd, val_words, val_words);
  x1 = get_x_value();
  y1 = get_y_value();
  txt_count = get_short();
  foutput("           Character txt_count = %d\n", txt_count);
  foutput("           Starting X,Y        = %6ld, %6ld\n", x1, y1);
  if (type > 3) {
    while (val_words > 0) {
      i = get_short();
      foutput("           Character Index     =  0x%04X\n",i);
      txt_count--;
    }
  } else {
    while (val_words > 0 || (val_words == 0 && odd_byte_flag)) {
      c = get_byte(TRUE);
      foutput("           Character Index     =  0x%02X\n",c);
      txt_count--;
    }
  }
  foutput("\n");
  if (txt_count != 0 && txt_count != -1) {
    woutput("WARNING: Draw Text - Character count was off by %d\n",
            txt_count);
    err_count++;
  }
}


/* ************************************************************************* */
/* Draw POLYMARKER VCGL Command Processor                                    */
/* ************************************************************************* */

static draw_polymarker()
{
  long x2;
  long y2;
  long dx;
  long dy;

  val_words = get_val();
  output_ck(P_DRAW_POLYMARKER);
  foutput("%6d DRAW POLYMARKER,             ccnn = %X%02X (%ld)\n",
          count[P_DRAW_POLYMARKER], last_vcgl_cmd, val_words, val_words);
  foutput("           Marker Index = %d\n", get_short());
  x2 = get_x_value();
  y2 = get_y_value();
  foutput("           Starting X,Y = %6ld, %6ld\n", x2, y2);
  while (val_words > 0) {
    dx = get_rel_value();
    dy = get_rel_value();
    x2 = x2 + dx;
    y2 = y2 + dy;
    foutput("           Delta X,Y    = ");
    foutput("%6ld, %6ld  (Abs X,Y = %6ld,%6ld)\n", dx, dy, x2, y2);
  }
}


/* ************************************************************************* */
/* Draw POLYGON VCGL Command Processor                                       */
/* ************************************************************************* */

static draw_polygon()
{
  long x2;
  long y2;
  long dx;
  long dy;

  val_words = get_val();
  output_ck(P_DRAW_POLYGON);
  foutput("%6d DRAW POLYGON,                ccnn = %X%02X (%ld)\n",
          count[P_DRAW_POLYGON], last_vcgl_cmd, val_words, val_words);
  x2 = get_x_value();
  y2 = get_y_value();
  foutput("           Starting X,Y = %6ld, %6ld\n", x2, y2);
  while (val_words > 0) {
    dx = get_rel_value();
    dy = get_rel_value();
    x2 = x2 + dx;
    y2 = y2 + dy;
    foutput("           Delta X,Y    = ");
    foutput("%6ld, %6ld  (Abs X,Y = %6ld,%6ld)\n", dx, dy, x2, y2);
  }
}

/* ************************************************************************* */
/* Draw POLYGON SET VCGL Command Processor                                   */
/* ************************************************************************* */

static draw_polygon_set()
{
  long x2;
  long y2;
  long dx;
  long dy;
  long pol_count;

  val_words = get_val();
  output_ck(P_DRAW_POLYGON_SET);
  foutput("%6d DRAW POLYGON SET,            ccnn = %X%02X (%ld)\n",
          count[P_DRAW_POLYGON_SET], last_vcgl_cmd, val_words, val_words);
  while (val_words > 0) {
    pol_count = get_long() - 1;
    x2 = get_x_value();
    y2 = get_y_value();
    foutput("           Starting X,Y = %6ld, %6ld\n", x2, y2);
    while (pol_count-- > 0) {
      dx = get_rel_value();
      dy = get_rel_value();
      x2 = x2 + dx;
      y2 = y2 + dy;
      foutput("           %6ld Delta X,Y    = ",pol_count);
      foutput("%6ld, %6ld  (Abs X,Y = %6ld,%6ld)\n",dx, dy, x2, y2);
    }
  }
}


/* ************************************************************************* */
/* Draw RECTANGLE VCGL Command Processor                                     */
/* ************************************************************************* */

static draw_rectangle()
{
  long x2;
  long y2;
  long dx;
  long dy;

  val_words = get_val();
  output_ck(P_DRAW_RECTANGLE);
  foutput("%6d DRAW RECTANGLE,              ccnn = %X%02X (%ld)\n",
          count[P_DRAW_RECTANGLE], last_vcgl_cmd, val_words, val_words);
  x2 = get_x_value();
  y2 = get_y_value();
  foutput("           Starting X,Y = %6ld, %6ld\n", x2, y2);
  dx = get_rel_value();
  dy = get_rel_value();
  x2 = x2 + dx;
  y2 = y2 + dy;
  foutput("           Delta X,Y    = ");
  foutput("%6ld, %6ld  (Abs X,Y = %6ld,%6ld)\n", dx, dy, x2, y2);
}

/* ************************************************************************* */
/* Draw CIRCLE VCGL Command Processor                                        */
/* ************************************************************************* */

static draw_circle()
{
  long x1;
  long y1;

  val_words = get_val();
  output_ck(P_DRAW_CIRCLE);
  foutput("%6d DRAW CIRCLE,                 ccnn = %X%02X (%ld)\n",
          count[P_DRAW_CIRCLE], last_vcgl_cmd, val_words, val_words);
  x1 = get_x_value();
  y1 = get_y_value();
  foutput("           Center X,Y = %6ld, %6ld, Diameter = %d\n",
          x1, y1, get_short());
}

/* ************************************************************************* */
/* Draw RASTER STAMP VCGL Command Processor                                  */
/* ************************************************************************* */

static draw_raster_stamp()
{
  long x1;
  long y1;

  val_words = get_val();
  output_ck(P_DRAW_RASTER_STAMP);
  foutput("%6d DRAW RASTER STAMP,           ccnn = %X%02X (%ld)\n",
          count[P_DRAW_RASTER_STAMP], last_vcgl_cmd, val_words, val_words);
  x1 = get_x_value();
  y1 = get_y_value();
  foutput("           Low X,Y = %6ld, %6ld, index = %d\n",
          x1, y1, get_short());
}


/* ************************************************************************ */
/* Draw CELL ARRAY VCGL Command Processor                                   */
/* ************************************************************************ */

static draw_cell_array()
{
  long l;
  short s;

  val_words = get_val();
  output_ck(P_DRAW_CELL_ARRAY);
  foutput("%6d DRAW CELL ARRAY,             ccnn = %X%02X (%ld)\n",
          count[P_DRAW_CELL_ARRAY], last_vcgl_cmd, val_words, val_words);
  l = get_long();
  foutput("           X,Y Alignment Point = %6ld, %6ld\n", l, get_long());
  foutput("           dx, dy = %6ld, %6ld\n", get_long(), get_long());
  s = get_short();
  foutput("           # columns = %d, # rows = %d\n", s, get_short());
  dump_data();
}


/* ************************************************************************* */
/* Draw RGB CELL ARRAY VCGL Command Processor                                */
/* ************************************************************************* */

static draw_RGB_cell_array()
{
  long l;
  short s;

  val_words = get_val();
  output_ck(P_DRAW_RGB_CELL_ARRAY);
  foutput("%6d DRAW RGB CELL ARRAY,         ccnn = %X%02X (%ld)\n",
          count[P_DRAW_RGB_CELL_ARRAY], last_vcgl_cmd, val_words, val_words);
  l = get_long();
  foutput("           X,Y Alignment Point = %6ld, %6ld\n", l, get_long());
  foutput("           dx, dy = %6ld, %6ld\n", get_long(), get_long());
  s = get_short();
  foutput("           # columns = %d, # rows = %d\n", s, get_short());
  dump_data();
}


/****************************************************************************/
/* Draw RUN LENGTH VCGL Command Processor                                   */
/****************************************************************************/

static draw_run_length()
{
  long l;

  val_words = get_val();
  output_ck(P_DRAW_RUN_LENGTH);
  foutput("%6d DRAW RUN LENGTH,             ccnn = %X%02X (%ld)\n",
          count[P_DRAW_RUN_LENGTH], last_vcgl_cmd, val_words, val_words);
  l = get_long();
  foutput("           X,Y Alignment Point = %6ld, %6ld\n", l, get_long());
  dump_data();
}

/* ************************************************************************* */
/* Draw RUN RGB LENGTH VCGL Command Processor                                */
/* ************************************************************************* */

static draw_RGB_run_length()
{
  long l;

  val_words = get_val();
  output_ck(P_DRAW_RGB_RUN_LENGTH);
  l = get_long();
  foutput("%6d DRAW RGB RUN LENGTH,         ccnn = %X%02X (%ld)\n",
          count[P_DRAW_RGB_RUN_LENGTH], last_vcgl_cmd, val_words, val_words);
  foutput("           X,Y Alignment Point = %6ld, %6ld\n", l, get_long());
  dump_data();
}


/* ************************************************************************* */
/* Draw CMY RASTER VCGL Command Processor                                    */
/* ************************************************************************* */

static draw_CMY_raster()
{
  long l;

  val_words = get_val();
  output_ck(P_DRAW_CMY_RASTER);
  foutput("%6d DRAW CMY RASTER,             ccnn = %X%02X (%ld)\n",
          count[P_DRAW_CMY_RASTER], last_vcgl_cmd, val_words, val_words);
  l = get_long();
  foutput("           X,Y Alignment Point = %6ld, %6ld\n",
          l, get_long());
  l = get_long();
  foutput("           raster dx,dy = %6ld, %6ld\n", l, get_long());
  foutput("           mask    data primary flag = 0x%04X\n", get_short());
  foutput("           black   data primary flag = 0x%04X\n", get_short());
  foutput("           cyan    data primary flag = 0x%04X\n", get_short());
  foutput("           magenta data primary flag = 0x%04X\n", get_short());
  foutput("           yellow  data primary flag = 0x%04X\n", get_short());
  dump_data();
}
#define _IN_GETS

/* ************************************************************************* */
/* vcgl_gets.c --  Routines to get VCGL data in various forms                */
/*                 such as SHORT, CHAR, LONG, etc.                           */
/*                                                                           */
/* This code was written by:                                                 */
/*                           Brian Jones                                     */
/*                           VerDak Graphics Software & Systems              */
/*                           versatc!brian                                   */
/*                           October 25, 1988                                */
/*                                                                           */
/* (C) Copyright 1988, Versatec Inc., All Rights Reserved                    */
/*                                                                           */
/* ************************************************************************* */

/* char sccs_gets[] = vcgl_gets.c 1.15 12/9/88 */

/* The following routines are available for the VCGL data processing         */
/* functions:                                                                */
/*    void           get_init()  -> Called once from main routine to init    */
/*    unsigned short get_cmd()   -> Called to get a VCGL command byte        */
/*    char           get_val()   -> Called to get a VCGL command word count  */
/*    char           get_byte()  -> Called to get a byte (limited checking)  */
/*    short          get_short() -> Called to get a short's worth of data    */
/*    long           get_long()  -> Called to get a long's worth of data     */
/*    long           get_x_value(type) -> Called to get an X value           */
/*    long           get_y_value(type) -> Called to get an Y value           */
/*    long           get_rel_value(type) -> Called to get a rel value        */
/*                                                                           */



static short vcgl_form;                  /* Wordcount format of VCGL command */
#define VERY_LONG_FORM          1        /* 32 bits of wordcount             */
#define LONG_FORM               2        /* 16 bits of wordcount             */
#define SHORT_FORM              3        /* 8 bits of wordcount              */

static short    stop_output_flag;        /* Suspend dummy 0 data warnings    */
static short   *buf_ptr;                 /* Pointer to current buffer pos    */
static int      buf_words;               /* Valid word count in input buffer */

#define BUFFER_SIZE 16383
static short buffer[BUFFER_SIZE];         /* The input buffer                */
static short save_word;                   /* Save word for byte accesses     */
static FILE *in_stream;                   /* Input stream pointer            */


/***************************************************************************/
/* Get Init Function                                                       */
/***************************************************************************/

/* This function is used to initialize the input related variables    */

static void get_init(source_stream)
FILE *source_stream;
{
  short i;

  print_sum = 0;
  val_words = 0;
  eof_found = 0;
  err_count = 0;
  byte_offset = 0;
  odd_byte_flag = 0;
  cmd_byte_offset = 0;
  end_frame_found = 0;
  stop_output_flag = 0;
  frame_sync_found = 0;
  in_stream = source_stream;
  buf_words = 0;
  for (i = 0; i < NUM_VCGL_COMMANDS; i++) {
    count[i] = 0L;
    count_old[i] = 0L;
    start_output[i] = 0L;
    end_output[i] = 4000000L;
  }
}


/****************************************************************************/
/* Get Buffer function                                                      */
/****************************************************************************/

/* This function is used to read in a new buffer of input data        */

static get_buffer()
{
  static int swap_flag = 0;
  static int unlocked = 1, swap_correct,vms_spooled = 0;
  int vms_rec = 1, vms_off = 1;

  buf_ptr = buffer;

  if ((buf_words = F_READ(buffer,1,sizeof(short)*BUFFER_SIZE,in_stream)) < 1) {
    if ( frame_sync_found != 0 )
      printf("Unexpected end of file\n");
    if ( print_sum ) summary();
    exit();
  }

  addr(&vms_rec,&vms_off);

  if ((buf_words%2) == 1) {
    printf("Odd byte boundary\n");
    if ( print_sum ) summary();
    exit(1);
  }

  if (unlocked) {
    unlocked = 0;
    switch(buffer[0]&0xffff) {

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
        if ( buf_words == 2 ) {
          swap_correct = 1;
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
        if ( buf_words == 2 ) vms_spooled++;
      break;

      case 0x0011:
      case 0x0012:
      case 0x0091:
      case 0x0092:
        swap_correct = 1;
        vms_spooled++;
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
      break;
      default:
        swap_correct = 0;
      break;
    }
  }
  if ( vms_spooled ) {
    if ( swap_correct == 0 ) {
      switch(buffer[0]&0xffff) {
        case 0x9100:
          swap_flag = 1;
          buf_ptr++;
          buf_words -= 2;
          byte_offset += 2;
        break;
        case 0x1100:
          swap_flag = 0;
          buf_ptr++;
          buf_words -= 2;
          byte_offset += 2;
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
          if ( buf_words == 2 ) {
            buf_ptr++;
            buf_words -= 2;
            byte_offset += 2;
          }
        break;
        case 0x9200:
          swap_flag = 1;
          buf_ptr++;
          buf_words -= 2;
          byte_offset += 2;
        break;
        case 0x1200:
          swap_flag = 0;
          buf_ptr++;
          buf_words -= 2;
          byte_offset += 2;
        break;
        default:  
          vms_rec = 0;
          vms_off = 0;
          addr(&vms_rec,&vms_off);
          printf("\nbyte 0x%X, record %d\n",byte_offset,vms_rec);
          printf("           WARNING: expecting VMS spooler control code\n");
        break;
      }
    } else {
      switch(buffer[0]&0xffff) {
        case 0x0091:
          swap_flag = 1;
          buf_ptr++;
          buf_words -= 2;
          byte_offset += 2;
        break;
        case 0x0011:
          swap_flag = 0;
          buf_ptr++;
          buf_words -= 2;
          byte_offset += 2;
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
          if ( buf_words == 2 ) {
            buf_ptr++;
            buf_words -= 2;
            byte_offset += 2;
          }
        break;
        case 0x0012:
          swap_flag = 0;
          buf_ptr++;
          buf_words -= 2;
          byte_offset += 2;
        break;
        case 0x0092:
          swap_flag = 1;
          buf_ptr++;
          buf_words -= 2;
          byte_offset += 2;
        break;
        default:  
          vms_rec = 0;
          vms_off = 0;
          addr(&vms_rec,&vms_off);
          printf("\nbyte 0x%X, record %d\n",byte_offset,vms_rec);
          printf("           WARNING: expecting VMS spooler control code\n");
        break;
      }
    }
  }
  if ( swap_flag || swap_correct )
    if ( swap_flag != swap_correct) swap(buf_ptr,buf_words); 
  buf_words /= 2;
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


/****************************************************************************/
/* Get Byte function                                                        */
/****************************************************************************/

/* This function is used to read in a byte from the input stream      */
/* If the parameter is non-zero, then a check is performed to see     */
/* if enough data of the current VCGL command remains.                */

static char 
get_byte(val_words_check)
  short val_words_check; 

 /* routines and by the vcgl_parser.   */
 /* The latter needs val_words (the    */
 /* number of words left in the        */
 /* command) checked, the former       */
 /* does not...hence the parameter.    */
{
  /* This routine gets a byte from the current word...decrement's   */
  /* val_words only when the last byte has been taken out...        */
  /* note that get_short and other get_... routines should          */
  /* check to see if things were left on an odd byte boundary       */
  /* and complain if so.                                            */

  char i;

  if (val_words_check && val_words <= 0 && !odd_byte_flag) {
    if (stop_output_flag++ > 1) {       /* Messages Once/Cmd   */
      printf("*** WARNING - Attempting to read into next command\n");
      err_count++;
      printf("    Last VCGL command = 0x%X\n", last_vcgl_cmd);
      printf("    Command was at byte offset %d (0x%X)\n",
              cmd_byte_offset, cmd_byte_offset);
      printf("    Currently at byte offset   %d (0x%X)\n",
              byte_offset, byte_offset);
      printf("    Returning a Dummy 0 word\n");
    }
    return (0);                 /* Return a dummy 0 byte */
  }
  if (buf_words < 1)            /* If no words in buffer then get more */
    get_buffer();
  if (odd_byte_flag)            /* If odd byte, extract from save word */
    i = save_word & 0xff;       /* Byte to return is odd byte          */
  else {                        /* Else, get a word from the buffer */
    save_word = *buf_ptr++;
    if (val_words_check)
      val_words--;              /* Decrement every even bytes       */
    buf_words--;                /* Decrement words in buffer        */
    i = save_word >> 8;         /* Byte to return is even byte      */
  }

  byte_offset++;                /* Increment bytes in stream        */
  odd_byte_flag = ((odd_byte_flag + 1) & 1);
  if (debug_level > 5)
    printf("get_byte returning %X (odd_byte_flag = %X)\n",
            i, odd_byte_flag);
  return (i);
}

/****************************************************************************/
/* Get Short function                                                       */
/****************************************************************************/

static short 
get_short()
{
  /* The basic idea of this function is to return a short value */
  /* from the input stream for as long as the global val_words  */
  /* is positive...If val_words goes negative...issue a warning */
  /* message and feed back a dummy 0 without extracting from    */
  /* the input stream...in this way we can re-sync at command   */
  /* boundaries (hopefully).                                    */
  /* */
  /* This routine is written for VCGL analysis and not for      */
  /* speed.  It assumes that something will go wrong (either    */
  /* with the program itself or with the input data.  These     */
  /* get routines are not the ones you should use for normal    */
  /* translators unless you are willing to sacrifice            */
  /* performance for analysis.                                  */


  short i;

  /* Check for off by one byte from where we should be */

  if (odd_byte_flag) {
    printf( "*** WARNING - get_short called with odd_byte_flag TRUE (%X)\n",
            odd_byte_flag);
    err_count++;
    printf("    Last VCGL command = 0x%X\n", last_vcgl_cmd);
    printf("    Command was at byte offset %d (0x%X)\n",
            cmd_byte_offset, cmd_byte_offset);
    printf("    Currently at byte offset   %d (0x%X)\n",
            byte_offset, byte_offset);
    printf("    Discarding odd byte....Continuing\n");
    get_byte(TRUE);             /* Get a byte to throw away */
  }
  /* Check to see if within current command */

  if (val_words > 0) {          /* Words left in the command */
    if (buf_words < 1)
      get_buffer();             /* Get more bytes in buffer  */

    i = *buf_ptr++;             /* Pick up short from buffer */
    buf_words--;                /* Decrement bytes in buffer */
    byte_offset += 2;           /* Increment bytes in stream */
    val_words--;                /* Decrement wd count in command */
  }
  /* More words are being read than the VCGL command specified */
  /* Returning dummy 0 words until the next command...         */

  else {                        /* Too many words read */
    if (stop_output_flag++ > 1) {       /* Messages Once/Cmd   */
      printf("*** WARNING - Attempting to read into next command\n");
      err_count++;
      printf("    Last VCGL command = 0x%X\n", last_vcgl_cmd);
      printf("    Command was at byte offset %d (0x%X)\n",
              cmd_byte_offset, cmd_byte_offset);
      printf("    Currently at byte offset   %d (0x%X)\n",
              byte_offset, byte_offset);
      printf("    Returning a Dummy 0 word\n");
    }
    i = 0;                      /* Spoon feed a dummy zero short back */
  }
  if (debug_level > 5)
    printf("Get Short returning %d (0x%X), val_words = %d\n",
            i, i, val_words);
  return (i);
}

/* ************************************************************************* */
/* Get Long function                                                         */
/* ************************************************************************* */
static long 
get_long()
{

  /* This function returns a long value from the input stream   */
  /* for as long as the global val_words is positive.           */
  /* If val_words goes negative...issue a warning               */
  /* message and feed back a dummy 0 without extracting from    */
  /* the input stream...in this way we can re-sync at command   */
  /* boundaries (hopefully).                                    */

  long i;
  long j;

  /* Check for off by one byte from where we should be */
  if (odd_byte_flag) {
    printf("*** WARNING - get_long called with odd_byte_flag TRUE\n");
    err_count++;
    printf("    Last VCGL command = 0x%X\n", last_vcgl_cmd);
    printf("    Command was at byte offset %d (0x%X)\n",
            cmd_byte_offset, cmd_byte_offset);
    printf("    Currently at byte offset   %d (0x%X)\n",
            byte_offset, byte_offset);
    printf("    Discarding odd byte....Continuing\n");
    get_byte(TRUE);             /* Get a byte to throw away */
  }
  /* Check to see if within current command */
  if (val_words > 0) {          /* Words left in the command */
    i = get_short();
    j = get_short() & 0xffff;
    i = (i << 16) + j;          /* Avoids byte swap problem  */
  }
  /* More words are being read than the VCGL command specified */
  /* Returning dummy 0 words until the next command...         */

  else {                        /* Too many words read */
    if (stop_output_flag++ > 1) {       /* Messages Once/Cmd   */
      printf("*** WARNING - Attempting to read into next command\n");
      err_count++;
      printf("    Last VCGL command = 0x%X\n", last_vcgl_cmd);
      printf("    Command was at byte offset %d (0x%X)\n",
              cmd_byte_offset, cmd_byte_offset);
      printf("    Currently at byte offset   %d (0x%X)\n",
              byte_offset, byte_offset);
      printf("    Returning a Dummy 0 word\n");
    }
    if (val_words == 0)
      i = 0;                    /* Spoon feed a dummy zero short back */
    else
      i = (long) get_short();
  }
  if (debug_level > 5)
    printf("get_long returning %ld\n", i);
  return (i);
}


/* ****************************************************************** */
/* Get Val function (Count of words in VCGL command)                  */
/* ****************************************************************** */

/* This function returns the count of words in the command            */

static long 
get_val()
{
  unsigned long   i;            /* unsigned to avoid sign extend */

  switch (vcgl_form) {
  case VERY_LONG_FORM:
    i = get_long();
    break;

  case LONG_FORM:
    i = ((unsigned long) get_short() & 0xffff);
    break;

  case SHORT_FORM:
    i = (get_byte(FALSE) & 0xff);
    break;
  }

  return (i);
}

/* ************************************************************************* */
/* Get Command function (Returns a VCGL command word                         */
/* ************************************************************************* */

/* This function returns a VCGL command from the input stream         */
static unsigned short 
get_cmd()
{
  int vms_rec = 0, vms_off = 0;

  /* Check to see that the last functions left the pointers in the  */
  /* Correct Place in the input data stream.                        */

  cmd_byte_offset = byte_offset;/* Save current byte offset   */
  stop_output_flag = 0;         /* Reset */
  if (odd_byte_flag) {
    if (err_count > MAX_ERRORS) {
      if ( print_sum ) summary();
      exit(1);
    }
    printf("\n***WARNING - Last Command ended in an odd byte\n");
    printf("                 odd_byte_flag = %X\n", odd_byte_flag);
    odd_byte_flag = 0;
    if (val_words >= 1)
      printf(" Discarded byte = %X\n", get_byte(TRUE));
    if (err_count++ > MAX_ERRORS) {
      if ( print_sum ) summary();
      exit(1);
    }
  }
  if (val_words != 0) {
    printf("\n\n***WARNING - Last Command did not leave word count at zero\n");
    printf("       Skipping the following %ld words of Data\n\n",
            val_words);
    dump_data();
    if (err_count++ > MAX_ERRORS) {
      if ( print_sum ) summary();
      exit(1);
    }
  }
  /* This is where we handle the short form, long form, and        */
  /* the very long form type of input command.  A flag word is     */
  /* set for the get_val routine so it knows how long of a data    */
  /* count to get.                                                 */

  switch (last_vcgl_cmd = (get_byte(FALSE) & 0xff)) {

    /* Very Long Form VCGL Command FEcc nnnn nnnn */
  case 0xfe:
    cmd_byte_offset = byte_offset - 1;
    vcgl_form = VERY_LONG_FORM;
    val_words = 2;              /* 2 words count */
    last_vcgl_cmd = get_byte(FALSE) & 0xff;
#ifdef vms
    addr(&vms_rec,&vms_off);
    sprintf(sbuf,"\nbyte 0x%X, record %d, offset 0x%X\n           VERY LONG ",
            cmd_byte_offset,vms_rec,vms_off);
#else
    sprintf(sbuf,"\nbyte 0x%X:\n           VERY LONG ",
            cmd_byte_offset);
#endif
    break;

    /* Long Form VCGL Command FFcc nnnn           */
  case 0xff:
    cmd_byte_offset = byte_offset - 1;
    vcgl_form = LONG_FORM;
    val_words = 1;              /* 1 word count */
    last_vcgl_cmd = get_byte(FALSE) & 0xff;
#ifdef vms
    addr(&vms_rec,&vms_off);
    sprintf(sbuf,"\nbyte 0x%X, record %d, offset 0x%X\n           LONG      ",
            cmd_byte_offset,vms_rec,vms_off);
#else
    sprintf(sbuf,"\nbyte 0x%X:\n           LONG      ",
            cmd_byte_offset);
#endif
    break;

    /* Possible Frame Sync                        */
  case 0x18:
    cmd_byte_offset = byte_offset - 1;
    vcgl_form = SHORT_FORM;
#ifdef vms
    addr(&vms_rec,&vms_off);
    sprintf(sbuf,"\nbyte 0x%X, record %d, offset 0x%X\n           ",
            cmd_byte_offset,vms_rec,vms_off);
#else
    sprintf(sbuf,"\nbyte 0x%X:\n           ",
            cmd_byte_offset);
#endif
    break;

    /* Soft Form VCGL Command ccnn                */
  default:
    cmd_byte_offset = byte_offset - 1;
    vcgl_form = SHORT_FORM;
#ifdef vms
    addr(&vms_rec,&vms_off);
    sprintf(sbuf,"\nbyte 0x%X, record %d, offset 0x%X\n           SHORT     ",
            cmd_byte_offset,vms_rec,vms_off);
#else
    sprintf(sbuf,"\nbyte 0x%X:\n           SHORT     ",
            cmd_byte_offset);
#endif
    break;
  }
  return (last_vcgl_cmd);
}

/****************************************************************************/
/* Get X Value function (Returns a VCGL X value into a long variable        */
/****************************************************************************/

static long 
get_x_value()
 /* type *//* Offset from base vcgl command      */
 /* I.E. 1st Polyline cmd = 0x60       */
 /* 16X,32Y,8Rel = cmd 0x62, type = 2  */
{
  long l;

  /* Observe that the least significant bit of type (2**0)          */
  /* Tells us the sizeof the X value (16 or 32).                    */
  if (type & 1)
    l = get_long();             /* X is a 32 bit value */
  else
    l = (long) get_short();     /* X is a 16 bit value */
  return (l);
}


/* ************************************************************************* */
/* Get Y Value function (Returns a VCGL Y value into a long variable         */
/* ************************************************************************* */
static long
get_y_value()
 /* type *//* Offset from base vcgl command      */
 /* I.E. 1st Polyline cmd = 0x60       */
 /* 16X,32Y,8Rel = cmd 0x62, type = 2  */
{
  long l;

  /* Observe that bit 2**1 of type tells us the size of the Y       */
  /* value:  16 bits or 32 bits.                                    */
  if (type & 2)
    l = get_long();             /* Y is a 32 bit value */
  else
    l = (long) get_short();     /* Y is a 16 bit value */
  return (l);
}


/***************************************************************************/
/* Get rel value function (Returns a VCGL rel value into a long var        */
/***************************************************************************/
static long 
get_rel_value()
 /* type *//* Offset from base vcgl command      */
 /* I.E. 1st Polyline cmd = 0x60       */
 /* 16X,32Y,8Rel = cmd 0x62, type = 2  */
{
  long l;

  /* Observe that bits 2**3 & 2**2 of type tells us the size        */
  /* of the deltas (or relative values).                            */
  /* Type =  00xx (0)     : 8 bit relative values                   */
  /* 01xx (4)     : 16 bit relative values                          */
  /* 10xx (8)     : 32 bit relative values                          */

  switch (type & 0xc) {
  case 0:
    l = (long) get_byte(TRUE);
    break;
  case 4:
    l = (long) get_short();
    break;
  case 8:
    l = get_long();
    break;
  default:
    printf("*** WARNING - Bad Type in get_dx_value **\n");
    err_count++;
    printf("    Last VCGL command = 0x%X\n",
            last_vcgl_cmd);
    printf("    Command was at byte offset %d (0x%X)\n",
            cmd_byte_offset, cmd_byte_offset);
    l = 0;
    break;
  }
  return (l);
}

static addr(vms_rec,vms_off)
int *vms_rec,*vms_off;
{
  static int current_rec = 0;
  static int begin_offset = 0, next_offset = 0, previous_offset = 0;

  if ( *vms_rec == 1 ) {
    previous_offset = begin_offset;
    begin_offset += next_offset;
    next_offset = buf_words;
    current_rec++;
  } else {
    if ( (cmd_byte_offset - begin_offset ) >= 0 ) {
      *vms_rec = current_rec;
      *vms_off = cmd_byte_offset - begin_offset;
    } else {
      *vms_rec = current_rec - 1;
      *vms_off = cmd_byte_offset - previous_offset;
    }    
  }
}
