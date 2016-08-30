
/*
 * module         vrf.c V1.2
 * last modified  15:20:17 8/11/89
 * current date   15:20:47 8/11/89
 *
 * 1.2  for QA round 4 of VAX/VGS
 *      no changes
 * 1.1  for QA round 3 of VAX/VGS
 *      base code
 */

#include "dump.h"

#define MSIZE 240

#define NUM_VRF_COMMANDS                 22
#define LOGICAL_BLOCK_HEADERS             0
#define LEVEL_1_COMMANDS                  1
#define BEGIN_PARTITION_COMMANDS          2
#define DRAW_COMMANDS                     3
#define MOVE_COMMANDS                     4
#define DEFINE_PEN_COMMANDS               5
#define SET_PEN_COMMANDS                  6
#define DEFINE_PATTERN_COMMANDS           7
#define DRAW_POLYGON_COMMANDS             8
#define TEXT_STRING_COMMANDS              9
#define SET_FONT_COMMANDS                10
#define SET_FONT_BASE_LINE_COMMANDS      11
#define SKIP_COMMANDS                    12
#define DRAW_CIRCLE_COMMANDS             13
#define END_VRF_COMMANDS                 14
#define A_SUMMARY_AFTER_EACH_COLOR_PASS  15

/*------------------------------------------------------------------*/
/*   Filename : Vrfdmpr.c                                           *
 *   AUTHOR   : WS HARPER  - Copyright WSH Resources                *
 *   PROGRAM  : VRFDMPR.C OR VRFDMPR.PAS                            *
 *   AUTHOR   : WS HARPER  - Copyright WSH Resources                *
 *   Owner    : WSH Resources, Newark CA  94560                     *
 *              Propritory code to WSH resources.  All re-ports     *
 *              must be apporoved by Wsh Resourses.                 *
 *   Language : Origionally written using Pascal (turbo)            *
 *            : This version is written in  C (portable)            *
 *   Purpose  : Vrfdmpr is a Systems Analyst written utility.       *
 *   THIS PROGRAM WAS WRITTEN TO PARSE THROUGH "VRF" PLOT DATA      *
 *   AND CHECK TO SEE IF IT IS CORRECT. (I.E. CHECKING ALL THE      *
 *   HEADERS AND BYTE COUNTS. IF THE PLOT FILE HAS A PROBLEM THE    *
 *   UTILITY WILL POST THE PROBLEM IN THE PRINTED DUMP.             *
 *                                                                  *
 *   NOTE: THIS PLOT DATA IS PHASE II GENERATED DATA FROM VERSAPLOT *
 *         PACKAGES. BUT IT MUST BE CLEAN DATA  i.e. no system      *
 *         dependent driver codes in the data stream.               *
 *                                                                  *
 ********************************************************************
 * <<<<<<<<<    Pascal coding History begins here    >>>>>>>>>>>>>> *
 ********************************************************************
 *   HISTORY  :                                                     *
 *   1/19/86 -> DATE     : 1/19/86                                  *
 *              VERSION  : 1.0                                      *
 *              REVISION : A                                        *
 *             - RUNS WITH PHASE II GENERATED VERSAPLOT COLOR RANDOM*
 *               1.0, 1.2 (B/W) AND 1.1 RAW PLOT DATA  AS AN "INPUT"*
 *               FILE TO THIS UTILITY.                              *
 *    2/3/86    REVISION : B                                        *
 *              -FIXED BUGS RELATED TO FLAGS SET IN NONPOSCOM       *
 *              -FIXED BUGS TO DO WITH END VRF LOGIC IN COLOR MODE  *
 *              -FIXED BUGS IN LOGIC IN POSCOM                      *
 *    2/7/86    REVISION : C                                        *
 *              -ADDED A CHOICE OF OUTPUT EITHER SCREEN OF PRINTER  *
 *              -CHANGED HEADING ON UTILITY                         *
 *    2/9/86 -> DATE     : 2/9/86                                   *
 *              VERSION  : 1.1                                      *
 *              REVISION : A                                        *
 *              -ADDED NEW MODULES SET_UP,RESETSUM, AND LOGIC TO    *
 *               GO ALONG WITH IT.                                  *
 *              -MADE INITVARS AN SEPERATE PROCEDURE INCLUDE FILE   *
 *              -ADDED A HALT AFTER THE SUMMARY PER FRAME           *
 *              -FIXED ERRORS IN VARABLES THAT WERE NOT BEING RESET *
 *              -ADDED FASTER I/O OF 2K BUFFERS FROM DISK           *
 *              -PADDED LAST BUFFER                                 *
 *     5/6/86   REVISION : B                                        *
 *              -ADDED COUNT DOWN SUMMARY TO RUN AS PROGRAM PARCES  *
 *              -ADDED COLOR TO SCREEN SUMMARY MODE                 *
 *              -ADDED TOTAL ELEMENT/BYTE COUNT PER COLOR           *
 *              -ADDED TOTAL ELEMENT/BYTE COUNT FOR WHOLE PLOT      *
 *              -FIXED PATTERN DEFITION IN FULL DUMP MODE TO        *
 *               OUTPUT A BINARY PATTERN OF THE PATTERN DEFITION    *
 *                                                                  *
 *   12/15/86 -> VERSION :  2.0                                     *
 *               REVISION : A                                       *
 *               -ADDED VRF 2.0 SUPPORT; COPY COMMAND AND           *
 *                CONTROL ARRAY COMMAND AND NEW ETX '1803'          *
 *                COMMAND. Mods to take care of vrf 2.0 bigger      *
 *                blocks sizes                                      *
 *               -FIXED GENERAL BUGS FOUND AT TIME OF MODIFICATION  *
 *               -ALSO SCREWED UP 1.1 DATA MODE                     *
 *            ---> 12-22-86                                         *
 *               -FIXED 1.1 MODE ... but screwed up buffering       *
 *                on small files...   oh well ... re-port soon      *
 *                                                                  *
 ********************************************************************
 * <<<<<<<<<  C    Coding  History   starts   Here   >>>>>>>>>>>>>> *
 ********************************************************************
 *                                                                  *
 *** 12/28/86 -> Started porting code to C for lattice compiler     *
 *   (2.0)       12-30-86 Up in a 1801 1802 Trap mode.....          *
 *               1-12-87 General Message updates, and have          *
 *               enough code compiled to get the whole thing        *
 *               up in a test mode.                                 *
 *    2/9/87 ->  Too many bugs found in lattice C compiler          *
 *               (old version probably) so started using            *
 *               MicroSoft C V4.00.  It  seems to be much           *
 *               faster (at least in screen I/O) and much friendler *
 *               to use.  Re-compiler with only 3 errors and 6      *
 *               warnings. Have gotten it down to only 2 warnings   *
 *               but this is it because my fuctions are too big     *
 *               to be postprocessed optimized (this is a warning). *
 *    2/17/87 -> It works !!! at least on simple files.  More       *
 *               testing needed but it looks good.  Lets hope       *
 *               the origional pascal logic is sound re-written     *
 *               in C.                                              *
 *    3/22/87 -> Well it looks as if I have finially got all the    *
 *               BUGS out of the C version.  Wow what a challenge   *
 *               for the first program I ever wrote in C. It runs   *
 *               on known good color 2.0 vrf files ...              *
 *    3/30/87 -> Found some more bugs and solved them.  Some may    *
 *               may be actually differences between escape codes   *
 *               on the yellow color pass given out by differnt s/w *
 **** 6/1/87  -> Bug Reported today by Don Roysdon and Andy Chung   *
 *     (2.01)    to do with the utility blowing up after black      *
 *               putting out a Can't find CAN SOH message and       *
 *               showing an CAN STX in the input buffers.           *
 *               In essance what was happining is the file had      *
 *               so much garbage in it being (C0nn) skipped over    *
 *               and then an CAN STX before the CAN ETX.            *
 *               To fix it I changed the control (done) on the inner*
 *               parser loop from the 8000 to the 1803, which means *
 *               the utility can now only support 2.0 and up files  *
 *               Snuffed out Random 1.1 support with this patch     *
 *    7/14/87 -> Start of patch work for design flaw left in the    *
 *    (2.02)     logic of the code from turbo pascal of handling 32 *
 *               bit VRF data.  This requset was also needed by     *
 *               Bob Dietz to help analyze a problem.               *
 **** 7/23/87 -> Think I got it up and running in 32 bit vrf mode   *
 *    (2.02)     at least on a 32 bit version of volts.  Also       *
 *               tested it on a 32 bit version of clracc. They      *
 *               both passed with flying colors.                    *
 *               BETA release to Bob Deitz to try on that 3.5 MB    *
 *               questionable file on Alert!!!                      *
 *    8/16/87 -> No updates, but the beta release did parse the     *
 *               3.5 MB file with no problems.                      *
 *    8/17/87 -> Added a question at the end of the program         *
 *               to  abort or continue and branched on users        *
 *               Also added VRF_HELP routine for online help        *
 *               request.  Implemented per request of Don Royston.  *
 *    8/22/87 -> Started special request by Don Royston to have     *
 *               summary mode do nothing but give all commands,     *
 *               and totals for each color pass                     *
 *    8/30/87 -> Compleation of the above task                      *
 *    9/13/87 -> Added or fixed the design flaw of the Utility      *
 *               to deal with more than one VRF frame or plot       *
 *               per file.  Fixed because Bob Dietz felt it         *
 *               very inportant that I offer that capability        *
 **** 9/15/87 -> Finished fixing the above design flaw and the      *
 *    (2.03)     bug that was fixed along the way.                  *
 *               Bug: if we had 8D 03 18 02 01 F8 the utility       *
 *               would get mixed up and use the same varable for    *
 *               the bytcnt as the 03 after 8D, (namely byt1,byt2)  *
 *               therefore the utility thought it had an            *
 *               83 F8 and called it an error, because it is.       *
 *               Actually byt2 was getting restuffed with F8        *
 *               while caculating the Byte count, and then          *
 *               droping back into nonposc() with a check           *
 *               to byt2 to make sure its a legal draw circle       *
 *               command, that is have either a 03 or 04 in         *
 *               the byt2 varable.   This bug was probable          *
 *               introduced when 32 bit precision was added         *
 *               but not identified at that time.                   *
 *   10/11/87 -> Bug found by Toomey Freeman while he was           *
 *               porting the code over to TOPPS.                    *
 *               The bug is in poscom (Positing command)            *
 *               routine, I wasn't masking off the move/            *
 *               draw bits.                                         *
 *   11-3-87 ->  Added the implementation of Toomeys speed up       *
 *               patches.  One in Readfil() basically to buffer     *
 *               the data into sectors on the PC for Speed.  This   *
 *               is something I also had in the works.  It speeds   *
 *               the Program up about 5 times I would say.  The     *
 *               other tweek was in the parser() and number().      *
 *               There the byte count caculation was switched from  *
 *               multiplys and divides to Shift left 8 bits then    *
 *               add on the second byte.   Thanks for the help !!   *
 ****11-7-87 ->  General program maintence.  Fixed naming convention*
 *   (2.04)      back to origional state.  Streamlined extra stuff  *
 *               out of code.  Patched in uniform error messages    *
 *               in level1.c routine to match those of parser(),    *
 *               nonposco(), and poscom().                          *
 *   1-10-88 ->  Bug found in NONPOSCO to do with VRF skip commands *
 *               Fixed it. See that routine for more details.       *
 *               Basically the logic for that comand was wrong.     *
 *               It took an IBM 2.1.1 10mb file to do it.           *
 *               Fixed spelling errors in parser() (cancil>>cancel) *
 *               Fixed spelling errors in readfil() (phisical>>     *
 *               physical.                                          *
 *   1-14-88 ->  Also fixed the byte count offset.  It was wrong.   *
 *   (2.05)      Added full comments to number() to explain what    *
 *               all the routines do.  Also tell which ones are     *
 *               system specific.                                   *
 *               Added a new feature when an EOF is reached, the    *
 *               filesize (in bytes actually counted) is written    *
 *               out with the physical eof message.                 *
 *               Changed ERROR abortion back to a choice to         *
 *               the user,  ABORT or CONTINUE.  This was the        *
 *               origional way it was written but New Purduct       *
 *               Support wanted a exit() call on ERROR.  NOW        *
 *               they have changed their mind, they want a choice   *
 *   2-8-88 ->   Fixed the pgm_equal routine to actually work.      *
 *   (2.06)      What it does is check to see if the program        *
 *               has had its name changed and if so it blows        *
 *               up with a FATAL ERROR.                             *
 *   2-14-88 ->  Finished 18_>01_>00 00 State Machine on parser     *
 *   (2.07)      so that print data could be ignored before a       *
 *               Vrf file.  This was done by popular request.       *
 *   5/6/88  ->  Print data routine changed to output the data      *
 *               it finds (hex and ascii) if in full dump or        *
 *               command is flaged.                                 *
 *   8/1/88  ->  Bug reported by Bob Dietz;  Plot summary for       *
 *   (2.08)      for entire plot has fields that overflow. This     *
 *               happens on the SLC plot from Xerox.  Need file     *
 *   12/18/88->  Tested for Bug with Dietz, noticed that the        *
 *               overflow only happened on the summary screen not   *
 *               on the summary screen for the individual color.    *
 *   1/13/89 ->  Try  to resolve problem: by changing all the       *
 *               counting varables to the same type (i.e. long).    *
 *               Currently mov_com (or color summary) are short.    *
 *               See nonposcom.c, poscom.c and parser.c for more    *
 *               details.                                           *
 *   1/19-89 ->  Above fix tested and it works.  Must be bug in     *
 *               MS-C 5.0 COMPILER???...                            *
 *                                                                  */
/*------------------------------------------------------------------*/
/*------------------------------*/
/*   Program GLOBOL Varables    */
/*------------------------------*/

/*---------------------*/
/*--< file varables >--*/
/*---------------------*/
static FILE *fd;

/*-------------------*/
/*--< Char's Used >--*/
/*-------------------*/
static char dmpmode, view_print;
static char sp[] = "        ";

/*---------------------*/
/*--< INTEGERS USED >--*/
/*---------------------*/
static int array_cnt, byt2, byt3, byt5, byt4, bytin, bytct1, bytct2,
bytcnt, can, color_pass, comand, etx, lvl1_lbytcnt, last_com, 
soh, stx, wrdcnt, vv31_bit_pos_wrdcnt, lbuf1, lbuf2, maxl, maxu, 
xtmph, ytmph, xtmpl, ytmpl, plot_frame, zeros1, zeros2,

/*-----------------------------*/
/*--< flags (BOOLEANs) USED >--*/
/*-----------------------------*/

