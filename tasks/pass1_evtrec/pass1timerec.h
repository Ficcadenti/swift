/*
 *	pass1timerec.h: definitions and declarations for pass1timerec
 *
 *	AUTHOR:
 *            Italian Swift Archive Center (FRASCATI)
 *  
 *     
 */
#ifndef PASS1TIMEREC_H
#define PASS1TIMEREC_H

/******************************
 *        header files        *
 ******************************/
#include <stdio.h>	/* Note: for IRAF compatibility,
			standard I/O calls should NOT be used. */
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* for getpid */

/* xrt local headers */
#include <highfits.h>
#include <xrt_termio.h>
#include <xrtfits.h>
#include <xrtdefs.h>
#include <xrtcaldb.h>
#include <misc.h>
#include <xrtdasversion.h>
/* headas headers */
#include <fitsio.h>	 /*Required to use c_fcerr, c_fcecho, etc. */
#include <pil.h>


#ifndef errno
extern int errno;
#endif

/*******************************
*      defines / typedefs      *
********************************/
/* input parameter names */

#define PAR_INFILE		   "infile"
#define PAR_OUTFILE		   "outfile"
#define PAR_GRADEFILE		   "gradefile"
#define PAR_EVENT 		   "event"
#define PAR_SPLIT 		   "split"
#define PAR_ADDCOL 		   "addcol"
#define PAR_DELNULL 		   "delnull"
#define PAR_OUTER                  "outer"
#define PAR_GUARDFLAG              "guardflag"



/* miscellaneous */
#define BUFSIZE           1000        /* temporary strings buffer size */
#define CLEN_NAME           81        /* C Columns name length */
#define BINTAB_ROWS       1000 

#define STEMLEN 14
#define PHAS3_MOL       3   
typedef struct {
  int gradeid;
  int grade[PHAS3_MOL];
} WTGradeRow_t;


/* I/O Data structure, windowed timing mode columns from:  
Swift XRT Science FITS Format Design v0.6 June 29,2001) */

typedef struct {
  unsigned CCDFrame;
  unsigned OFFSET;
  unsigned RAWX;
  unsigned RAWY;
  unsigned PHA;  
  unsigned EVTPHA;  
  unsigned PHAS; 
  unsigned GRADE;  
  unsigned ABOVE;
} Ev2Col_t;

typedef struct {
  int nrow;
  BOOL flag;
  int rawx;
} Ev2Info_t;


typedef struct {
  /*----------   Input Parameters ---------------------*/
  struct {
    char infiles[MAX_FILES*FLEN_FILENAME];     /* Name of FITS file and [ext#] */
    char outfile[FLEN_FILENAME]; /* Name of FITS output file */
    char gradefile[PIL_LINESIZE]; /* Name of FITS input grade file */
    int split;
    int event;
    int outer;
    BOOL addcol;
    BOOL delnull;
    BOOL guardflag;
  } par;
  char *infile;
/*------------------------------------------------------------------------*/
  char taskname[FLEN_FILENAME]; /* Name and version of the task */
  Version_t swxrtdas_v;          /* SWXRTDAS version */
  WTGradeRow_t *gradevalues;
  BOOL wt;
  BOOL pd;
  BOOL hist;
  BOOL list;
  BOOL evtrec;
  BOOL first;
  int onboardsplit;
  int onboardevent;
  int onboardouter;
  char strhist[256];
  char date[25];
  char tmpevtfilename[FLEN_FILENAME];
  int maxrows;
  int graderows;
  int totrows;
  int tot_sat;
  int grade[5];
  double per_grade[5];
  double per_tot_sat;
} Global_t;

extern Global_t global;
/* Prototypes */

int pass1timerec();
int pass1timerec_work();
int pass1timerec_getpar();
void WriteInfoParams ();
int WTEventRec(FitsFileUnit_t inevent,FitsFileUnit_t ounit);
int PDPUEventRec(FitsFileUnit_t inevent,FitsFileUnit_t ounit);
int EventRec3x3(int, int, Bintable_t , Ev2Col_t);
int EventRec3x1(int, int, Bintable_t , Ev2Col_t);
int WTEventRec3x1(int, int, Bintable_t , Ev2Col_t);
int faint3x1(float handle[PHAS3_MOL], float *pha, int *grade, int *above);
int wtfaint3x1(float handle[PHAS3_MOL], float *pha, int *grade, int *above);
int InitBintable(FitsHeader_t *head, Bintable_t *intable, unsigned *nCols, Ev2Col_t *indxcol, int blockn);
void  SetNullEvent(Bintable_t , int,  Ev2Col_t);
void PrintStatistics(void);
int DelCols(FitsFileUnit_t evunit);
int RemoveNullEvents(Bintable_t *table, unsigned *nRows, int colnum);
int AddStatKeywords(FitsFileUnit_t *evunit);
int GetGrades(long extno);
#endif
