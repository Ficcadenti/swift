/*
 *	xrtph1sim.h: definitions and declarations with global relevance
 *
 *	AUTHOR:
 *
 *      Barbara Saija gio set 27 17:33:13 CEST 2001 
 *     
 */
#ifndef XRTPH1SIM_H
#define XRTPH1SIM_H
#include <stdio.h>	/* Note: for IRAF compatibility,
			standard I/O calls should NOT be used. */
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <fitsio.h>
#include <pil.h>

#include <misc.h>
#include <highfits.h>
#include <xrt_ran2.h>
#include <astro.h>

#include <xrtdasversion.h>
#include <xrtfits.h>
#include <xrtdefs.h>
#include <xrt_termio.h>

                  /********************************/
		  /*      defines / typedefs      */
		  /********************************/

#define PRG_NAME "xrtph1sim"
#define VERSION  "0.1.1"

#define BUFSIZE 81
#define PHAS_REPEAT   9
#define PHAS_REPEAT_DIM   "9I"
#define FLEN_STRING   66 /* maximum keyword string lenght */ 
#define MAXSIZE       1000

#define PAR_INFILE       "infile"    
#define PAR_OUTFILE      "outfile"
#define PAR_VERBOSITY    "verbosity"
#define PAR_CLOBBER      "clobber"
#define PAR_DATEOBS      "dateobs"
#define PAR_TIMEOBS      "timeobs"

#define KWNM_RADECSYS    "RADECSYS"         /* stellar reference frame */
#define KWVL_RADECSYS           "FK5"        
#define CARD_COMM_RADECSYS      "stellar reference frame"

#define KWNM_EQUINOX            "EQUINOX"          /* Julian epoch for RADECSYS                    */
#define KWVL_EQUINOX            2000.0         
#define CARD_COMM_EQUINOX       "Julian epoch for RADECSYS"

#define KWNM_RA_PNT             "RA_PNT"           /* median RA during this data set - TBD Format              */
#define CARD_COMM_RA_PNT        " TBD median RA during this data set"

#define KWNM_RA_NOM             "RA_NOM"           /* median RA during this data set               */
#define CARD_COMM_RA_NOM        "median RA during this data set"

#define KWNM_DEC_PNT            "DEC_PNT"          /* median DEC during this data set              */
#define CARD_COMM_DEC_PNT       "median DEC during this data set"

#define KWNM_DEC_NOM            "DEC_NOM"          /* median DEC during this data set              */
#define CARD_COMM_DEC_NOM       "median DEC during this data set"

#define KWNM_RA_SCX             "RA_SCX"           /* spacecraft X-axis RA pointing direction      */
#define CARD_COMM_RA_SCX        "spacecraft X-axis RA pointing direction"

#define KWNM_DEC_SCX            "DEC_SCX"          /* spacecraft X-axis DEC pointing direction     */
#define CARD_COMM_DEC_SCX       "spacecraft X-axis DEC pointing direction"

#define KWNM_RA_SCY             "RA_SCY"           /* spacecraft Y-axis RA pointing direction      */
#define CARD_COMM_RA_SCY        "spacecraft Y-axis RA pointing direction"

#define KWNM_DEC_SCY            "DEC_SCY"          /* spacecraft Y-axis DEC pointing direction     */
#define CARD_COMM_DEC_SCY       "spacecraft Y-axis DEC pointing direction"

#define KWNM_RA_SCZ             "RA_SCZ"           /* spacecraft Z-axis RA pointing direction      */
#define CARD_COMM_RA_SCZ        "spacecraft Z-axis RA pointing direction"

#define KWNM_DEC_SCZ            "DEC_SCZ"          /* spacecraft Z-axis DEC pointing direction     */
#define CARD_COMM_DEC_SCZ       "spacecraft Z-axis DEC pointing direction"

#define KWNM_TCTYP4             "TCTYP4"          /* TAN projection used */
#define KWVL_TCTYP4             "RA---TAN"
#define CARD_COMM_TCTYP4        "TAN  projection used"                          
#define KWNM_TCRPX4             "TCRPX4"           /* reference pixel                              */
#define KWVL_TCRPX4              300
#define CARD_COMM_TCRPX4        "reference pixel"