a1_found, alldone, author_on, color_mode_on, command_started,
control_array_found, canetxok, dra_cir_on, dra_poly_on, done, end_vrf,
full_dump, got_y, v31_bit_prec, last_pass, level2_on, pc_found, 
set_fnt_bas_on, skpwrds_on, tone_def_on, skip_on, txt_str_on,
vrfplot_started, recv_prt;

/*---------------------*/
/*--< long integers >--*/
/*---------------------*/

static long l_xcord, l_ycord, off_set;
static long lnumber();

static int x_cur, y_cur, ton_seq[4], plot_print, count[NUM_VRF_COMMANDS];
static int start_output[NUM_VRF_COMMANDS],end_output[NUM_VRF_COMMANDS];
static int sum_cmd[NUM_VRF_COMMANDS][5],pass_print;
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_*/
/*                                                               */
/*                 MAIN PROGRAM ---> VRF DUMPER <---             */
/*                                   -----------                 */
/*-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/


vrf(ifile,mode,rc_file)
FILE *ifile;
char mode,*rc_file;
{
  initvars();  /* Initialize all Varables */
  fd = ifile;
  switch(mode) {
    case 'S':
      dmpmode = 'S';
      full_dump = 0;
    break;
    case 'P':
      dmpmode = 'C';
      setup(rc_file);
    break;
    case 'F':
      dmpmode = 'F';
      full_dump = 1;
    break;
  }
  parser();
}


/*---------------------------------------------------------------------*/
/* Program   : VRFDMPR.PAS or Vrfdmpr.c
   Module    : BITMAP.DPR  or Vrfdmpr.c
   Author    : W.S. Harper
   Calls     : Only standard C printf routine
   Called by : NONPOSCO.DPR or Nonposco.c
   History   : Origionally written in pascal but re-written
               in C for portability.
   Purpose   : CALLED BY NONPOSCO WHEN A TONE PATTERN IS DEFINED.
               THIS ROUTINE WILL WRITE OUT THE BINARY BIT PATTERNS
               NEEDED BY DEFINING A PATTERN.
                                                                       */
/*---------------------------------------------------------------------*/

static bitmap(half_byt5, half_byt6)
int half_byt5;
int half_byt6;
{

  static int tsthalf, i;

  /*-------------------------------*/
  /* Set up double for loop for a  */
  /* CASE against byte   instead   */
  /* of a word  to save on binary  */
  /* printf 1010101 patterns...    */
  /*-------------------------------*/

  for (i = 1; i <= 2; ++i) {
    switch (i) {
      /*---------------------------*/
      /* Get the correct nibble    */
      /* for the bitmap case       */
      /*---------------------------*/
      case 1:
        tsthalf = half_byt5;
      break;
      case 2:
        tsthalf = half_byt6;
      break;
    }
    switch (tsthalf) {
      /*----------------------------*/
      /* compair the nibble to get  */
      /* the correct binary pattern */
      /*----------------------------*/
      case 0x00:
        printf("0000");
      break;
      case 0x01:
        printf("0001");
      break;
      case 0x02:
        printf("0010");
      break;
      case 0x03:
        printf("0011");
      break;
      case 0x04:
        printf("0100");
      break;
      case 0x05:
        printf("0101");
        break;
      case 0x06:
        printf("0110");
      break;
      case 0x07:
        printf("0111");
      break;
      case 0x08:
        printf("1000");
      break;
      case 0x09:
        printf("1001");
      break;
      case 0x0A:
        printf("1010");
      break;
      case 0x0B:
        printf("1011");
      break;
      case 0x0C:
        printf("1100");
      break;
      case 0x0D:
        printf("1101");
      break;
      case 0x0E:
        printf("1110");
      break;
      case 0x0F:
        printf("1111");
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
/* Program : Vrfdmpr()
   Module  : finlsum.c
   Author  : W.S. Harper
   Purpose : This routine gives the finial summary (finlsum) of all the vrf
             commands that were found during the parse.
   History : 8-28-87 - Written to produce a summary mode to parse the whole
             file and spit out the statitics of the plot job at the end of
             the parse.  Requested idea from Don Roysdon, and I thought is
             was good request to expand the way Vrfdmpr() runs.  Implemented
             10-10-87 - Added Mdos timers for benchmarks.  This is not need for
             reports of this code unless one wants to call the SYSTEM Specific
             timer.  The lines of code will have additional comments around
             them to inform the code porter.
             1-9-88 - Fixed dos timer.  The origional version didn't work
             right.  This one does, It has been tested on both big and small
             PLOT files.
                                                                             */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

static plot_sum()
{
  int pass,cmd;

  plot_frame = plot_frame + 1;
  printf("\nSummary for Plot %u\n", plot_frame);

  for ( pass = 0 ; pass < 4 ; pass++)
    switch(ton_seq[pass]) {
      case 0:
        printf("     Black");
      break;
      case 1:
        printf("      Cyan");
      break;
      case 2:
        printf("   Magenta");
      break;
      case 3:
        printf("    Yellow");
      break;
    }

  printf("     Total  Level 2 VRF Commands\n");
  print_pass("DEFINE PATTERNS COMMANDS",DEFINE_PATTERN_COMMANDS);
  print_pass("DEFINE PEN COMMANDS",DEFINE_PEN_COMMANDS);
  print_pass("DRAW CIRCLES COMMANDS",DRAW_CIRCLE_COMMANDS);
  print_pass("DRAW COMMANDS",DRAW_COMMANDS);
  print_pass("DRAW POLYGONS COMMANDS",DRAW_POLYGON_COMMANDS);
  print_pass("LOGICAL VDS BLOCK COUNT",LOGICAL_BLOCK_HEADERS);
  print_pass("MOVE COMMANDS",MOVE_COMMANDS);
  print_pass("SET FONT BASELINE COMMANDS",SET_FONT_BASE_LINE_COMMANDS);
  print_pass("SET FONT COMMANDS",SET_FONT_COMMANDS);
  print_pass("SET PEN COMMANDS",SET_PEN_COMMANDS);
  print_pass("SKIP WORD COMMANDS",SKIP_COMMANDS);
  print_pass("TEXT STRING COMMANDS",TEXT_STRING_COMMANDS);

  plot_print = 0;
  for ( pass = 0 ; pass < 4 ; pass++ )
    for ( cmd = 0 ; cmd < NUM_VRF_COMMANDS ; cmd++ )
      sum_cmd[cmd][pass] = 0;

  ton_seq[0] = 0;               /* default single pass black */
  ton_seq[1] = -1;              /* no-op color pass */
  ton_seq[2] = -1;              /* no-op color pass */
  ton_seq[3] = -1;              /* no-op color pass */
}

/*=================================================================*/
/* PROGRAM: VRFDMPR.PAS or Vrfdmpr.c
 * MODULE : INITVARS.DPR of initvars.c
 * Author : W.S. Harper
 * Language: Origionally written in PASCAL but This version was
             re-writter in C and further maintence was done on the C
             version over the PASCAL.
 * PURPOSE: INITIALIZE GLOBOL VARIBLES FOR ALL MODULES USED IN THE
 *          PROGRAM
   Called by  : Parser() at the begining to initialize all varables
   Calls      : No one !
 ==================================================================*/


/*======================================================================*/

static initvars()
{
  int i,j;

/*-----------------------------------------*/
/* Flag varables for Command selected mode */
/*-----------------------------------------*/
  view_print = 'N';
  dmpmode = 'H';
/*------------------------*/
/* buffers used           */
/*------------------------*/
  bytin = 0;
/*------------------------*/
/* flag/booleans used     */
/*------------------------*/
  recv_prt = 0;
  vrfplot_started = 0;
  control_array_found = 0;
  color_mode_on = 0;
  command_started = 0;
  done = 0;
  full_dump = 0;
  level2_on = 0;
  dra_cir_on = 0;
  txt_str_on = 0;
  got_y = 0;
  dra_poly_on = 0;
  tone_def_on = 0;
  skpwrds_on = 0;
  end_vrf = 0;
  author_on = 0;
  skip_on = 0;                  /* level1 skip command */
  alldone = 0;
  last_pass = 0;
/*  cntsum_started =  0; */
/*  BEGN_PART_ON = 0;    */
/*  begn_com_on =   0;   */
/*-----------------*/
/* short counters  */
/*-----------------*/
  bytcnt = 0;
  plot_frame = 0;
/*---------------*/
/* long counters */
/*---------------*/
  off_set = 0;
  color_pass = 0;               /* black */
/*------------------*/
/* special counters */
/*------------------*/
  ton_seq[0] = 0;               /* default single pass black */
  ton_seq[1] = -1;              /* no-op color pass */
  ton_seq[2] = -1;              /* no-op color pass */
  ton_seq[3] = -1;              /* no-op color pass */
  x_cur = -1;                   /* undefined until VRF INIT  */
  y_cur = -1;                   /* undefined until VRF INIT  */
  for ( i = 0 ; i < NUM_VRF_COMMANDS ; i++ ) {
    start_output[i] = 0;
    end_output[i] = 0;
    count[i] = 0;
    for ( j = 0 ; j < 5 ; j++ ) {
      sum_cmd[i][j] = 0;
    }
  }
}

/*---------------------------------------------------------------------*/
/* Program: Vrfdmpr.pas or Vrfdmpr.c
   Module : Level1.dpr or level1.c
   Author : W.S. Harper
   Purpose: Level1() is called from the parser() to process level1
            VDS and plotter commands.  In other words at times level1()
            is a plotter emulator for level1 commands.
                                                                       */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/

static vds_copies(bytin)
int bytin;
{
  if (command_started == 0) {
    readfil(&lvl1_lbytcnt);
    if (lvl1_lbytcnt == 6) {
      bytcnt = bytcnt - 1;
      command_started = 1;
      count[LEVEL_1_COMMANDS]++;
      if (ck(LEVEL_1_COMMANDS) || full_dump) {
        addr(-2);
        printf("           %-9d ",count[LEVEL_1_COMMANDS]);
        printf("LEVEL 1 COMMAND,%s           ccnn = C506 (3)\n",sp);
        printf("           multiple copies command\n");
      }
    } else {
      printf("\nERROR: Unrecognized VDS Level1 Command\n");
      printf("in MULTIPLE COPIES command parse\n");
      printf("expecting C506 (hex), found %X (hex)\n", number(bytin,byt2));
      printf("offset of bytes into file is %X (hex)\n",off_set);
      exit(1);
    }
  }
  while ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
    switch (lvl1_lbytcnt) {
    case (6):{
        readfil(&byt3);
        readfil(&byt4);
        bytcnt = bytcnt - 2;
        if (ck(LEVEL_1_COMMANDS) || full_dump)
          printf("           knt = %d\n",number(byt3,byt4));
        lvl1_lbytcnt = lvl1_lbytcnt - 2;
        break;
      }
    case (4):{
        readfil(&byt3);
        readfil(&byt4);
        bytcnt = bytcnt - 2;
        if (ck(LEVEL_1_COMMANDS) || full_dump)
          printf("           maxu = %u\n",number(byt3,byt4));
        lvl1_lbytcnt = lvl1_lbytcnt - 2;
        command_started = 0;
        break;
      }
    case (2):{
        readfil(&byt3);
        readfil(&byt4);
        bytcnt = bytcnt - 2;
        if (ck(LEVEL_1_COMMANDS) || full_dump)
          printf("           maxl = %u\n",number(byt3,byt4));
        lvl1_lbytcnt = lvl1_lbytcnt - 2;
        command_started = 0;
        break;
      }
    default:{
        printf("\nERROR: unrecognized VDS level1 command\n");
        printf("in COPY COUNT command parse;\n");
        printf("The error was after the C506 (hex)\n");
        printf("offset of bytes into file is %X (hex)",off_set);
        exit(1);
      }
      break;
    }
  }                             /* end of while loop */
}




/*---------------------------------------------------------------------*/
/* Program: Vrfdmpr.pas or Vrfdmpr.c
   Module : Level1.dpr or level1.c
   Author : W.S. Harper                                                */

/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/

static rpm_control_array()
{
  static int tmp_local;

  if (command_started == 0) {
    readfil(&lvl1_lbytcnt);
    array_cnt = lvl1_lbytcnt;
    bytcnt = bytcnt - 1;
    command_started = 1;
  }
  while ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
    readfil(&byt3);
    readfil(&byt4);
    bytcnt = bytcnt - 2;
    lvl1_lbytcnt = lvl1_lbytcnt - 2;
    if (((byt3 == 0x50) && (byt4 == 0x43)) || (pc_found))
      if (pc_found == 0)
        pc_found = 1;
      else if ((byt3 == 0x41) && (byt4 == 0x31)) {
        a1_found = 1;
        count[LEVEL_1_COMMANDS]++;
        if (ck(LEVEL_1_COMMANDS) || full_dump) {
          addr(-6);
          printf("           %-9d ",count[LEVEL_1_COMMANDS]);
          printf("LEVEL 1 COMMAND,%s           ccnn = %X (%d)\n",sp,
            0xc600+array_cnt,array_cnt/2);
          printf("           control array command\n");
          printf("           control array flagword: PCA1\n");
        }
      } else if (ck(LEVEL_1_COMMANDS) || full_dump) 
        switch ((array_cnt - lvl1_lbytcnt)) {
          case 8:
            switch((tmp_local = lnumber(tmp_local,number(byt3,byt4)))) {
              case -1:
                printf("           non VRF Data\n");
              break;
              default:
                printf("           total elements bytes in vrf partition = ");
                printf("%u\n",tmp_local);
              break;
            }
          break;
          case 12:
            switch((tmp_local = lnumber(tmp_local,number(byt3,byt4)))) {
              case -1:
                printf("           non VRF Data\n");
              break;
              default:
                printf("           maximum range of any partition = ");
                printf("%u\n",tmp_local);
              break;
            }
          break;
          case 16:
            switch((tmp_local = lnumber(tmp_local,number(byt3,byt4)))) {
              case -1:
                printf("           non VRF Data\n");
              break;
              default:
                printf("           maximum x = ");
                printf("%u\n",tmp_local);
              break;
            }
          break;
          case 18:
            switch(number(byt3,byt4)) {
              case 0:
                printf("           non-VRF data\n");
              break;
              case -1:
              /* case 0xffff: */
                printf("           vrf data is not partitioned by host\n");
              break;
              default:
                printf("           vrf data is partitioned by host\n");
              break;
            }
          break;
          case 20:
            printf("           (1) reserved word 10 = %u\n",number(byt3,byt4));
          break;
          case 22:
            printf("           (2) reserved word 11 = %u\n",number(byt3,byt4));
          break;
          case 24:
            printf("           (3) plotter error action is ");
            switch (number(byt3, byt4)) {
              case 1:
                printf("terminate\n");
              break;
              case 2:
                printf("continue\n");
              break;
              case -1:
              /* case 0xffff: */
                printf("use default\n");
              break;
              default:
                printf("non-VRF data\n");
              break;
            }
          break;
          case 26:
            printf("           (4) vrf data error action is ");
            switch (number(byt3, byt4)) {
              case 1:
                printf("ignore\n");
              break;
              case 2:
                printf("plot upt to error\n");
              break;
              case 3:
                printf("discard plot\n");
              break;
              case -1:
              /* case 0xffff: */
                printf("use default\n");
              break;
            }
          break;
          case 28:
            printf("           (5) reserved word 14 = %u\n",number(byt3,byt4));
          break;
          case 30:
            printf("           (6) plot author flagword is ");
            switch (number(byt3, byt4)) {
              case 0:
                printf("disable\n");
              break;
              case -1:
              /* case 0xffff: */
                printf("use default\n");
              break;
              default:
                printf("enable\n");
              break;
            }
          break;
          case 32:
            printf("           (7) statistics is plot statistics ");
            switch (number(byt3, byt4)) {
              case 0:
                printf("disable\n");
              break;
              case -1:
              /* case 0xffff: */
                printf("use default\n");
              break;
              default:
                printf("enable\n");
              break;
            }
          break;
          case 34:
            printf("           (8) reserved word 17 = ");
            printf("%u\n",number(byt3,byt4));
          break;
          case 36:
            printf("           (9) disk control is ");
            switch (number(byt3,byt4)) {
              case 1:
                printf("do not use disk\n");
              break;
              case 2:
                printf("always spool raster if memory exceeded\n");
              break;
              case 3:
                printf("always spool vrf elements\n");
              break;
              case 4:
                printf("overflow vrf elements if memory exceeded\n");
              break;
              case -1:
              /* case 0xffff: */
                printf("use installation default\n");
              break;
            }
          break;
          case 38:
            printf("           (10) reserved word 19 = ");
            printf("%u\n",number(byt3,byt4));
          break;
          case 40:
            printf("           (11) reserved word 20 = ");
            printf("%u\n",number(byt3,byt4));
          break;
          default:
            tmp_local = number(byt3,byt4);
          break;
        }                       /* OF switch */
  }
  if (lvl1_lbytcnt == 0) {
    command_started = 0;
    pc_found = 0;
    a1_found = 0;
  }
}


/*---------------------------------------------------------------------*/
/* Program: Vrfdmpr.pas or Vrfdmpr.c
   Module : Level1.dpr or level1.c
   Author : W.S. Harper                                                */

/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/


static passall_printmode(bytin)
int bytin;
{
  static int tmp,tmp2;
  /* note added during PC port */
  /* had to make these varables unsigned */
  static unsigned tmp_local, rmtfct = 0;

  if (command_started == 0) {
    readfil(&lvl1_lbytcnt);
    bytcnt = bytcnt - 1;
    command_started = 1;
  }
  while ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
    if ( rmtfct == 0 ) {
      readfil(&byt3);
      readfil(&byt4);
      bytcnt = bytcnt - 2;
      lvl1_lbytcnt = lvl1_lbytcnt - 2;
      rmtfct = (byt3 << 8) + byt4;
      tmp_local = 1;
    } 
    if (byt3 == 0x1B) {
      count[LEVEL_1_COMMANDS]++;
      if (ck(LEVEL_1_COMMANDS) || full_dump) {
        addr(-2);
        printf("           %-9d ",count[LEVEL_1_COMMANDS]);
        printf("LEVEL 1 COMMAND,%s           ccnn = 1B%2X (0)\n",sp,byt4);
        printf("           print pass all\n");
        printf("           multiplexer control\n");
      }
      rmtfct = 0x1B00;
    }
    switch (rmtfct) {
      case 0x1B00: /* MUX */
        rmtfct = 0;
      break;
      case 0x9B3C: /* ESC < */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B3C (1)\n",sp);
              printf("           print pass all\n");
              printf("           remote enable ff and eot\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      case 0x9B3E: /* ESC > */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B3E (1)\n",sp);
              printf("           print pass all\n");
              printf("           remote disable ff and eot\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      case 0x9B42: /* ESC B */
        if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
          switch(tmp_local) {
            case 1:
              readfil(&byt3);
              readfil(&byt4);
              tmp = (byt3 << 8) + byt4;
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
              count[LEVEL_1_COMMANDS]++;
              if (ck(LEVEL_1_COMMANDS) || full_dump) {
                addr(-2);
                printf("           %-9d ",count[LEVEL_1_COMMANDS]);
                printf("LEVEL 1 COMMAND,%s           ",sp);
                printf("ccnn = 9B42 (%d)\n",tmp+1);
                printf("           print pass all\n");
                printf("           remote plot label\n");
                printf("           ");
              }
            break;
            default:
              readfil(&byt3);
              tmp--;
              bytcnt--;
              lvl1_lbytcnt--;
              if (ck(LEVEL_1_COMMANDS) || full_dump) printf("0x%X ",byt3&0xff);
              if (tmp == 0) rmtfct = 0;
            break;
          }
          tmp_local++;
        }
      break;
      case 0x9B43: /* ESC C */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B43 (1)\n",sp);
              printf("           print pass all\n");
              printf("           remote clear buffer command\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      case 0x9B45: /* ESC E */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B45 (1)\n",sp);
              printf("           print pass all\n");
              printf("           remote line enhance command\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      case 0x9B46: /* ESC F */
        if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
          switch(tmp_local) {
            case 1:
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
              tmp = (byt3 << 8) + byt4;
              tmp_local++;
            break; 
            case 2:
              count[LEVEL_1_COMMANDS]++;
              if (ck(LEVEL_1_COMMANDS) || full_dump) {
                addr(-2);
                printf("           %-9d ",count[LEVEL_1_COMMANDS]);
                printf("LEVEL 1 COMMAND,%s           ccnn = 9B46",sp);
                printf("(%d)\n",tmp/2+1);
                printf("           print pass all\n");
                printf("           remote font load\n");
                printf("           byte count = %d\n",tmp);
              }
              tmp_local++;
            default:
              readfil(&byt3);
              bytcnt--;
              lvl1_lbytcnt--;
              tmp--;
              if ( tmp == 0 ) rmtfct = 0;
            break;
          }
        }
      break; 
      case 0x9B47: /* ESC G */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B47 (1)\n",sp);
              printf("           print pass all\n");
              printf("           remote select page mode\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      case 0x9B48: /* ESC H */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B48 (2)\n",sp);
              printf("           print pass all\n");
              printf("           remote set character height\n");
            }
            tmp_local++;
          break;
          case 2:
          case 3:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
              tmp_local++;
            }
          break;
          case 4:
            rmtfct = 0;
          break;
        }
      break;
      case 0x9B49: /* ESC I */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B49 (1)\n",sp);
              printf("           print pass all\n");
              printf("           remote inverse image command\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      case 0x9B4B: /* ESC K */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B4B (1)\n",sp);
              printf("           print pass all\n");
              printf("           remote cut immediate\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      case 0x9B4D: /* ESC M */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B4D (1)\n",sp);
              printf("           print pass all\n");
              printf("           remote mirror image command\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      case 0x9B50: /* ESC P */
        if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
          switch (tmp_local) {
            case 1:
            case 2:
              tmp_local++;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            break;
            default:
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
              count[LEVEL_1_COMMANDS]++;
              if (ck(LEVEL_1_COMMANDS) || full_dump) {
                addr(-2);
                printf("           %-9d ",count[LEVEL_1_COMMANDS]);
                printf("LEVEL 1 COMMAND,%s           ccnn = 9B50 (3)\n",sp);
                printf("           print pass all\n");
                printf("           color plotter preamble\n");
                printf("           ");
              }
              if ( byt3 == 0x1b ) {
                if (ck(LEVEL_1_COMMANDS) || full_dump) {
                  printf("intermediate pass for multi-pass color plot\n");
                  printf("           yellow pass\n");
                  printf("           color plotter control byte = 0x1b");
                }
                color_pass = 3;
                color_mode_on = 1;
                addtoning(3);
              } else {
                color_pass = byt3&0x03;
                switch (byt3&0x0c) {
                  case 0x00:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("single pass plot\n");
                    ton_seq[0] = color_pass;
                    color_mode_on = 0;
                  break;
                  case 0x04:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("first pass of multi-pass color plot\n");
                    ton_seq[0] = color_pass;
                    color_mode_on = 1;
                  break;
                  case 0x08:
                    if (ck(LEVEL_1_COMMANDS) || full_dump) {
                      printf("intermediate or last pass for multi-pass ");
                      printf("color plot\n");
                    }
                    color_mode_on = 1;
                    addtoning(color_pass);
                  break;
                  case 0x0c:
                    printf("illegal - bits (2-3) set\n");
                  break;
                }
                switch(byt3&0x30) {
                  case 0x00:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           not last pass\n");
                  break;
                  default:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           last pass\n");
                    color_mode_on = 0;
                  break; 
                }
                switch(byt3&0x40) {
                  case 0x00:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           use color plotting width\n");
                  break;
                  case 0x40:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           use full plotting width\n");
                  break;
                }
                switch(byt3&0x80) {
                  case 0x00:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           do not merge tick marks\n");
                  break;
                  case 0x80:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           merge tick marks\n");
                  break;
                }
                switch(color_pass) {
                  case 0x00:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           black pass\n");
                  break;
                  case 0x01:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           cyan pass\n");
                  break;
                  case 0x02:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           magenta pass\n");
                  break;
                  case 0x03:
                    if (ck(LEVEL_1_COMMANDS) || full_dump)
                      printf("           yellow pass\n");
                  break;
                }
              }
              rmtfct = 0;
            break;
          }
        }
      break;
      case 0x9B52: /* ESC R */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B52 (2)\n",sp);
              printf("           print pass all\n");
              printf("           remote rdt command\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      case 0x9B53: /* ESC S */
        if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
          switch(tmp_local) {
            case 1:
              count[LEVEL_1_COMMANDS]++;
              if (ck(LEVEL_1_COMMANDS) || full_dump) {
                addr(-2);
                printf("           %-9d ",count[LEVEL_1_COMMANDS]);
                printf("LEVEL 1 COMMAND,%s           ccnn = 9B53 (2)\n",sp);
                printf("           print pass all\n");
                printf("           remote speed control of ");
              }
            break;
            case 2:
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            break;
            case 3:
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
              rmtfct = 0;
              if (ck(LEVEL_1_COMMANDS) || full_dump) {
                switch ((tmp2 = number(byt3,byt4))) {
                  case 0x0:
                    printf("0.125 ips\n");
                  break;
                  case 0x1:
                    printf("0.25 ips\n");
                  break;
                  case 0x2:
                    printf("0.50 ips\n");
                  break;
                  case 0x3:
                    printf("0.75 ips\n");
                  break;
                  case 0x4:
                    printf("1.00 ips\n");
                  break;
                  case 0x5:
                    printf("1.25 ips\n");
                  break;
                  case 0x6:
                    printf("1.50 ips\n");
                  break;
                  case 0x7:
                    printf("1.75 ips\n");
                  break;
                  case 0x8:
                    printf("2.00 ips\n");
                  break;
                  case 0xff:
                    printf("Full Speed\n");
                  break;
                  default:
                    printf("%d\n",tmp2);
                    printf("           invalid speed\n");
                  break;
                }
              }
            break;
          }
          tmp_local++;
        }
      break;
      case 0x9B54: /* ESC T */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B54 (2)\n",sp);
              printf("           print pass all\n");
              printf("           remote select plot mode command\n");
            }
            tmp_local++;
          break;
          case 2:
          case 3:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
              tmp_local++;
            }
          break;
          case 4:
            rmtfct = 0;
          break;
        }
      break;
      case 0x9B57: /* ESC W */
        switch(tmp_local) {
          case 1:
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 9B57 (1)\n",sp);
              printf("           print pass all\n");
              printf("           remote color plotter rewind command\n");
            }
            tmp_local++;
          break;
          case 2:
            if ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
              rmtfct = 0;
              readfil(&byt3);
              readfil(&byt4);
              bytcnt = bytcnt - 2;
              lvl1_lbytcnt = lvl1_lbytcnt - 2;
            }
          break;
        }
      break;
      default:
        count[LEVEL_1_COMMANDS]++;
        if (ck(LEVEL_1_COMMANDS) || full_dump) {
          addr(-2);
          printf("           %-9d ",count[LEVEL_1_COMMANDS]);
          printf("LEVEL 1 COMMAND,%s           ccnn = %X (0)\n",sp,rmtfct);
          printf("           print pass all\n");
          printf("           data = %X\n",rmtfct);
        }
        rmtfct = 0;
      break;
    }
  }
  if (lvl1_lbytcnt == 0) {
    command_started = 0;
  }
}



