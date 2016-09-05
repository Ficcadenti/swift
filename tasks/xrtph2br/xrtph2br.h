/*
 *	xrtph2br.h: definitions and declarations for xrtph2br
 *
 *	AUTHOR:
 *            Italian Swift Archive Center (FRASCATI)
 *  
 *     
 */
#ifndef XRTPH2BR_H
#define XRTPH2BR_H

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
#include <misc.h>
#include <xrtdasversion.h>
#include <xrtcaldb.h>
/* headas headers */
#include <fitsio.h>	 /*Required to use c_fcerr, c_fcecho, etc. */
#include <pil.h>
#include <hdcal.h>

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
#define PAR_SPLIT                  "split"
#define PAR_ASCAGRADE              "ascagrade"

/* miscellaneous */
#define BUFSIZE           1000        /* temporary strings buffer size */
#define CLEN_NAME           81        /* C Columns name length */
#define BINTAB_ROWS       1000

#define BRFITSEXT "br.evt1"
#define BRFFITSEXT "brf.evt1"
#define STEMLEN 14

typedef struct {
  int amp;
  int gradeid;
  int grade[PHAS_MOL];
} GradeRow_t;

typedef struct {
  /*----------   Input Parameters ---------------------*/
  struct {
    char infiles[MAX_FILES*PIL_LINESIZE];     /* Name of FITS file and [ext#] */
    char outfile[PIL_LINESIZE];               /* Name of FITS output file */
    char gradefile[PIL_LINESIZE];             /* Name of FITS CALDB grade file */
    int  split;                                /* Split threshold level   */
    BOOL ascagrade;
  } par;
  char *infile;
/*----------   Output Parameters ---------------------*/
  char origmode[FLEN_KEYWORD];
/*------------------------------------------------------------------------*/
  char taskname[PIL_LINESIZE]; /* Name and version of the task */
  Version_t swxrtdas_v;          /* SWXRTDAS version */
  GradeRow_t *gradevalues;
  int maxrows;
  BOOL hist;
  BOOL list;
  char strhist[256];
  char date[25];
  char tmpevtfilename[PIL_LINESIZE];
  int grade_tot[33];
  int grade_asca[8];
  int sat_event;
  int sat_asca;
  JTYPE grademin;
  JTYPE grademax;
} Global_t;

extern Global_t global;

/* I/O Data structure, photon counting columns from:  
Swift XRT Science FITS Format Design v0.6 June 29,2001) */

typedef struct {
  unsigned PHAS; 
  unsigned PHA; 
  unsigned ASCAPHA; 
  unsigned PI;
  unsigned GRADE;  
  unsigned ASCAGRADE;  
  unsigned ABOVE;
  unsigned Amp;
} Ev2Col_t;


/* Prototypes */

int xrtph2br();
int xrtph2br_work();
int xrtph2br_getpar();
void WriteInfoParams ();
int AddEventBintable(FitsFileUnit_t inevent,FitsFileUnit_t ounit);
int GetGrades(long);

#endif