#define KWNM_TCRVL4             "TCRVL4"           /* TBD reference value                              */
#define CARD_COMM_TCRVL4        "TBD reference pixel"   

#define KWNM_TCDLT4             "TCDLT4"           /* TBD increment per pixel                          */
#define CARD_COMM_TCDLT4        "TBD increment per pixel"                          

#define KWNM_TCUNI4             "TCUNI4"           /* physical units of axis 1                     */
#define KWVL_TCUNI4              UNIT_DEG
#define CARD_COMM_TCUNI4        "physical units of axis 1"                     

#define KWNM_TCTYP5             "TCTYP5"          /* TAN projection used                          */
#define KWVL_TCTYP5             "DEC--TAN"
#define CARD_COMM_TCTYP5        "TAN  projection used" 

#define KWNM_TCRPX5             "TCRPX5"           /* reference pixel                              */
#define KWVL_TCRPX5              300
#define CARD_COMM_TCRPX5        "reference pixel"

#define KWNM_TCRVL5             "TCRVL5"           /* TBD reference value                              */
#define CARD_COMM_TCRVL5        " TBD reference pixel" 

#define KWNM_TCDLT5             "TCDLT5"           /* TBD increment per pixel                          */

#define CARD_COMM_TCDLT5        "increment per pixel" 

#define KWNM_TCUNI5             "TCUNI5"           /* physical units of axis 2                     */
#define KWVL_TCUNI5              UNIT_DEG
#define CARD_COMM_TCUNI5        "physical units of axis 1"                    

#define KWNM_TCROT5             "TCROT5"           /* TBD image rotation (degrees)*/

#define CARD_COMM_TCROT5        "TBD image rotation (degrees)"


#define KWNM_TPC0404            "TPC0404"          /* TBD Coord. Descrp. Matrix: cos(TCROT4)           */
#define CARD_COMM_TPC0404       "TBD Coord. Descrp. Matrix: cos(TCROT4)"

#define KWNM_TPC0505            "TPC0505"          /* TBD Coord. Descrp. Matrix: -sin(TCROT4)           */
#define CARD_COMM_TPC0505       " TBD Coord. Descrp. Matrix: -sin(TCROT4)"

#define KWNM_TPC0405            "TPC0405"          /* TBD Coord. Descrp. Matrix:  sin(TCROT4)           */

#define CARD_COMM_TPC0405       " TBD Coord. Descrp. Matrix: sin(TCROT4)"

#define KWNM_TPC0504            "TPC0504"          /* TBD Coord. Descrp. Matrix:  cos(TCROT4)           */

#define CARD_COMM_TPC0504       "TBD Coord. Descrp. Matrix: cos(TCROT4)"
                                              

#define KWVL_OBS_ID    "00000001001"
#define KWVL_RA_SCX    0.0
#define KWVL_DEC_SCX   0.0
#define KWVL_RA_SCY    0.0
#define KWVL_DEC_SCY   0.0
#define KWVL_RA_SCZ    0.0
#define KWVL_DEC_SCZ   0.0
#define KWVL_TCRVL4    0.0 /*TBD*/
#define KWVL_TCDLT4    0.0 /*TBD*/
#define KWVL_TCRVL5    0.0 /*TBD*/
#define KWVL_TCDLT5    0.0
#define KWVL_TCROT5    0.0 /*TBD*/
#define KWVL_TPC0404   0.0 /*TBD*/
#define KWVL_TPC0505   0.0 /*TBD*/
#define KWVL_TPC0405   0.0 /*TBD*/
#define KWVL_TPC0504   0.0 /*TBD*/

#define KWNM_TZERO3             "TZERO3"           /* CCDFrame was 4 byte unsigned remap FITS J*/
/*
 #define KWVL_TZERO3             2147483648
*/

#define CARD_COMM_TZERO3        "CCDFrame was 4 byte unsigned remap FITS J"

#define KWVL_TIMESYS            1993.0
#define BINTAB_ROWS             10