/*---------------------------------------------------------------------*/
/* Program: Vrfdmpr.pas or Vrfdmpr.c
   Module : Level1.dpr or level1.c
   Author : W.S. Harper
   Purpose: This routine is called from the parser routine and it's
            purpose is to parse LEVEL I VDS/(VRF) commands.
   Calls  : Readfil() - grabs a pointer to a byte of data
            number() - gives 16 bit number
            lnumber() - gives 32 bit number
   Called
       by : Parser() - main vds data parser

   History: Origionally written (Turbo) Pascal, but this version was
            re-written in (Lattice) C. Then recompiled using ms C.
            7-14-87 - Added the hooks to allow 32 bit precision.
            10-27-87 - Found bug in copy count inplementation while
            working with Mr. Dietz.  It looks like a real live failure
            of good data.  Wopps I left out some common code.  Sorry !!
            Anyways it has been resolved with the of other common code
            to this routine.

   Notes:   This routine probably has the most options and combinations
            of vds level1 commands.  Thus it has the most code and could
            contain the most bugs.  Since all the different combinations
            of level 1 commands in supported in this routine has never
            been fully tested.  All the standard ways and combinations have
            but the odd ones have not, although the locic says they should.

            Please contact the developer if a bug is found.
                                                                       */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/

static level1(bytin)
int bytin;
{
  if (bytcnt > 0) {
    switch (bytin) {
    case (0xC0):{
        if (command_started == 0) {
          readfil(&lvl1_lbytcnt);
          bytcnt = bytcnt - 1;
          command_started = 1;
        }
        if (lvl1_lbytcnt == 0) {
          count[LEVEL_1_COMMANDS]++;
          if (ck(LEVEL_1_COMMANDS) || full_dump) {
            addr(-2);
            printf("           %-9d ",count[LEVEL_1_COMMANDS]);
            printf("LEVEL 1 COMMAND,%s           ccnn = C000 (0)\n",sp);
            printf("           skip command 0 bytes skipped");
            printf(" (no-op)");
          }
        }
        while ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
          readfil(&byt3);
          readfil(&byt4);
          bytcnt = bytcnt - 2;
          lvl1_lbytcnt = lvl1_lbytcnt - 2;
          if ((((byt3 == 0x49) && (byt4 == 0x44)) && (author_on == 0)) ||
              ((author_on == 1) && (skip_on == 0))) {
            if (author_on == 0) {
              count[LEVEL_1_COMMANDS]++;
              if (ck(LEVEL_1_COMMANDS) || full_dump) {
                addr(-4);
                printf("           %-9d ",count[LEVEL_1_COMMANDS]);
                printf("LEVEL 1 COMMAND,%s           ccnn = %X (%d)\n",sp,
                  0xc000+lvl1_lbytcnt+2,lvl1_lbytcnt/2+1);
                printf("           author command\n");
                printf("           ");
              }
              author_on = 1;
            } else if (ck(LEVEL_1_COMMANDS) || full_dump)
              printf("%c%c", byt3, byt4);       /* CHR(byt3),CHR(byt4)); */
          } else if (skip_on == 0) {
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-4);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = %X (%d)\n",sp,
                0xc000+lvl1_lbytcnt+2,lvl1_lbytcnt/2+1);
              printf("           skip command\n");
              printf("           bytes skipped = %d", lvl1_lbytcnt + 2);
            }
            skip_on = 1;
          }
        }                       /* end of while loop */
        if (lvl1_lbytcnt == 0) {
          command_started = 0;
          author_on = 0;
          skip_on = 0;
          bytin = 0;
          if (ck(LEVEL_1_COMMANDS) || full_dump)
            printf("\n");
        }
        break;
      }                         /* OF 0xC0 : */
    case (0xC2):{
        /*-------------------------------*/
        /* Well this is where the  plot  */
        /* finishes . . . we then signal */
        /* back to the parser that its   */
        /* our last pass, and the file   */
        /* has just been processed.      */
        /*-------------------------------*/
        readfil(&byt2);
        bytcnt = bytcnt - 1;
        if (byt2 == 0x00) {
          last_pass = 1;
          vrfplot_started = 0;
          count[LEVEL_1_COMMANDS]++;
          if (ck(LEVEL_1_COMMANDS) || full_dump) {
            addr(-2);
            printf("           %-9d ",count[LEVEL_1_COMMANDS]);
            printf("LEVEL 1 COMMAND,%s           ccnn = C200 (0)\n",sp);
            printf("           form feed command\n");
          }
        } else {
          printf("\nERROR: Unrecognized VDS Level1 Command\n");
          printf("in FORM FEED COMMAND parse\n");
          printf("expecting C200 (hex), found %X (hex)\n",number(bytin,byt2));
          printf("offset of bytes into file is %X (hex)\n",off_set);
          exit(1);
        }
        break;
      }
    case 0xC3:{
        /*-------------------------------*/
        /* Well this is where the  plot  */
        /* finishes . . . we then signal */
        /* back to the parser that its   */
        /* our last pass, and the file   */
        /* has just been processed.      */
        /*-------------------------------*/
        readfil(&byt2);
        bytcnt = bytcnt - 1;
        if (byt2 == 0x00) {
          last_pass = 1;
          vrfplot_started = 0;
          count[LEVEL_1_COMMANDS]++;
          if (ck(LEVEL_1_COMMANDS) || full_dump) {
            addr(-2);
            printf("           %-9d ",count[LEVEL_1_COMMANDS]);
            printf("LEVEL 1 COMMAND,%s           ccnn = C300 (0)\n",sp);
            printf("           end of transmission command\n");
          }
        } else {
          printf("\nERROR: Unrecognized VDS Level1 Command\n");
          printf("in END OF TRANSMISSION command parse\n");
          printf("expecting C300 (hex), found %X (hex)\n",number(bytin,byt2));
          printf("offset of bytes into file is %X (hex)\n",off_set);
          exit(1);
        }
        break;
      }
    case 0xC4:{
        readfil(&byt2);
        bytcnt = bytcnt - 1;
        count[LEVEL_1_COMMANDS]++;
        if (ck(LEVEL_1_COMMANDS) || full_dump)
          if (byt2 == 0) {
            addr(-2);
            printf("           %-9d ",count[LEVEL_1_COMMANDS]);
            printf("LEVEL 1 COMMAND,%s           ccnn = C400 (0)\n",sp);
            printf("           remote line terminate command\n");
          } else {
            printf("\nERROR: Unrecognized VDS Level1 Command\n");
            printf("in REMOTE LINE TERMINATE command parse\n");
            printf("expecting C400 (hex), ");
            printf("found %X (hex)\n",number(bytin,byt2));
            printf("offset of bytes into file is %X (hex)\n",off_set);
            exit(1);
          }
        break;
      }
    case 0xC5:{
        vds_copies(bytin);
        break;
      }
    case 0xC6:{
        rpm_control_array();
        break;
      }
    case 0xCC:{
        passall_printmode(bytin);
        break;
      }
    case 0x83:{
        if (command_started == 0) {
          readfil(&byt2);
          lvl1_lbytcnt = 4;
          bytcnt = bytcnt - 1;
          if (byt2 == 0x00) {
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 8300 (2)\n",sp);
              printf("           initialization to 16 bit vrf");
              printf(" level 2 commands\n");
            }
            x_cur = 0;
            y_cur = 0;
            level2_on = 1;
            lvl1_lbytcnt = 4;
            command_started = 1;
            vrfplot_started = 1;
          } else {
            printf("\nERROR: Unrecognized VDS Level1 Command\n");
            printf("in VRF INITIALIZATION TO 16 BIT LEVEL 2 command parse\n");
            printf("expecting 8300 (hex), found ");
            printf("%X (hex)\n",number(bytin,byt2));
            printf("offset of bytes into file is %X (hex)\n",off_set);
            exit(1);
          }
        }
        while ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
          readfil(&byt3);
          readfil(&byt4);
          bytcnt = bytcnt - 2;
          lvl1_lbytcnt = lvl1_lbytcnt - 2;
          if (ck(LEVEL_1_COMMANDS) || full_dump)
            switch (lvl1_lbytcnt) {
            case 0x02:{
                printf("           idens = %u\n", number(byt3, byt4));
                break;
              }
            case 0x00:{
                printf("           iscan = %u\n", number(byt3, byt4));
                break;
              }
            default:{
                printf("\nERROR: Unrecognized VDS Level1 Command\n");
                printf("in INITIALIZATION VRF parse in IDENS ");
                printf("and IBYTES values\n");
                printf("offset of bytes into file is %X (hex)\n",off_set);
                exit(1);
              }
            }
        }
        if (lvl1_lbytcnt == 0x00)
          command_started = 0;
        break;
      }
    case 0x84:{
        if (command_started == 0) {
          readfil(&byt2);
          lvl1_lbytcnt = 8;
          v31_bit_prec = 1;
          bytcnt = bytcnt - 1;
          if (byt2 == 0x00) {
            count[LEVEL_1_COMMANDS]++;
            if (ck(LEVEL_1_COMMANDS) || full_dump) {
              addr(-2);
              printf("           %-9d ",count[LEVEL_1_COMMANDS]);
              printf("LEVEL 1 COMMAND,%s           ccnn = 8400 (4)\n",sp);
              printf("           initialization to 32 bit vrf ");
              printf("level 2 commands\n");
            }
            x_cur = 0;
            y_cur = 0;
            command_started = 1;
            level2_on = 1;
            vrfplot_started = 1;
          } else {
            printf("\nERROR: Unrecognized VDS Level1 Command\n");
            printf("in VRF INITIALIZATION TO 32 BIT LEVEL 2 command parse\n");
            printf("expecting 8400 (hex), found ");
            printf("%X (hex)\n",number(bytin,byt2));
            printf("offset of bytes into file is %X (hex)\n",off_set);
            exit(1);
          }
        }
        while ((bytcnt > 0) && (lvl1_lbytcnt > 0)) {
          readfil(&byt3);
          readfil(&byt4);
          bytcnt = bytcnt - 2;
          lvl1_lbytcnt = lvl1_lbytcnt - 2;
          if (ck(LEVEL_1_COMMANDS) || full_dump)
            switch (lvl1_lbytcnt) {
            case 0x06:{
                lbuf1 = number(byt3, byt4);
                break;
              }
            case 0x04:{
                lbuf2 = number(byt3, byt4);
                l_xcord = lnumber(lbuf1, lbuf2);
                printf("           idens = %u\n", l_xcord);
                break;
              }
            case 0x02:{
                lbuf1 = number(byt3, byt4);
                break;
              }
            case 0x00:{
                lbuf2 = number(byt3, byt4);
                l_ycord = lnumber(lbuf1, lbuf2);
                printf("           iscan = %u\n", l_ycord);
                break;
              }
            default:{
                printf("\nERROR: Unrecognized VDS Level1 Command\n");
                printf("in INITIALIZATION VRF parse in IDENS ");
                printf("and IBYTES values\n");
                printf("offset of bytes into file is %X (hex)\n",off_set);
                exit(1);
              }
            }                   /* end of switch lvl1_bytcnt of */
        }                       /* end of while */
        if (lvl1_lbytcnt == 0x00)
          command_started = 0;
        break;
      }
    }                           /* of switch bytin of */
  }
}

