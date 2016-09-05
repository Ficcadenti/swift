/*
 *	xrtimage.h: definitions and declarations for xrtimage
 *
 *	AUTHOR:
 *            Italian Swift Archive Center (FRASCATI)
 *  
 *     
 */
#ifndef XRTIMAGE_H
#define XRTIMAGE_H

/******************************
 *        header files        *
 ******************************/
#include <stdio.h>	/* Note: for IRAF compatibility,
			standard I/O calls should NOT be used. */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h> /* for getpid */

/* xrt local headers */
#include <highfits.h>
#include <xrt_termio.h>
#include <xrtfits.h>
#include <xrtdefs.h>
#include <misc.h>
#include <xrtdasversion.h>
#include <xrtcaldb.h>
#include <xrtbpfunc.h>


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

#define BINTAB_ROWS                1000

#define STEMLEN                      14
#define COUNTFITSEXT               "cn.img"


typedef struct {

  unsigned  naxis1;             /* NAXIS1 keyword  */
  unsigned  naxis2;             /* NAXIS2 keyword  */
  int       amp;                /* AMP keyword */
  int       nullval;            /* BLANK keyword  */
  double    refvalx;            /* CRVAL1 keyword */
  double    refvaly;            /* CRVAL2 keyword */
  double    refx;               /* CRPIX1 keyword */
  double    refy;               /* CRPIX2 keyword */
  double    pixdeltx;           /* CDELT1 keyword */
  double    pixdelty;           /* CDELT2 keyword */
  unsigned  xmax;
  unsigned  ymax;
  unsigned  xmin;
  unsigned  ymin;
  int       type;               /* Coordinate type flag: 0 - raw coordinates 
                                                         1 - det coordinates
                                                         2 - sky coordinates 
				   FitsImageType_t number
				*/
} ImageVars_t;

typedef enum { RAW = 0, DET, SKY }        FitsImageType_t;
char *FitsImageTypeString[] = { "RAW", "DET","SKY" };
int  FitsImageNullvalDef[] = { -1, -1 };
double FitsImageRefvalxDef[] = { 0, 1 };
double FitsImageRefvalyDef[] = { 0, 1 };
double FitsImageRefxDef[] = { 0, 0 };
double FitsImageRefyDef[] = { 0, 0 };
double FitsImagePixdeltxDef[] = { 1, 1 };
double FitsImagePixdeltyDef[] = { 1, 1};


typedef struct {
  /*----------   Input Parameters ---------------------*/
  struct {
    char    infile[PIL_LINESIZE];     /* Name of FITS file and [ext#] */
    char    outfile[PIL_LINESIZE];               /* Name of count image file */
  } par;
  /*------------------------------------------------------------------------*/
  char      taskname[FLEN_FILENAME];              /* Name and version of the task */
  Version_t swxrtdas_v;                           /* SWXRTDAS version */
  BOOL      hist;
  BOOL      first;
  char      strhist[256];
  char      date[25];
  char      tmpcount[FLEN_FILENAME];
  int       **count_table;
  ImageVars_t im;
  unsigned naxis1;
  unsigned naxis2;
  int      type;
  int      amp;
  double   refvalx;
  double   refvaly;
  double   refx;
  double   refy;
  double   pixdeltx;
  double   pixdelty;
} Global_t;

extern Global_t global;




/* I/O Data structure, photon counting columns from:  
Swift XRT Science FITS Format Design v0.6 June 29,2001) */

/* Prototypes */

int xrtimage2cnt();
int xrtimage2cnt_work();
int xrtimage2cnt_getpar();
void xrtimage2cnt_info(void);
int xrtimage2cnt_checkinput(void);
int CountImage (FitsFileUnit_t, int);
int InitImagingVars ( FitsHeader_t );
int GetCoordinateType ( FitsHeader_t );
int JValCust( FitsCard_t *, int *);
int DValCust( FitsCard_t *, double *);
int WriteCntKey(FitsFileUnit_t unit);

#endif