typedef struct {
  struct{
    char infile[FLEN_FILENAME];
    char outfile[FLEN_FILENAME];
    Verbosity_t verbosity;
    int clobber;
    char dateobs[11];
    char timeobs[9];
  } par;
  char taskname[FLEN_FILENAME];  /* Name of the task */
  char tasknamev[FLEN_FILENAME]; /* Name and version of the task */
  Version_t swxrtdas_v;          /* SWXRTDAS version */
  BOOL hist;
  char strhist[256];
  char date[25];
  double mjdtimesys;
  double timesys;
  double tstart;
  double tstop;
  SPTYPE object;
  char creator[FLEN_VALUE];
} Global_t;

extern Global_t global;

typedef  struct {
  double time[MAXSIZE];
  int    tdhxi[MAXSIZE];
  int    ccdframe[MAXSIZE];
  int    x[MAXSIZE];
  int    y[MAXSIZE];
  int    rawx[MAXSIZE];
  int    rawy[MAXSIZE];
  int    detx[MAXSIZE];  
  int    dety[MAXSIZE];
  int    phas[MAXSIZE][PHAS_REPEAT];
} Ph1EventTbl;


typedef struct {
  unsigned TIME;
  unsigned TDHXI;
  unsigned CCDFrame;
  unsigned X;
  unsigned Y;
  unsigned RAWX;
  unsigned RAWY;
  unsigned DETX;
  unsigned DETY;
  unsigned PHAS; 
  unsigned STATUS; 
} EvCol_t;

typedef struct {
  unsigned TSTART;
  unsigned TSTOP;
} GTICol_t;

/*---------------------------------------------------------------*/
/*                  Add Block of Cards Macros                    */
/*---------------------------------------------------------------*/

#define ADD_CARD_BLOCK_HDU_E(head)\
        AddCard(head, KWNM_HDUCLASS, S, KWVL_HDUCLASS, CARD_COMM_HDUCLASS);\
        AddCard(head, KWNM_HDUCLAS1, S, KWVL_HDUCLAS1_E, CARD_COMM_HDUCLAS1_E);\
        AddCard(head, KWNM_TSORTKEY, S, KWVL_TSORTKEY, CARD_COMM_TSORTKEY); 

#define ADD_CARD_BLOCK_HDU_GTI(head)\
        AddCard(head, KWNM_HDUCLASS, S, KWVL_HDUCLASS, CARD_COMM_HDUCLASS);\
        AddCard(head, KWNM_HDUCLAS1, S, KWVL_HDUCLAS1_G, CARD_COMM_HDUCLAS1_G);\
        AddCard(head, KWNM_HDUCLAS2, S, KWVL_HDUCLAS2, CARD_COMM_HDUCLAS2);


#define ADD_CARD_BLOCK_COMMON(head)\
        AddCard(head,KWNM_TELESCOP, S, KWVL_TELESCOP ,CARD_COMM_TELESCOP);\
        AddCard(head,KWNM_INSTRUME, S, KWVL_INSTRUME, CARD_COMM_INSTRUME);
        

#define ADD_CARD_BLOCK_TIME2_E(head)\
        AddCard(head,  KWNM_CLOCKAPP, S, KWVL_CLOCKAPP, CARD_COMM_CLOCKAPP);\
        AddCard(head,  KWNM_TASSIGN, S, KWVL_TASSIGN, CARD_COMM_TASSIGN);
        
#define ADD_CARD_BLOCK_GTI(head)\
        AddCard(head,  KWNM_MTYPE1, S, KWVL_MTYPE1, CARD_COMM_MTYPE1);\
        AddCard(head,  KWNM_MFORM1, S, KWVL_MFORM1, CARD_COMM_MFORM1);\
        AddCard(head,  KWNM_METYP1, S, KWVL_METYP1, CARD_COMM_METYP1);\
        AddCard(head,  KWNM_TCUNI5, S, KWVL_TCUNI5, CARD_COMM_TCUNI5);


/********************************/
/*       functions prototypes    */
/********************************/
void disp_info(void);
int ReadFilePar(void);
FitsFileUnit_t WritePrimaryHeader(void);
int WriteEventBinTable(FitsFileUnit_t);
int WriteGTIBinTable(FitsFileUnit_t outunit);
int AddTimeKeyword(FitsHeader_t *);
int CalDateEnd(FitsHeader_t *);

#endif	