/*---------------------------------------------------------------------*/
/*
  Program :Vrfdmpr
  Modules :Drapolygon, Setfntbaseline, Endvrf, Nonposco
           Nonposcom is the main module which calls:
               \_______________________ Drawpolygon
                \______________________ Setfntbaseline
                 \_____________________ Endvrf
           Which are are all entry points in this routine.
  Author  :W.S Harper
  History :Origionally written in (Turbo) Pascal but re-written
           in (Lattice) C.
           Re-Ported code to MicroSoft C for Analyst PC project.
  Purpose :This routine is called from the Parser routine.  It is called
           to process VRF non-positioning commands.
  History :Added 31 bit precision per request of Bob Deitz.
  7/14/87  Had to change commands Begin Partitioning, Defpen,
           Set font baseline, Draw Circle.
           BUGS: were found after the modifications: in: begpart, and
           resolved almost immeditely.
  7/15/87  Added "uniform" error messages and fixed all commands to termite
           upon any error in the VFR data stream at all. Also chaned
           huge code in case statement in nonposcom.c to call routines
           so compiler could optimize the code more.
  1/6/88   Bug identified with large (10 MB) IC plot file from Scott
           Paskett.  The problem was the file Scott had containded
           VRF skip word(s) command.  I guess this portion of the code
           was never tested before.  Maybe it's because most of the
           larger files that I've parsed have been VAX/Sun files.
           Scott's was an IBM (VM/CMS) Random 2.1 file with about
           Ten Megebytes of data.  I have parsed IBM 2.1 files before
           but never this large.   Keep those big files a comming so
           we can rattle all the BUGS out of the Utility.
   1-13-89 Changed all varables to one common extern area (finially!)
           changed all single pass color counting varables:
             [mov_com,   draw_com,  set_pens,   def_pens, def_pats, dra_cirs,]
             [dra_polys, set_fonts, set_fbases, skp_wrds, tex_strs, lgcl_blks,]
           to long from int.  In effort to cure problem of counter over-run.
           While doing the changes lots of other small problem were resolved,
           mostly code clean-up.
                                                                       */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/

static drawpolygon(bytin)
int bytin;
{
  static int tmp_local;

  if (dra_poly_on == 0) {
    count[DRAW_POLYGON_COMMANDS]++;
    if (ck(DRAW_POLYGON_COMMANDS) || full_dump) {
      addr(-4);
      printf("           %-9d ",count[DRAW_POLYGON_COMMANDS]);
      printf("DRAW POLYGON COMMAND,%s      ccnn = %X (%d)\n",sp,
        0x8800+byt2,byt2);
      if ( x_cur != -1 )
      printf("           current position x = %u, y = %u\n",x_cur,y_cur);
      printf("           fill pattern = %u\n", lbuf2);
    }
    dra_poly_on = 1;
  } else {
    if (v31_bit_prec) {
      switch(got_y) {
        case 0:
        case 2:
          tmp_local = number(byt3,byt4) << 16;
          got_y++;
        break;
        case 1:
          tmp_local += number(byt3,byt4);
          if (ck(DRAW_POLYGON_COMMANDS) || full_dump)
            printf("           vertex at x = %u,",tmp_local&0x7fffffff);
          got_y++;
        break;
        case 3:
          tmp_local += number(byt3,byt4);
          if (ck(DRAW_POLYGON_COMMANDS) || full_dump) {
            printf("           vertex at x = %u,",tmp_local&0x7fffffff);
            if ( tmp_local > 0x7fffffff )
              printf("(outlining)\n");
            else
              printf("(no outlining)\n");
          }
          got_y = 0;
        break;
      }
    } else {
      tmp_local = number(byt3,byt4);
      if (got_y == 0) {
        if (ck(DRAW_POLYGON_COMMANDS) || full_dump)
          printf("           vertex at x = %u,",tmp_local&0x7fff);
        got_y = 1;
      } else {
        if (ck(DRAW_POLYGON_COMMANDS) || full_dump) {
          printf(" y = %u ",tmp_local&0x7fff);
          if ( tmp_local > 0x7fff )
            printf("(outlining)\n");
          else
            printf("(no outlining)\n");
        }
        got_y = 0;
      }
    }
  }
}


/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/

static setfntbaseline(bytin)
int bytin;
{
  if ((byt2 == 0x04) && (v31_bit_prec)) {
    switch (wrdcnt) {
    case (03):{
        lbuf1 = number(byt3, byt4);
        break;
      }
    case (02):{
        lbuf2 = number(byt3, byt4);
        l_xcord = lnumber(lbuf1, lbuf2);
        break;
      }
    case (01):{
        lbuf1 = number(byt3, byt4);
        break;
      }
    case (00):{
        lbuf2 = number(byt3, byt4);
        l_ycord = lnumber(lbuf1, lbuf2);
        count[SET_FONT_BASE_LINE_COMMANDS]++;
        if (ck(SET_FONT_BASE_LINE_COMMANDS) || full_dump) {
          addr(-10);
          printf("           %-9d ",count[SET_FONT_BASE_LINE_COMMANDS]);
          printf("SET FONT BASELINE COMMAND,%s ccnn = 8B04 (4)\n",sp);
          printf("           31 bit precision\n");
          printf("           x component = %u\n", l_xcord);
          printf("           y component = %u\n", l_ycord);
        }
        break;
      }
    }
    /* break; */
  } else if (byt2 == 0x02) {    /* 15 bit precision */
    if (set_fnt_bas_on == 0) {
      count[SET_FONT_BASE_LINE_COMMANDS]++;
      if (ck(SET_FONT_BASE_LINE_COMMANDS) || full_dump) {
        addr(-4);
        printf("           %-9d ",count[SET_FONT_BASE_LINE_COMMANDS]);
        printf("SET FONT BASELINE COMMAND,%s ccnn = 8B02 (2)\n",sp);
        printf("           15 bit precision\n");
        printf("           x component = %u\n", number(byt3, byt4));
        set_fnt_bas_on = 1;
      }
    } else 
      if (ck(SET_FONT_BASE_LINE_COMMANDS) || full_dump) 
        printf("           y component = %u\n", number(byt3, byt4));
    /* break; */
  } else {
    printf("\nERROR: Unrecognized VRF Nonpositioning Command\n");
    printf("in SET FONT BASELINE COMMAND parse.  ");
    printf("Unable to Parse Second Byte\n");
    printf("expecting 87 nn {nn=02,nn=04} (hex), found ");
    printf("%X (hex)\n",number(bytin, byt2));
    printf("offset of bytes into file is %X (hex)\n",off_set);
    exit(1);
  }
  /* break;  */
}

/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/

static endofvrf(bytin)
int bytin;
{
  if (wrdcnt == 0x00) {
    level2_on = 0;
    end_vrf = 1;
    count[END_VRF_COMMANDS]++;
    if (ck(END_VRF_COMMANDS) || full_dump) {
      addr(-2);
      printf("           %-9d ",count[END_VRF_COMMANDS]);
      printf("END VRF COMMAND,%s           ccnn = 8000 (0)\n",sp);
    }
    if (color_mode_on == 0) {
      last_pass = 1;
    }
  } else {
    printf("\nERROR: Unrecognized VRF Nonpositioning Command\n");
    printf("in END VRF COMMAND parse. ");
    printf("Unable to Parse Second Byte of Command\n");
    printf("expecting 8000 (hex), found %X (hex)\n",number(bytin,byt2));
    printf("offset of bytes into file is %X (hex)\n",off_set);
    exit(1);
  }
}


/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/

static drawcircle(bytin)
int bytin;
{
  if ((byt2 == 0x04) && (v31_bit_prec)) {       /* 31 bit precision */
    if (dra_cir_on == 0) {
      count[DRAW_CIRCLE_COMMANDS]++;
      if (ck(DRAW_CIRCLE_COMMANDS) || full_dump) {
        addr(-4);
        printf("           %-9d ",count[DRAW_CIRCLE_COMMANDS]);
        printf("DRAW CIRCLE COMMAND,%s       ccnn = 8D04 (4)\n",sp);
        printf("           31 bit precision\n");
        if ( x_cur != -1 )
        printf("           current position x = %u, y = %u\n",x_cur,y_cur);
        printf("           fill pattern = %u\n", number(byt3, byt4));
      }
      dra_cir_on = 1;
    } else if (ck(DRAW_CIRCLE_COMMANDS) || full_dump)
      switch (wrdcnt) {
      case 0x02:{
          printf("           width = %u\n", number(byt3, byt4));
          break;
        }
      case 0x01:{
          lbuf1 = number(byt3, byt4);
          break;
        }
      case 0x00:{
          lbuf2 = number(byt3, byt4);
          printf("           radius = %u\n", lnumber(lbuf1, lbuf2));
          break;
        }
      }
    /* break; */
  } else if (byt2 == 0x03) {    /* 15 bit precision */
    if (dra_cir_on == 0) {
      count[DRAW_CIRCLE_COMMANDS]++;
      if (ck(DRAW_CIRCLE_COMMANDS) || full_dump) {
        addr(-4);
        printf("           %-9d ",count[DRAW_CIRCLE_COMMANDS]);
        printf("DRAW CIRCLE COMMAND,%s       ccnn = 8D03 (3)\n",sp);
        printf("           15 bit precision\n");
        if ( x_cur != -1 )
          printf("           current position x = %u, y = %u\n",x_cur,y_cur);
        printf("           fill pattern = %u\n", number(byt3, byt4));
      }
      dra_cir_on = 1;
    } else if (ck(DRAW_CIRCLE_COMMANDS) || full_dump) 
      switch (wrdcnt) {
      case 0x01:
        printf("           width = %u\n", number(byt3, byt4));
        break;
      case 0x00:
        printf("           radius = %u\n", number(byt3, byt4));
        break;
      }
    /* break; */
  } else {
    printf("\nERROR: Unrecognized VRF Nonpositioning  Command\n");
    printf("in DRAW CIRCLE COMMAND parse. ");
    printf("Unable to Parse Second Byte of Command\n");
    printf("expecting 8Dnn {nn=3,nn=4} (hex), found ");
    printf("%X (hex)\n",number(bytin,byt2));
    printf("offset of bytes into file is %X(hex)\n",off_set);
    exit(1);
  }
  /* break; */
}




/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/

static defpen(bytin)
int bytin;
{
  if (byt2 = 0x06)
    /* 15 bit Precision */
  {
    if (wrdcnt == 0x05) {
      count[DEFINE_PEN_COMMANDS]++;
      if (ck(DEFINE_PEN_COMMANDS) || full_dump) {
        addr(-4);
        printf("           %-9d ",count[DEFINE_PEN_COMMANDS]);
        printf("DEFINE PEN COMMAND,%s        ccnn = 8206 (6)\n",sp);
        printf("           15 bit precision\n");
        printf("           pen index = %u\n", number(byt3, byt4));
      }
    }
    if (ck(DEFINE_PEN_COMMANDS) || full_dump)
      switch (wrdcnt) {
      case 0x02:
      case 0x04:
          printf("           nibs on = %u\n", number(byt3, byt4));
          break;
      case 0x01:
      case 0x03:
          printf("           nibs off = %u\n", number(byt3, byt4));
          break;
      case 0x00:
          printf("           line width = %d\n", number(byt3, byt4));
          break;
      }
    /* break; */
  } else
    /*************************************/
    /* 32 Bit precision has been invoked */
    /*************************************/
  if ((byt2 = 0x0A) && (v31_bit_prec)) {        /* 31 bit Precision */
    if (wrdcnt == 0x09) {
      count[DEFINE_PEN_COMMANDS]++;
      if (ck(DEFINE_PEN_COMMANDS) || full_dump) {
        addr(-4);
        printf("           %-9d ",count[DEFINE_PEN_COMMANDS]);
        printf("DEFINE PEN COMMAND,%s        ccnn = 820A (10)\n",sp);
        printf("           31 bit precision\n");
        printf("           pen index = %u\n", number(byt3, byt4));
      }
    }
    if (ck(DEFINE_PEN_COMMANDS) || full_dump)
      switch (wrdcnt) {
      case 0x08:
      case 0x06:
      case 0x04:
      case 0x02:
          lbuf1 = number(byt3, byt4);
          break;
      case 0x07:
      case 0x03:
          lbuf2 = number(byt3, byt4);
          printf("           nibs on = %u\n", lnumber(lbuf1, lbuf2));
          break;
      case 0x05:
      case 0x01:
          lbuf2 = number(byt3, byt4);
          printf("           nibs off = %u\n", lnumber(lbuf1, lbuf2));
          break;
      case 0x00:{
          printf("           line width = %d\n", number(byt2, byt3));
          break;
        }
      }
    /* break; */
  } else {
    printf("\nERROR: Unrecognized VRF Nonpositioning Command\n");
    printf("in DEFINE PEN COMMAND Parse. ");
    printf("Unable to Parse Second Byte of Command\n");
    printf("expecting 8206 or 0A (hex), found %X (hex)\n",number(bytin,byt2));
    printf("offset of bytes into file is %X (hex)\n",off_set);
    exit(1);
  }
}


/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/

static nonposco(bytin)
int bytin;
{
/*************************/
/* static local varables */
/*************************/
  static int
   halfbyt1, halfbyt2, halfbyt3, halfbyt4;
  static int tmp_local,tmp_cr;
  /**************************/
  /* start of code lnonposc */
  /**************************/
  if (bytcnt > 0) {
    if (command_started == 0)
      /******************************************************/
      /* If its the first time through  for  this  command  */
      /* Then read the second byte, and set command started */
      /* to a true state because we just started a command. */
      /* Otherwise bypass this code block.                  */
      /******************************************************/
    {
      readfil(&byt2);
      wrdcnt = byt2;
      bytcnt = bytcnt - 1;
      command_started = 1;
    }
    /******************************************************/
    /* Special code for endvrf command and a level2 vrf  */
    /* skip command if 0 words...  patched in to fix     */
    /* BUG in this routine dealing with skip commands    */
    /* */
    /******************************************************/

    if (byt2 == 0x00)
      switch (bytin) {
      case 0x80:{
          endofvrf(bytin);
          break;
        }
      case 0x8C:{
          count[SKIP_COMMANDS]++;
          if (ck(SKIP_COMMANDS) || full_dump) {
            addr(-2);
            printf("           %-9d ",count[SKIP_COMMANDS]);
            printf("SKIP COMMAND,%s              ccnn = %X (%d)\n",sp,
              0x8c00+wrdcnt+1,wrdcnt+1);
            printf("           words to skip = %d\n", (wrdcnt + 1));
          }
          break;
        }
      }                         /* end of switch statement */


    /* while (((bytcnt > 0) && (wrdcnt > 0)) || (((wrdcnt == 0) && (bytcnt >
     * 0) && (end_vrf == 0)) && ((bytin == 0x80) || (bytin == 0x8C)))) */

    while ((bytcnt > 0) && (wrdcnt > 0))
      /*********************************************************/
      /* While the 1802 bytcnt > 0, Nonposcom wrdcnt > 0, or   */
      /* wrdcnt = 0 and bytcnt > 0 and end vrf not set and     */
      /* bytin is either hex(80) or hex(8c)                    */
      /*********************************************************/
    {
      if (wrdcnt > 0) {
        readfil(&byt3);
        readfil(&byt4);
        bytcnt = bytcnt - 2;
        wrdcnt = wrdcnt - 1;
      }
      switch (bytin) {
        /* case 0x80:{ Checked in the special case above endofvrf(bytin);
         * break; } */
      case 0x81:{
          if (byt2 == 0x01)
            /********************/
            /* 15 bit Precision */
            /********************/
          {
            count[BEGIN_PARTITION_COMMANDS]++;
            if (ck(BEGIN_PARTITION_COMMANDS) || full_dump) {
              addr(-4);
              printf("           %-9d ",count[BEGIN_PARTITION_COMMANDS]);
              printf("BEGIN PARTITION COMMAND,   %sccnn = 8101 (1)\n",sp);
              printf("           15 bit precision\n");
              printf("           width = %u scan lines\n", number(byt3, byt4));
            }
            break;
          } else
            /********************/
            /* 31 bit precision */
            /********************/
          if (byt2 == 0x02) {
            switch (wrdcnt) {
            case 01:{
                lbuf1 = number(byt3, byt4);
                break;
              }
            case 00:{
                lbuf2 = number(byt3, byt4);
                count[BEGIN_PARTITION_COMMANDS]++;
                if (ck(BEGIN_PARTITION_COMMANDS) || full_dump) {
                  addr(-6);
                  printf("           %-9d ",count[BEGIN_PARTITION_COMMANDS]);
                  printf("BEGIN PARTITION COMMAND,%sccnn = 8102 (2)\n"
                    ,sp);
                  printf("           31 bit precision\n");
                  printf("           width = ");
                  printf("%u scan lines\n", lnumber(lbuf1, lbuf2));
                }
                break;
              }
            }
          } else {
            printf("\nERROR: Unrecognized VRF Nonpositioning Command in\n");
            printf("in BEGIN PARTITION COMMAND parse. ");
            printf("Unable to Parse Second Byte\n");
            printf("expecting 8101 or 02 (hex), found ");
            printf("%X (hex)\n",number(bytin, byt2));
            printf("offset of bytes into file is %X (hex)\n",off_set);
            exit(1);
          }
          break;
        }
      case 0x82:{
          defpen(bytin);
          break;
        }
      case 0x83:{
          if (byt2 == 0x01) {
            count[SET_PEN_COMMANDS]++;
            if (ck(SET_PEN_COMMANDS) || full_dump) {
              addr(-4);
              printf("           %-9d ",count[SET_PEN_COMMANDS]);
              printf("SET PEN COMMAND,%s           ccnn = 8301 (1)\n",sp);
              printf("           index = %u\n", number(byt3, byt4));
              command_started = 0;
            }
            break;
          } else {
            printf("\nERROR: Unrecognized VRF Nonpositioning Command\n");
            printf("in SET PEN COMMAND parse. ");
            printf("Unable to Parse Second Byte of Command\n");
            printf("expecting 8301 (hex), found ");
            printf("%X (hex)\n",number(bytin,byt2));
            printf("offset of bytes into file is %X (hex)\n",off_set);
            exit(1);
          }
          break;
        }
      case 0x87:{
          if ((byt2 >= 5) && (byt2 < 0xFF)) {
            if (tone_def_on == 0) {
              count[DEFINE_PATTERN_COMMANDS]++;
              if (ck(DEFINE_PATTERN_COMMANDS) || full_dump) {
                addr(-4);
                printf("           %-9d ",count[DEFINE_PATTERN_COMMANDS]);
                printf("DEFINE TONE PATTERN,%s       ccnn = %X (%d)\n",
                  sp,0x8700+byt2,byt2);
                printf("           fill pattern index = ");
                printf("%u\n", number(byt3, byt4));
              }
              tone_def_on = 1;
            } else {
              if (ck(DEFINE_PATTERN_COMMANDS) || full_dump) {
                if (wrdcnt >= (byt2 - 4)) {
                  switch (byt2 - wrdcnt) {
                  case 0x02:
                    printf("           reserved word\n");
                    break;
                  case 0x03:{
                      printf("           width in nibs y axis = ");
                      printf("%d\n", number(byt3, byt4));
                      break;
                    }
                  case 0x04:
                    printf("           height in nibs x axis = ");
                    printf("%d\n", number(byt3, byt4));
                    break;
                  }
                } else {
                  /****************************************/
                  /* Do the approate ANDs and ORs and     */
                  /* LOGICAL SHIFTS to get the right data */
                  /****************************************/
                  halfbyt1 = (byt3 & 0x0F);
                  halfbyt2 = (byt3 >> 4);
                  halfbyt3 = (byt4 & 0x0F);
                  halfbyt4 = (byt4 >> 4);
                  if (ck(DEFINE_PATTERN_COMMANDS) || full_dump) {
                    printf("           ");
                  /***************************************/
                  /* Dump bitmap in binary to the screen */
                  /***************************************/
                    bitmap(halfbyt1, halfbyt2);
                    bitmap(halfbyt3, halfbyt4);
                    printf("\n");
                  }
                }
              }                 /* IF DEF_PATS OF FULL DUMP */
            }
            break;
          } else {
            printf("\nERROR: Unrecognized VRF Nonpositioning Command in\n");
            printf("in DEFINE TONE PATTERN COMMAND parse. ");
            printf("Unable to Parse Second Byte\n");
            printf("expecting 87nn {0<nn<FF} (hex), found ");
            printf("%X (hex)\n",number(bytin, byt2));
            printf("offset of bytes into file is %X (hex)\n",off_set);
            exit(1);
          }
          break;
        }
      case 0x88:{
          lbuf2 = number(byt3, byt4);
          drawpolygon(bytin);
          break;
        }
      case 0x89:
        if (txt_str_on == 0) {
          txt_str_on++;
          tmp_local = number(byt3,byt4);
          tmp_cr = 0;
          count[TEXT_STRING_COMMANDS]++;
          if (ck(TEXT_STRING_COMMANDS) || full_dump) {
            addr(-4);
            printf("           %-9d ",count[TEXT_STRING_COMMANDS]);
            printf("TEXT STRING COMMAND,%s       ccnn = %X (%d)\n",
              sp,0x8900+byt2,byt2);
            if ( x_cur != -1 )
            printf("           current position x = %u, y = %u\n",x_cur,y_cur);
            printf("           number of characters = %d\n",tmp_local);
            printf("           ");
          }
          x_cur = -1;
        } else {
          if (ck(TEXT_STRING_COMMANDS) || full_dump) {
            tmp_cr++;
            charhex(byt3);
            if ( tmp_local != 1 ) charhex(byt4);
            if (tmp_cr%5 == 0) printf("\n           ");
            if ((tmp_cr%5 != 0)&&(wrdcnt == 0)) printf("\n");
          }
          tmp_local -= 2;
        }
      break;
      case 0x8A:
        count[SET_FONT_COMMANDS]++;
        if (ck(SET_FONT_COMMANDS) || full_dump) {
          addr(-4);
          printf("           %-9d ",count[SET_FONT_COMMANDS]);
          printf("SET FONT COMMAND,%s          ccnn = 8A01 (1)\n",sp);
          printf("           font index = %u\n", number(byt3, byt4));
        }
      break;
      case 0x8B:{
          setfntbaseline(bytin);
          break;
        }

      case 0x8C:{
          if ((wrdcnt >= 0) && (wrdcnt <= 256)) {
            if (skpwrds_on == 0) {
              count[SKIP_COMMANDS]++;
              if (ck(SKIP_COMMANDS) || full_dump) {
                addr(-4);
                printf("           %-9d ",count[SKIP_COMMANDS]);
                printf("SKIP WORD COMMAND,%s         ccnn = %X (%d)\n",
                  sp,0x8c00+wrdcnt+1,wrdcnt+1);
                printf("           words to skip = %d\n", (wrdcnt + 1));
              }
              skpwrds_on = 1;
            }
            break;
          } else {
            printf("\nERROR: Unrecognized VRF Nonpositioning Command\n");
            printf("in SKIP WORD(S) COMMAND parse. Unable to Parse ");
            printf("Second Byte of Command\n");
            printf("expecting 8Cnn (hex), found ");
            printf("%X (hex)\n",number(bytin, byt2));
            printf("offset of bytes into file is %X (hex)\n",off_set);
            exit(1);
          }
          break;
        }
      case 0x8D:{
          drawcircle(bytin);
          break;
        }

      default:
        {
          printf("\nERROR: Unrecognized VRF Nonpositioning Command\n");
          printf("Unable to Parse first byte of command\n");
          printf("found %X (hex)\n",number(bytin, byt2));
          printf("offset of bytes into file is %X (hex)\n",off_set);
          exit(1);
        }
      }                         /****  switch  bytin  ****/
    }                           /****  end of while loop Test !!! ****/
    if (wrdcnt == 0)
      /********************************************/
      /* If were done with the command then reset */
      /* all the flag type varables.              */
      /********************************************/
    {
      command_started = 0;
      dra_cir_on = 0;
      skpwrds_on = 0;
      set_fnt_bas_on = 0;
      txt_str_on = 0;
      got_y = 0;
      tone_def_on = 0;
      dra_poly_on = 0;
    }
    /*          }          *//****  end of while loop ****/
  }                             /****  end of if bytcnt > 0 ***/
}
/*************************************************************************/
/*  This file contains several small functions.
    They are named as follows:
-----------------------------------------------------------------------
 Routine        | System Specific |        Purpose
-----------------------------------------------------------------------
 number.c       |        NO       | Caculate 16 bit number from two bytes
 lnumber.c      |        NO       | Caculate 32 bit number from four bytes
----------------------------------------------------------------------
                                                                          */
/*===========================================================================*/
/* PROGRAM : VRFDMPR.PAS or vrfdmpr.c
   MODULE  : NUMBER.PAS or number.c
   Author  : W.S. Harper
   PURPOSE : A function which TAKES TWO BYTE NUMBER AND CHANGES
             THEM INTO ONE NUMBER, AN INTEGER.  IT THEN RETURNS
             THE VALUE TO THE CALLING PROGRAM. This is accomplished by
             executing a bit wise AND to mask against a hex 0F and a hex
             F0 to get the most significant digits caculated correct.
   HISTORY : 04-05-86 -> ORIGIONALLY WRITTEN (TURBO) PASCAL, BUT THIS
             VERSION WAS RE-WRITTEN IN (LATTICE) C.
             1-3-87 -> In C version had to add a varable (RESULT) to return the
             function value with.  In pascal I did not have to have this
             because pascal returns the value to the fuction name.           */
/*===========================================================================*/


/*===========================================================================*/

static int number(num1, num2)
int num1, num2;
{
  return ((num1 << 8) + num2);
}


/*===========================================================================*/
/* PROGRAM : VRFDMPR.PAS or vrfdmpr.c
   MODULE  : lnumber.c
   Author  : W.S. Harper
   PURPOSE : A function which TAKES TWO words AND CHANGES
             THEM INTO ONE NUMBER, AN INTEGER.  IT THEN RETURNS
             THE VALUE TO THE CALLING PROGRAM. This is accomplished by
             executing a bit wise AND to mask against a hex 0F and a hex
             F0 to get the most significant digits caculated correct.
   HISTORY : 04-05-86 -> ORIGIONALLY WRITTEN (TURBO) PASCAL, BUT THIS
             VERSION WAS RE-WRITTEN IN (LATTICE) C then again is MS C.
             1-3-87 -> In C version had to add a varable (RESULT) to return the
             function value with.  In pascal I did not have to have this
             because pascal returns the value to the fuction name.           */
/*===========================================================================*/
/*   Purpose: takes two 16 bit precision numbers and makes them into one
            32 bit number.
*/
/*===========================================================================*/


/*===========================================================================*/

static long lnumber(lnum1, lnum2)
int lnum1, lnum2;
{
  return ((lnum1 << 16) + lnum2);
}

/*------------------------------------------------------------------------*/
/* PROGRAM: VRFDMPR.PAS or vrfdmpr.c
   MODULE : PARCER.DPR or parser.c
   Author : W.S. Harper
   History: 02-05-86 -> Origional release written in Turbo Pascal.
            12-26-86 -> Rewritten in C using pascal source code.
            7-14-87  -> Modifed to accept 32 bit precision
            8-28-87  -> Added comments to assist in program reading.
            9-13-87  -> Added or fixed the design flaw to only deal
                        with one plot or frame per phisical file.
            9-15-87  -> Fixed bug related to 8d 03 18 02 01 f8 in
                        the vrf data stream.  To fix it I changed
                        the varables caculating the byte count from
                        byt1 and byt2 to bytct1 and bytct2 respectively.
                        This fixed the bug, and gives me just that much
                        higher level of confidence in the utility.
            1--31-87 -> Changed the way the bytcnt varable was caculated.
                        I use to do ands, multiplys, and divides.
                        Thanks to a suggestion by Toomey Freeman and Pete
                        Holztman we now shift left 8 bits then add on the
                        other byte (i.e. bytct2).  Thanks for the the help.
            2/7/88   -> Added Separate 1801 conditional for print data 1801
                        Scan through.  This was added so if PRINT data was
                        captured prior to an 1801 then it could be ignored.
                        It is setup when togled in SETUP.C.
            2/14/88  -> Finished adding 18 01 00 00 state machine to parser
                        so that print data could be bypassed before a Vrf
                        plot file.  Also restructured the parser for easy
                        reading.
        ********************************************************************
        *              Parser() Vds Header State Machine                   *
        ********************************************************************
        *                                                                  *
        *    stream in      ----   y   ----   y    -------   can soh 00    *
        *      ----------->| 18 |---->| 01 |----->| 00 00 |------>>>       *
        *           |       ----       ----        -------                 *
        *           ^        | n        | n           | n                  *
        *           |        |          |             |                    *
        *            ---<--------<------------<--------                    *
        *                print data ...                                    *
        ********************************************************************
            5/6/88  ->  Changed the way Printdta() is called;  Added the
                        actual data the parser found as print as a parameter
                        passed; Printdta(varable);
                    ->  Also added a custom PC screen as finlsum.pc; also
                        kept the UVS one as finlsum.uvs.  Will use BAT files
                        to rename module for compiles or manual renames.
            1-13-89 ->  Changed lgcl_blks from int to long.  Effort to cure but
                        that causes the summary varables to overflow ...

   Purpose: This routine is the "MAIN PARSER" for Vrfdmpr.  It is the
            guy who checks for 1801 0000 1802 0000 1802 bytcnt
            and also makes sure that the byte cuouts are correct.
            He  checks for either LEVEL1, NONPOSITING or
            POSITING Commands and calls the approate routine.

   Calls:   Readfil() - for data input from file
            Level1()  - to parse a Vds level 1 command
            Poscom()  - to parse a Vrf positioning Command
                        (i.e. a Move or Draw command)
            Nonposc() - to parse a Vrf non-positioning command
            Finlsum() - gives the finial summary of the plot

Called by:  Vrfdmpr()  - the main() for program Vrfdmpr().
                                                                             */
/*---------------------------------------------------------------------------*/



static parser()
{

/*--------------------------------------*/
/* Zero out parser header sync varables */
/*--------------------------------------*/
  can = 0;
  soh = 0;
  stx = 0;
  etx = 0;
  zeros1 = 0;
  zeros2 = 0;
  bytct1 = 0;
  bytct2 = 0;

  do {
    done = 0;
    end_vrf = 0;
    canetxok = 0;
    /*---------------------------*/
    /* The Main 1801 Sync loop   */
    /* Until we are alldone=1    */
    /*---------------------------*/
    readfil(&can);
    if (can == 0x18) {
      readfil(&soh);
      if (soh == 0x01) {
        readfil(&zeros1);
        readfil(&zeros2);
        if ((zeros1 == 0x00) && (zeros2 == 0x00)) {
          readfil(&can);
          readfil(&stx);
          if ( (can == 0xff) && (stx == 0xff) ) { /* 0xffff for Sun */
            count[LOGICAL_BLOCK_HEADERS]++;
            if (ck(LOGICAL_BLOCK_HEADERS) || full_dump) {
              addr(-4);
              printf("           %-9d ",count[LOGICAL_BLOCK_HEADERS]);
              printf("LOGICAL BLOCK HEADER,%s      ccnn = ffff (0)\n",sp);
              printf("           ignoring 0xffff between plots\n");
            }
            readfil(&can);
            readfil(&stx);
          }
          readfil(&zeros1);
          readfil(&zeros2);
          if (((can == 0x18) && (stx == 0x02)) && ((zeros1 == 0x00)
            && (zeros2 == 0x00))) {
            count[LOGICAL_BLOCK_HEADERS]++;
            if (ck(LOGICAL_BLOCK_HEADERS) || full_dump) {
              addr(-8);
              printf("           %-9d ",count[LOGICAL_BLOCK_HEADERS]);
              printf("LOGICAL BLOCK HEADER,%s      ccnn = 1801 (3)\n",sp);
              printf("           frame sync header\n");
            }
            plot_print = 1;
            pass_print = 1;
            /****************************************/
            /* Main loop for byte count bytes ...   */
            /* Do while not done, not end vrf and   */
            /* canetxok true ...                    */
            /****************************************/
            do {
              bytcnt = 0;
              readfil(&can);
              readfil(&stx);
              if ( (can == 0xff) && (stx == 0xff) ) { /* 0xffff for Sun */
                count[LOGICAL_BLOCK_HEADERS]++;
                if (ck(LOGICAL_BLOCK_HEADERS) || full_dump) {
                  addr(-4);
                  printf("           %-9d ",count[LOGICAL_BLOCK_HEADERS]);
                  printf("LOGICAL BLOCK HEADER,%s      ccnn = ffff (0)\n",sp);
                  printf("           ignoring 0xffff between plots\n");
                }
                readfil(&can);
                readfil(&stx);
              }
              /***********************************/
              /* read byte count for 1802 bytcnt */
              /***********************************/
              readfil(&bytct1);
              readfil(&bytct2);
              /****************************************/
              /* Implemented faster bytcnt caculation */
              /* supplied by Toomey Freeman and Mr.   */
              /* Holtzman. Uses more logical  math !! */
              /* Lots faster ....                     */
              /****************************************/
              bytcnt = (bytcnt = ((bytct1 << 8) + bytct2));
              if ((can == 0x18) && (stx == 0x02))
                /***********************************************/
                /* Now checking to see if we got an 1802 (hex) */
                /***********************************************/
              {
                recv_prt = 0;
                /* vrfplot_started = 1; */
                count[LOGICAL_BLOCK_HEADERS]++;
                if (ck(LOGICAL_BLOCK_HEADERS) || full_dump) {
                  addr(-4);
                  printf("           %-9d ",count[LOGICAL_BLOCK_HEADERS]);
                  printf("LOGICAL BLOCK HEADER,%s      ccnn = 1802 (1)\n",sp);
                  printf("           continuation header\n");
                  printf("           bytcnt = %d\n",bytcnt);
                }
                /**********************************/
                /* See if bytcnt is a valid value */
                /**********************************/
                if ((bytcnt <= 32767) && (bytcnt > 0))
                  while (bytcnt > 0)
                    /*******************************/
                    /* if so, start the loop . . . */
                    /* click down bytcnt bytes in  */
                    /* this loop . . .             */
                    /*******************************/
                  {
                    if (command_started == 1)
                      comand = last_com;
                    else {
                      readfil(&comand);
                      bytcnt = bytcnt - 1;
                    }
                    if ((((comand >= 0xC0) && (comand <= 0xCC)) ||
                       (((comand == 0x83) || (comand == 0x84))) &&
                       (level2_on == 0)))
                      /*****************************************************/
                      /* if command is level1 or command is initialization to
                       * level 2 */
                      /* and level2 has not been flaged to go ahead enter code
                       * block  */
                      /*****************************************************/
                    {
                      last_com = comand;
                      level1(comand);
                    } else if ((comand > 0x7F) && (level2_on))
                      /*****************************************************/
                      /* check to see if its a vrf nonpositioning command  */
                      /*****************************************************/
                    {
                      last_com = comand;
                      nonposco(comand);
                    } else
                      /**************************************************/
                      /* check to see if its a vrf positioning command  */
                      /* In other words either a MOVE or a DRAW command */
                      /**************************************************/
                    if ((comand < 0x80) && (level2_on)) {
                      last_com = comand;
                      poscom(comand);
                    } else {
                      printf("\nERROR: Unrecognized VRF Command\n");
                      printf("found %X\n", comand);
                      printf("offset of bytes into file is ");
                      printf("%X (hex)\n",off_set);
                      exit(1);
                    }
                  }
                /***********************************************************/
                /* end of while (bytcnt > 0) loop ...  must pop out of     */
                /* loop to sync up again i.e. 1802 bytcnt on logical block */
                /***********************************************************/
                else
                  /*******************************************************/
                  /* If bytcnt not valid then blow up with error message */
                  /*******************************************************/
                  /* if (bytcnt > 32767) *//* note: taking this line out */
                if (bytcnt > 32767)     /* note: taking this line out */
                  /*  fixes problem with error message */
                {
                  printf("\nERROR: byte count specified too large\n");
                  printf(" Offset of bytes into file is %X (hex)\n",off_set);
                  exit(1);
                }
              }
              /**************************/
              /* OF IF can stx  BYTECNT */
              /**************************/
              else
                /******************************/
                /* its not a CAN STX bytcnt   */
                /* what is it ?               */
                /******************************/
              {
                /****************************/
                /* is it a CAN ETX sequence */
                /****************************/
                if ((can == 0x18) && (stx == 0x03) && (zeros1 == 0x00)
                 && (zeros2 == 0x00))
                  /****************************/
                  /* If so then go for it !!! */
                  /****************************/
                {
                  done = 1;
                  canetxok = 1;
                  count[LOGICAL_BLOCK_HEADERS]++;
                  if (ck(LOGICAL_BLOCK_HEADERS) || full_dump) {
                    addr(-4);
                    printf("           %-9d ",count[LOGICAL_BLOCK_HEADERS]);
                    printf("LOGICAL BLOCK HEADER,%s      ccnn = 1803 (1)\n"
                      ,sp);
                    printf("           end of frame\n");
                  }
                  if (end_vrf) {
                    count[A_SUMMARY_AFTER_EACH_COLOR_PASS]++;
                    if (ck(A_SUMMARY_AFTER_EACH_COLOR_PASS) || full_dump)
                      pass_sum();
                    resetsum();
                    if (last_pass) plot_sum();
                  }
                  if (last_pass == 1) { /* kick our for 2.0 data */
                    done = 1;
                  }
                } else
                  /***************************************************/
                  /* I give up!! It must be an error in the Vrf file */
                  /* MT: code modified to support REP.  no 1803 0000 */
                  /***************************************************/
                {
                  if ((can == 0x18) && (stx == 0x01) &&
                  (zeros1 == 0x00) && (zeros2 == 0x00)) {
                    readfil(&can);
                    readfil(&stx);
                    readfil(&zeros1);
                    readfil(&zeros2);
                    if ((can == 0x18) && (stx == 0x02) &&
                    (zeros1 == 0x00) && (zeros2 == 0x00)) {
                      if (end_vrf) {
                        count[A_SUMMARY_AFTER_EACH_COLOR_PASS]++;
                        if (ck(A_SUMMARY_AFTER_EACH_COLOR_PASS) || full_dump)
                          pass_sum();
                        resetsum();
                        if (last_pass) plot_sum();
                      }
                      count[LOGICAL_BLOCK_HEADERS]++;
                      if (ck(LOGICAL_BLOCK_HEADERS) || full_dump) {
                        addr(-8);
                        printf("           %-9d ",count[LOGICAL_BLOCK_HEADERS]);
                        printf("LOGICAL BLOCK HEADER,%s      ",sp);
                        printf("ccnn = 1801 (3)\n");
                        printf("           expecting end of frame\n");
                        printf("           assuming REP controller\n");
                        addr(-8);
                        printf("           frame sync header\n");
                      }
                      plot_print = 1;
                      pass_print = 1;
                    } else {
                      printf("\nERROR: Expecting 1802 0000\n");
                      printf("in FRAME SYNC HEADER\n");
                      printf("found %X",number(can,stx));
                      printf(" %X\n",number(zeros1,zeros2));
                      printf("offset of bytes into file is ");
                      printf("%X (hex)\n",off_set);
                      exit(1);
                    }
                  } else {
                    printf("\nERROR: Byte count exhausted\n");
                    printf("found %X\n", comand);
                    printf("expecting 1802 for next logical block sync\n");
                    printf("offset of bytes into file is %X (hex)\n",off_set);
                    exit(1);
                  }
                }
              }
            }
            while (!done);
            /* while ((!done) || ((!end_vrf) && (canetxok))); */
            /*******************************/
            /* end of IF can stx ZERO ZERO */
            /*******************************/
          } else {
            printf("\nERROR: CAN STX 0000 sequence not found\n");
            printf("unable to parse found %X\n", comand);
            printf("expecting 1802 0000 for initial frame sync\n");
            printf("offset of bytes into file is %X (hex)\n",off_set);
            exit(1);
          }
        }
         /* 00 00 */ 
        else
          /****************************************************/
          /* Well  it's not an CAN SOH Zero Zero, What is it? */
          /* Is it an CAN ETX Zero Zero sequence ?            */
          /****************************************************/
        {
          if ((can == 0x18) && ((soh == 0x03) || (stx == 0x03))
           && (zeros1 == 0x00) && (zeros2 == 0x00))
            /*********************/
            /* If so, then go on */
            /*********************/
          {
            done = 1;
            canetxok = 1;
            count[LOGICAL_BLOCK_HEADERS]++;
            if (ck(LOGICAL_BLOCK_HEADERS) || full_dump) {
              addr(-4);
              printf("           %-9d ",count[LOGICAL_BLOCK_HEADERS]);
              printf("LOGICAL BLOCK HEADER,%s      ccnn = 1803 (1)\n",sp);
              printf("           end of frame\n");
            }
            if (end_vrf) {
              count[A_SUMMARY_AFTER_EACH_COLOR_PASS]++;
              if (ck(A_SUMMARY_AFTER_EACH_COLOR_PASS) || full_dump)
                pass_sum();
              resetsum();
              if (last_pass) plot_sum();
            }
            if (last_pass == 1) {       /* kick our for 2.0 data */
              done = 1;
            }
          }
          /*************************************/
          /* Well it was not that, what is it? */
          /* Maybe a VDS level1 NO-Operation   */
          /*************************************/
          else if ((can == 0x18) && (soh == 0x02) &&
                   (zeros1 == 0x00) && (zeros2 == 0x00)) {
            count[LOGICAL_BLOCK_HEADERS]++;
            if (ck(LOGICAL_BLOCK_HEADERS) || full_dump) {
              addr(-4);
              printf("           %-9d ",count[LOGICAL_BLOCK_HEADERS]);
              printf("LOGICAL BLOCK HEADER,%s      ccnn = 1802 (1)\n",sp);
              printf("           continutation header\n"); 
              printf("           bytcnt = 0\n"); 
            }
          } else
            /*----------------------------------*/
            /* Check to see we are really done  */
            /* if so flag it ...                */
            /*----------------------------------*/
          if (last_pass == 1) { /* kick our for --> data */
            done = 1;
          } else
            /********************************************/
            /* I give up it must be an error in the VRF */
            /* data file . . .                          */
            /********************************************/
          {
            if (vrfplot_started) {
              printf("\nERROR: Unrecognized Vds Command\n");
              printf("no cancel, start of header found.  Found");
              printf("%X %X\n",number(can, soh),number(zeros1,zeros2));
              printf("unable to parse, Non-VDS file encountered\n");
              printf("offset of bytes into file is %X (hex)\n",off_set);
              exit(1);
            } else
              printdta(zeros1);
            /* Note: zeros2 was also parsed but not passed */
          }
        }
      } else
        printdta(soh);
    } else
      printdta(can);
  }                             /* end of do */
  /********************************/
  /* do the whole mess til hear   */
  /* while alldone is false, else */
  /* keep looping...              */
  /* All_done is controled by     */
  /* readfil() and is trigered    */
  /* when an eof() is reached...  */
  /********************************/
  while (!alldone);
}

/*------------------------------------------------------------------------*/
/* Program : Vrfdmpr.pas or Vrfdmpr.c
   Module  : Poscom.pas or Poscom.c
   Author  : W.S. Harper
   Language: Origionally written in (Turbo) Pascal, but it
             was re-written in (Lattice) C.
   Purpose : Poscom is short for  VRF positioning commands.  This module
             is called from the parser to handle these types of commands.
             It will then print out the type of command it found, i.e.
             a MOVE command or a DRAW command.
   History : 7/23/87 -- Modified to handle 32 bit precision.  This was a
             design flaw on the origional version.
             10-11-87 -- Toomey Freeman found bug in move command logic
             in this routine.  When I caculated the integer for the nib
             coordinates I forgot to mask of the high order MOVE/DRAW
             bit.  Thats a good one to get, Thanks Toomey!!!              */
/*------------------------------------------------------------------------*/


static poscom(bytin)
int bytin;
            {
              if (command_started == 0) {
                readfil(&byt2);
                bytcnt = bytcnt - 1;
                command_started = 1;
                vv31_bit_pos_wrdcnt = 1;
              }
              if (bytcnt > 0) {
                if (v31_bit_prec)
                  /***************************************
                   *  32 bit precision cordinates have   *
                   *  been invoked.  So now everything   *
                   *  is done in 32 bit coordinates.     *
                   *  You bit the dice, now you pay      *
                   *  the price.  Get VCGL ..            *
                   ***************************************/
                {
                  readfil(&byt3);
                  readfil(&byt4);
                  vv31_bit_pos_wrdcnt = vv31_bit_pos_wrdcnt + 1;
                  bytcnt = bytcnt - 2;
                  switch (vv31_bit_pos_wrdcnt) {
                  case (2):{
                      xtmph = number(bytin, byt2);
                      xtmpl = number(byt3, byt4);
                      break;
                    }
                  case (3):{
                      byt5 = byt3;
                      byt3 = (0x7FFF & byt3);
                      ytmph = number(byt3, byt4); 
                      break; 
                    }

                  case (4):{
                      ytmpl = number(byt3, byt4);
                      l_ycord = lnumber(ytmph, ytmpl);
                      l_xcord = lnumber(xtmph, xtmpl);
                      command_started = 0;
                      if (byt5 > 0x7F) {
                        count[DRAW_COMMANDS]++;
                        if (ck(DRAW_COMMANDS) || full_dump) {
                          addr(-8);
                          printf("           %-9d ",count[DRAW_COMMANDS]);
                          printf("DRAW COMMAND,%s              ",sp);
                          printf("ccnn = %X (3)\n",xtmph);
                          printf("           31 bit precision\n");
                          if ( x_cur != -1 ) {
                            printf("           current position x = ");
                            printf("%u, y = %u\n",x_cur,y_cur);
                          }
                          printf("           x = %u\n",l_xcord&0x7fffffff);
                          printf("           y = %u\n",l_ycord&0x7fffffff);
                        }
                        x_cur = l_xcord&0x7fffffff;
                        y_cur = l_ycord&0x7fffffff;
                      } else if (byt5 < 0x7F) {
                        count[MOVE_COMMANDS]++;
                        if (ck(MOVE_COMMANDS) || full_dump) {
                          addr(-8);
                          printf("           %-9d ",count[MOVE_COMMANDS]);
                          printf("MOVE COMMAND,%s              ",sp);
                          printf("ccnn = %X (3)\n",xtmph);
                          printf("           31 bit precision\n");
                          if ( x_cur != -1 ) {
                            printf("           current position x = ");
                            printf("%u, y = %u\n",x_cur,y_cur);
                          }
                          printf("           x = %u\n",l_xcord&0x7fffffff);
                          printf("           y = %u\n",l_ycord&0x7fffffff);
                        }
                        x_cur = l_xcord&0x7fffffff;
                        y_cur = l_ycord&0x7fffffff;
                      } else {
                        printf("\nERROR: Unrecognized VRF 32 Bit ");
                        printf("Positioning Command\n");
                        printf("found %X %X", l_xcord, l_ycord);
                        printf(", expecting 0<x<7F00\n");
                        printf("Byte count offset into ");
                        printf("file %X (hex)\n",off_set);
                        exit(1);
                      }
                      break;
                    }           /* end case (4) */
                  }             /* end of case statement */
                } else
                  /*********************/
                  /* Standard (default) */
                  /* 15 bit precision  */
                  /*********************/
                {
                  readfil(&byt3);
                  readfil(&byt4);
                  bytcnt = bytcnt - 2;
                  command_started = 0;
                  if (byt3 > 0x7F) {
                    count[DRAW_COMMANDS]++;
                    if (ck(DRAW_COMMANDS) || full_dump) {
                      addr(-4);
                      printf("           %-9d ",count[DRAW_COMMANDS]);
                      printf("DRAW COMMAND,%s              ",sp);
                      printf("ccnn = %X (1)\n",number(bytin,byt2));
                      printf("           15 bit precision\n");
                      if ( x_cur != -1 ) {
                        printf("           current position x = ");
                        printf("%u, y = %u\n",x_cur,y_cur);
                      }
                      printf("           x = %u\n",number(bytin,byt2)&0x7fff);
                      printf("           y = %u\n",number(byt3, byt4)&0x7fff);
                    }
                    x_cur = number(bytin,byt2)&0x7fff;
                    y_cur = number(byt3,byt4)&0x7fff;
                  } else if (byt3 < 0x7F) {
                    count[MOVE_COMMANDS]++;
                    if (ck(MOVE_COMMANDS) || full_dump) {
                      addr(-4);
                      printf("           %-9d ",count[MOVE_COMMANDS]);
                      printf("MOVE COMMAND,%s              ",sp);
                      printf("ccnn = %X (1)\n",number(bytin,byt2));
                      printf("           15 bit precision\n");
                      if ( x_cur != -1 ) {
                        printf("           current position x = ");
                        printf("%u, y = %u\n",x_cur,y_cur);
                      }
                      printf("           x = %u\n",number(bytin,byt2)&0x7fff);
                      printf("           y = %u\n",number(byt3, byt4)&0x7fff);
                    }
                    x_cur = number(bytin,byt2)&0x7fff;
                    y_cur = number(byt3,byt4)&0x7fff;
                  } else {
                    printf("\nERROR: Unrecognized VRF 16 Bit ");
                    printf("Positioning Command\n");
                    printf("found %X %X, expecting 0<x<7F00\n", bytin, byt2);
                    printf("Byte count offset into file: %X (hex)\n",off_set);
                    exit(1);
                  }
                }
              }
            }
/*------------------------------------------------------------------------*/
/* Program : Vrfdmpr.c
   Module  : Printdta.c
   Author  : W.S. Harper
   Language: MicroSoft C version 5.0
   Purpose : Designed to be called from the parser when ever print type
             data was received before a VDS header Sync.
   History : 2/15/87 - Written as extention of Vrfdmpr() utility by
             popular request.  I segmented this portion so I could
             do something futher with the print data received.
             (I.E. Check for Valid ascII range then print to screen
              if flag in command selected was chosen.)
             5/6/88 - Added the output of Print data received by this
             routine; Print it Hex and AscII ( If possible).  This will
             only be enabled when in Full Dump MODE or in Command Select
             MODE and have the PRINT data option flaged for viewing.  Also
             changed the message for PRINT data received when in Summary
             mode.  The old way ate up too much screen, condenced to one
             line.                                                        */
/*------------------------------------------------------------------------*/

static printdta(printin)
int printin;
{
  if ((full_dump) || (view_print == 'Y')) {
    if (!recv_prt) {
      printf(" - Receiving Print Data ...\n");
      recv_prt = 1;
    } else if (((printin > 31) && (printin < 126)) ||
              ((printin == 0x0d) || (printin == 0x0a)))
      printf("%c", printin);
    else {
      printf("\n");
      printf("AscII Non-Printable found = %X\n", printin);
    }
  } else if ((!recv_prt) && (!full_dump) &&
             (view_print == 'N') && (dmpmode == 'S')) {
    recv_prt = 1;
    printf(" - Some Print Data was Received ...\n");
  }
}

/*---------------------------------------------------------------------*/
/*
Program  : Vrfdmpr()
Module   : Readfil.c
Author   : W.S. Harper
History  : 12-26-86 - Ported Turbo Pascal version of readfil routine to C.
                      Had to do a compleat re-write of the code because of
                      the differences between Turbo pascal and C.
           9/14/87  - Added comments to explain what the heck it is
                      that is going on in this routine.
           10/10/87 - Added BUFFERING within this routine to speed up
                      the code.  It made about a 15x improvement.
                      With some other modifications the program
                      has a 25x speed improvemens.  Note this buffering
                      was suggested by New Product Technical Support
                      and Octpus Enterprises.  Thanks for the HELP !!!

Purpose  : The  purpose of Readfil() is to be callable from anywhere
           with the Vrfdmpr() program.  It mearly pulls a byte of data
           from a file.  It uses a standard C read() call which means the
           C compiler's libraries will buffer the data read from the file
           somewhat.  This routine could really be tweeked up if rewritten in
           assemble.  But it was kept standard C because  the  Author reserves
           the right to re-port the code to other C compilers and systems.
                                                                           */
/*---------------------------------------------------------------------*/

static readfil(byt)
int *byt;
{
  static int c = 0,swap_flag = 0,swap_correct,unlocked = 1,vms_spooled = 0;
  static char *buf_ptr;
  static char lbuf[32768];

  if (c == 0) {
    addr(1);
    c = F_READ(lbuf,1,32768,fd);
    buf_ptr = lbuf;
    if (c > 0) {
      if (unlocked) {
        unlocked = 0;
        switch( ((lbuf[0]&0xff) << 8) + (lbuf[1]&0xff) ) {

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
            if ( c == 2 ) {
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
            if ( c == 2 )  vms_spooled++;
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
          switch( ((lbuf[0]&0xff) << 8) + (lbuf[1]&0xff) ) {
            case 0x9100:
              swap_flag = 1;
              buf_ptr += 2;
              c -= 2;
              off_set += 2;
            break;
            case 0x1100:
              swap_flag = 0;
              buf_ptr += 2;
              c -= 2;
              off_set += 2;
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
              if ( c == 2 ) {
                buf_ptr += 2;
                c -= 2;
                off_set += 2;
              }
            break;
            case 0x1200:
              swap_flag = 0;
              buf_ptr += 2;
              c -= 2;
              off_set += 2;
            break;
            case 0x9200:
              swap_flag = 1;
              buf_ptr += 2;
              c -= 2;
              off_set += 2;
            break;
            default:
              addr(0);
              printf("           WARNING: ");
              printf("expecting VMS spooler control code\n");
            break;
          } 
        } else {
          switch( ((lbuf[0]&0xff) << 8) + (lbuf[1]&0xff) ) {
            case 0x0091:
              swap_flag = 1;
              buf_ptr += 2;
              c -= 2;
              off_set += 2;
            break;
            case 0x0011:
              swap_flag = 0;
              buf_ptr += 2;
              c -= 2;
              off_set += 2;
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
              if ( c == 2 ) {
                buf_ptr += 2;
                c -= 2;
                off_set += 2;
              }
            break;
            case 0x0012:
              swap_flag = 0;
              buf_ptr += 2;
              c -= 2;
              off_set += 2;
            break;
            case 0x0092:
              swap_flag = 1;
              buf_ptr += 2;
              c -= 2;
              off_set += 2;
            break;
            default:
              addr(0);
              printf("           WARNING: ");
              printf("expecting VMS spooler control code\n");
            break;
          }
        }
      }
    }
    if ( swap_flag || swap_correct )
      if ( swap_flag != swap_correct) swap(buf_ptr,c); 
  }
  if (c > 0) {
    --c;
    off_set++;
    *byt = ((*buf_ptr) & 0x00ff);       /* MASK off the High bits */
    buf_ptr++;
  } else if (c == 0) {
    if (!last_pass) {
      printf("\nI/O ERROR: while reading input data set\n");
      printf("physical eof, done parsing data set\n");
      printf("done parsing data set before the end of plot was found\n");
      printf("bad data or file, termination within level 1 command mode\n");
      printf("byte offset into file is: %X (hex)\n",off_set);
      if (plot_print) plot_sum();
      exit();
    } else {
      done = 1;
      alldone = 1;
      if (pass_print) {
        count[A_SUMMARY_AFTER_EACH_COLOR_PASS]++;
        if (ck(A_SUMMARY_AFTER_EACH_COLOR_PASS) || full_dump)
          pass_sum();
        resetsum();
      }
      if (plot_print) plot_sum();
      exit();
    }
  } else if (c == -1) {
    printf("I/O Error: while reading input\n");
    printf("Error Ocurred in READ operation\n");
    printf("file or disk drive may be corrupt\n");
    printf("Byte offset into file is: %X (hex)\n",off_set);
    if (pass_print) {
      count[A_SUMMARY_AFTER_EACH_COLOR_PASS]++;
      if (ck(A_SUMMARY_AFTER_EACH_COLOR_PASS) || full_dump)
        pass_sum();
      resetsum();
    }
    if (plot_print) plot_sum();
    exit();
  }
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

/*-----------------------------------------------------------------*/
/* PROGRAM: VRFDMPR.pas or Vrfdmpr.c
   MODULE : RESETSUM.pas or resetsum.c
   PURPOSE: RESETS  GLOBOL VARIBLES FOR THE LEVEL 2 COMMAND  MODULES
            USED. (IE... NONPOSCO.PAS, POSCOM.PAS)
   History : Origionally written in pascal but rewritten in C
             1-13-89 changed all counters for single color passes
             from integer varables to long varables.
                                                                   */
/*-----------------------------------------------------------------*/

static resetsum()
{
  int i;

  for ( i = 0 ; i < NUM_VRF_COMMANDS ; i++ ) {
    sum_cmd[i][color_pass] += count[i] - sum_cmd[i][4];
    sum_cmd[i][4] = count[i];
  }
}

/*---------------------------------------------------------------------*/
/*Program:Vrfdmpr.pas or Vrfdmpr.c
  Module: Setup.dpr or Setup.c
  Purpose: Used to set up the flag masks for certain VRF
           commands.  This is used only if running partial
           command selected dump mode .
  History: Origionally written in (Turbo pascal) but rewritten
           in Generic C.

                                                                       */
/*---------------------------------------------------------------------*/

static setup(rc_file)
char *rc_file;
{

#define ALL_SIZ  83
#define OPT_SIZ  17
#define BUF_SIZ 240

  static char *menu_options[] = {
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
    "VDS_LOGICAL_BLOCK_HEADERS",
    "VDS_LEVEL_1_COMMANDS",
    "VDS_LEVEL_2_COMMANDS",
    "VMS_RASTER_PRINT",
    "VMS_RASTER_PLOT",
    "VMS_RASTER_REMOTE",
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

  int i,match,s_output,e_output;
  FILE *fp;
  char buf[BUF_SIZ],cmd[BUF_SIZ];

  if ( rc_file == NULL )
    fp = fopen("vgsdump.dat","r");
  else
    fp = fopen(rc_file,"r");

  if (fp != NULL) {
    while (fgets(buf,BUF_SIZ,fp) != NULL) {
      if (sscanf(buf,"%s%d%d",cmd,&s_output,&e_output) > 0 ) {
        if ( *cmd == '*' ) continue;
        match = 0;
        fold_upper(cmd);
        for ( i = 0 ; i < ALL_SIZ ; i++ ) {
          if (strcmp(cmd,menu_options[i]) == 0) {
            match = 1;
            if ( i < OPT_SIZ ) {
              start_output[i] = s_output;
              end_output[i] = e_output;
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
  view_print      = 'N';
}

static fold_upper(buf)
char *buf;
{
  int i;

  for ( i = 0 ; i < strlen(buf) ; i++ )
    if ((buf[i] >= 'a') && (buf[i] <= 'z' ))
      buf[i] += 'A' -'a';
}

/*--------------------------------------------------------------------------*/
/* Program: Vrfdmpr.pas or Vrfdmpr.c
   Module:  Summary.dpr or summary.c
   Purpose: This routine is called (If opted) at the end of each color pass
            to give a summary of the VRF commands.
   History: 1-19-86  - Origionally written in Pascal.
            12-28-86 - Re-written in C.
            1-13-89  - Changed all color summary varables from int to long in
                       effort to cure bug.
            1-13-89  - Changed all color summary varables from int to long in
                       effort to cure bug.
                       Changed screens to fancy PC screens with extened ASC
                       characters.

*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
static pass_sum()
{
  pass_print = 0;

  printf("\nSummary for ");
  switch (color_pass) {
    case 0:
      printf("black");
    break;
    case 1:
      printf("cyan");
    break;
    case 2:
      printf("magenta");
    break;
    case 3:
      printf("yellow");
    break;
  }
  printf(" color pass in Plot %u\n",plot_frame+1);

  printf(" %9u DEFINE PATTERN COMMANDS\n",
         count[DEFINE_PATTERN_COMMANDS] -
         sum_cmd[DEFINE_PATTERN_COMMANDS][4]);
  printf(" %9u DEFINE PEN COMMANDS\n",
         count[DEFINE_PEN_COMMANDS] -
         sum_cmd[DEFINE_PEN_COMMANDS][4]);
  printf(" %9u DRAW CIRCLE COMMANDS\n",
         count[DRAW_CIRCLE_COMMANDS] -
         sum_cmd[DRAW_CIRCLE_COMMANDS][4]);
  printf(" %9u DRAW COMMANDS\n",
         count[DRAW_COMMANDS] -
         sum_cmd[DRAW_COMMANDS][4]);
  printf(" %9u DRAW POLYGON COMMANDS\n",
         count[DRAW_POLYGON_COMMANDS] -
         sum_cmd[DRAW_POLYGON_COMMANDS][4]);
  printf(" %9u LOGICAL VDS BLOCK COUNT\n",
         count[LOGICAL_BLOCK_HEADERS] -
         sum_cmd[LOGICAL_BLOCK_HEADERS][4]);
  printf(" %9u MOVE COMMANDS\n",
         count[MOVE_COMMANDS] -
         sum_cmd[MOVE_COMMANDS][4]);
  printf(" %9u SET FONT BASELINE COMMANDS\n",
         count[SET_FONT_BASE_LINE_COMMANDS] -
         sum_cmd[SET_FONT_BASE_LINE_COMMANDS][4]);
  printf(" %9u SET FONT COMMANDS\n",
         count[SET_FONT_COMMANDS] -
         sum_cmd[SET_FONT_COMMANDS][4]);
  printf(" %9u SET PEN COMMANDS\n",
         count[SET_PEN_COMMANDS] -
         sum_cmd[SET_PEN_COMMANDS][4]);
  printf(" %9u SKIP WORD COMMANDS\n",
         count[SKIP_COMMANDS] -
         sum_cmd[SKIP_COMMANDS][4]);
  printf(" %9u TEXT STRING COMMANDS\n",
         count[TEXT_STRING_COMMANDS] -
         sum_cmd[TEXT_STRING_COMMANDS][4]);
}

static charhex(val)
int val;
{
  if ( val < 0x10 )
    printf("0x0%X ",val);
  else
    printf("0x%X ",val);
}

static addtoning(color)
int color;
{
  int i = 0;

  while ( (i != 4) && ((ton_seq[i] != -1) && (ton_seq[i] != color))) i++;
  if ( i != 4 ) 
    if ( ton_seq[i] != color ) ton_seq[i] = color; 
}

static print_pass(txt,index)
int index;
char txt[MSIZE];
{
  int tmp,pass,total = 0;

  for ( pass = 0 ; pass < 4 ; pass++)
    switch(ton_seq[pass]) {
      case 0:
      case 1:
      case 2:
      case 3:
        tmp = sum_cmd[index][ton_seq[pass]];
        printf(" %9u",tmp);
        total += tmp;
      default:
      break;
    }
  printf(" %9lu  %s\n",total,txt);
}

static int ck(option)
int option;
{
  return((start_output[option] <= count[option]) &&
         (count[option] <= end_output[option]));
}

static addr(rew)
int rew;
{
#ifdef vms
  static int current_rec = 0, begin_offset = 0, previous_offset;

  if ( rew == 1 ) {
    previous_offset = begin_offset;
    current_rec++;
    begin_offset = off_set;
  } else {
    printf("\nbyte 0x%X, ",off_set+rew);
    if ( (off_set - begin_offset + rew ) >= 0 ) {
      printf("record %d, ",current_rec);
      printf("offset 0x%X\n",off_set - begin_offset + rew);
    } else {
      printf("record %d, ",current_rec - 1);
      printf("offset 0x%X\n",off_set - previous_offset + rew);
    }    
  }
#else
  if ( rew != 1 ) printf("\nbyte 0x%X:\n",off_set+rew);
#endif
}
