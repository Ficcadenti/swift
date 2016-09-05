/*
 * 
 * File name: xrtph1sim.c
 * 
 * Task name: xrtph1sim
 * Description:
 *              Routine to create a Photon Counting Events FITS file
 *
 *
 * Author/Date: Barbara Saija September 2001
 *
 *
 * History:
 *        
 *
 * Notes:
 *
 *
 * Arguments:
 *
 *
 * Functions implemented in this file:
 *          int ReadFilePar(void); - reads the input parameter from 
 *                                   xrtph1sim.par
 *          int WritePrimaryHeader(fitsfile *); - generates the primary header
 *          int WriteEventBinTable(fitsfile *, char *); - generates the 
 *                                                        events extension
 *          int WriteGTIBinTable(fitsfile *, char *); - generates the GTI
 *                                                      extension
 *          int AddTimeKeyword(fitsfile *);
 *          void AddTimeStopKey(fitsfile *, double);
 *          float xrt_ran2(long *idum);
 *          void get_ran2seed(long *seed);
 *          void xrtph1sim(void); - main
 * 
 * Library routines called:
 *     libcfitsio libcftools:
 *       int PILGetFname(char *name, char* result);
 *       int PILGetString(char *name, char *result);
 * 	 int PILGetInt(char *name, int *result);
 * 	 int PILGetBool(char *name, int *result);
 *	 int headas_printf(const char* format, ...);
 *       int fits_str2time(char *datestr, int *year, int *month, int *day,
 *                     int *hour, int *minute, double *second, int *status);
 *   libswxrt:
 *          double ModJulday2(const struct tm Date, const double FracSecs);
 *          double Julday(const double day, const unsigned month, const unsigned year);
 *          double ModJulday(const double day, const unsigned month, const unsigned year);
 *          void ModJulday2Inv(double , struct tm *,double *);
 *          extern void Info(const Verbosity_t VerboseLevel,const char *fmt,...)
 */

#define TOOLSUB xrtph1sim           /* headas_main() requires that TOOLSUB be defined first */

/********************************/
/*        header files          */
/********************************/
#include "headas_main.c"
#include "xrtph1sim.h"

/********************************/
/*           defines            */
/********************************/
#define MJDREF    "1993-01-01T00:00:00"
#define TIME_GRAN   2.5

Global_t global;

struct tm date_time;

/***************************************************************************** *
 * Function: xrtph1sim
 * 
 * Description:	
 * 		  Routine to convert ascafile in Photon Counting Events 
 *                FITS file
 * 		  
 * 
 * Author/Date: Barbara Saija  September 2001
 * 
 * History:
 * 
 * Arguments: none
 * 
 * Functions Called:
 *          int ReadFilePar(void); - reads the input parameter from 
 *                                   xrtph1sim.par
 *          int WritePrimaryHeader(fitsfile *); - generates the primary header
 *          int WriteEventBinTable(fitsfile *, char *); - generates the 
 *                                                        events extension
 *          int WriteGTIBinTable(fitsfile *, char *); - generates the GTI
 *                                                      extension
 *          int AddTimeKeyword(fitsfile *);
 *          void AddTimeStopKey(fitsfile *, double);
 *          void xrtph1sim(void); - main
 * 
 * Library routines called:
 *     libcfitsio libcftools:

 *     libswxrt:
 *          double ModJulday2(const struct tm Date, const double FracSecs);
 *          double Julday(const double day, const unsigned month, const unsigned year);
 *          double ModJulday(const double day, const unsigned month, const unsigned year);
 *          void ModJulday2Inv(double , struct tm *,double *);

 *         
 *
 * Return Status:
 *         OK     : 0
 *         NOT_OK : 1
 *
 ******************************************************************************/
int xrtph1sim()
{
  FitsFileUnit_t outunit=NULL;

  /********************** 
   * set HEADAS globals *
   **********************/
  set_toolname(PRG_NAME);
  set_toolversion(VERSION);
  
  get_toolname(global.taskname);
  get_toolnamev(global.tasknamev);
  


  GetSWXRTDASVersion(global.swxrtdas_v);
  sprintf(global.creator, "%s (%s)", global.tasknamev, global.swxrtdas_v);

  if (ReadFilePar())
    {
      headas_chat(MUTE, "%s: Error reading input parameters.\n", global.tasknamev);
      goto end_func ;
    }

  /* Build Swift Xrt Primary header */
  if ((outunit=WritePrimaryHeader()) == (FitsFileUnit_t)0)
    {
      headas_chat(MUTE, "%s: Unable to write '%s' primary header\n",global.tasknamev, global.par.outfile);
      goto end_func;
    }

  if ((WriteEventBinTable(outunit)))
    {
      headas_chat(MUTE, "%s: Unable to write %s events extension\n", global.tasknamev, global.par.outfile);
      goto end_func;
    }

  if (WriteGTIBinTable(outunit))
    {
      headas_chat(MUTE, "%s: Unable to write %s GTI extension\n", global.tasknamev, global.par.outfile);
      goto end_func;
     }


  if (ChecksumCalc(outunit))
    {
      headas_chat(MUTE, "%s: Error writing checksum and datasum in '%s' file\n", global.tasknamev,global.par.outfile );
      goto end_func;
    }

  /* Close output file */
  if (CloseFitsFile(outunit))
    {
      headas_chat(MUTE, "%s: Unable to close '%s' file\n", global.tasknamev,global.par.outfile);
      goto end_func;
    }

  Info(NORMAL,"Info: File %s successfully written\n",
       global.par.outfile );
  Info(NORMAL, "Info: ----------------------------------------------------\n\n");
  return OK;
 
 end_func:
  if(outunit)
    CloseFitsFile(outunit); 
 
  Info(NORMAL, "Info: ...Exit with error\n");
  return NOT_OK;
}

/*
* 
* Function: disp_info
* 
* Description:	
* 		  Routine to read the input parameters from
* 		  xrtph1sim.par
* 		  
* 
* Author/Date: Barbara Saija September 2001
* 
* History:
* 
* Notes:	
* 
* Arguments: none
* 
* Functions Called:
* 
* Library routines called:
*     libcfitsio libcftools:
* 	  int headas_chat(int, const char* format, ...);
* 
*
*/
void disp_info(void)
{     
  /*******************************************
   *  Display info about input parameter     *
   *******************************************/

  headas_chat(NORMAL, "Info: ----------------------------------------------------\n");
  headas_chat(NORMAL, "Info: Running %s under headas\n",global.tasknamev);
  headas_chat(NORMAL, "Info: ----------------------------------------------------\n\n");
  headas_chat(NORMAL, "Info: Input Parameters:\n\n");
  headas_chat(NORMAL, "Info: Input File:            \t %s\n", global.par.infile);
  headas_chat(NORMAL, "Info: Output File:           \t %s\n", global.par.outfile);
  headas_chat(NORMAL, "Info: Observation Start Date:\t %s\n", global.par.dateobs);
  headas_chat(NORMAL, "Info: Observation Start Time:\t %s\n", global.par.timeobs);
  headas_chat(CHATTY, "Info: Clobber:               \t %d\n\n", headas_clobpar);
  headas_chat(NORMAL, "Info: ----------------------------------------------------\n\n");
}/* disp_info */

/*
 * 
 * Function: ReadFilePar
 * 
 * Description:	
 * 		  Routine to read the input parameters from
 * 		  xrtph1sim.par
 * 		  
 * 
 * Author/Date: Barbara Saija September 2001
 * 
 *  History:
 *  
 * Notes:	
 * 
 * Arguments: none
 * 
 * Functions Called:
 * 
 * Library routines called:
 *     libcfitsio libcftools:
 *        int PILGetFname(char *name, char* result);
 *        int PILGetString(char *name, char *result);
 * 	  int PILGetInt(char *name, int *result);
 * 	  int PILGetBool(char *name, int *result);
 * 	  int headas_chat(int, const char* format, ...);
 * 
 * Return Status:
 *        0: OK
 *        1: NOT_OK
 *
 */
int ReadFilePar(void)
{
  /************************/
  /* Gets a string param. */
  /************************/
  
  /* Get input events file name */  
  if(PILGetFname(PAR_INFILE, global.par.infile)) 
    {
      headas_chat(MUTE,"ReadFilePar: Error getting '%s'parameter.\n", PAR_INFILE);
      goto Error;
    }
  
  /* Get output swift xrt events file name */
  if(PILGetFname(PAR_OUTFILE, global.par.outfile)) 
    {
      headas_chat(MUTE,"ReadFilePar: Error getting '%s'parameter.\n", PAR_OUTFILE);
      goto Error;
    }
  
  /* Get observation start date */
  if(PILGetString(PAR_DATEOBS, global.par.dateobs)) 
    {
      headas_chat(MUTE,"ReadFilePar: Error getting '%s'parameter\n", PAR_DATEOBS);
      goto Error;
    }
  /* Get observation start time */  
  if(PILGetString(PAR_TIMEOBS, global.par.timeobs)) 
    {
      headas_chat(MUTE,"ReadFilePar: Error getting '%s'parameter\n", PAR_TIMEOBS);
      goto Error;
    }
  /* Display parameters */
  disp_info();
  return OK;

 Error:
  return NOT_OK;
}

/*
 * 
 * Function:  WritePrimaryHeader 
 * 
 * Description:	
 * 	          Routine to create a empty primary header
 * 		  
 * 
 * Author/Date: Barbara Saija September 2001
 * 
 * History:
 * 
 * Notes:	
 * 
 * Arguments: none
 * 
 * Functions Called:
 *        
 * Library routines called:
 *     libcfitsio libcftools:
 *        
 *      libswxrt:
 *        
 * 
 * Return 
 *        
 *
 */
FitsFileUnit_t WritePrimaryHeader(void)
{
  FitsHeader_t   newhead;
  FitsFileUnit_t outunit=NULL;
  
  TMEMSET0( &newhead, FitsHeader_t );

  /* Check if file exists */
  if(!FileExists(global.par.outfile)) 
    {
      /* Create new file */
      if ((outunit = OpenWriteFitsFile(global.par.outfile)) <= (FitsFileUnit_t )0)
	{
	  headas_chat(MUTE, "%s: Error creating '%s' fits file.\n", global.tasknamev, global.par.outfile);
	  goto write_end;
	}
    }
  else
    { 
      if(!headas_clobpar){
	headas_chat(MUTE, "%s: the '%s' file exists, please set the parameter clobber to overwrite it. \n", global.tasknamev, global.par.outfile);
	return OK;
      }
      headas_chat(MUTE, "%s: Warning: parameter 'clobber' set, the '%s' file will be overwritten\n", global.tasknamev, global.par.outfile);
      if(remove (global.par.outfile) == -1)
	{
	  headas_chat(NORMAL, "%s: Error removing '%s' file.\n",  global.tasknamev, global.par.outfile);
	  goto write_end;
	}
      outunit = OpenWriteFitsFile(global.par.outfile);
    }  
  headas_chat(MUTE, "%s: Info: Writing '%s' file.\n", global.tasknamev, global.par.outfile);
  
  newhead = NewPrimaryHeader(J, 0, VPNULL, VPNULL, TRUE);

  ADD_CARD_BLOCK_COMMON(&newhead);
  EndPrimaryHeader(&newhead);
  FinishPrimaryHeader(outunit, &newhead);

  
  return outunit;

 write_end:
  return (FitsFileUnit_t)0;
}/* WritePrimaryHeader */

/*
 * 
 * Function: WriteEventBinTable 
 * 
 * Description: Creates a events bintable extension 
 *      
 * 		  
 * 
 * Author/Date: Barbara Saija September, 2001
 * 
 * History:
 * 
 * 	
 * Notes:	
 * 	 
 * Arguments: none
 * 
 * Functions Called:
 *         int AddTimeKeyword(fitsfile *);
 *         void AddTimeStopKey(fitsfile *, double);
 * Library routines called:
 *     libcfitsio libcftools:
 *         int fits_movabs_hdu(fitsfile *fptr, int hdunum, > int * hdutype, 
 *                             int *status );
 *    libswxrt:
 *                void Addlongstr(fitsfile *, char * , char * , char * );
 *     
 * Return Status:
 *        0: OK
 *        1: NOT_OK
 *
 */
int WriteEventBinTable(FitsFileUnit_t outunit)
{
  
  int             ccdcount, nCols, status=OK;
  DTYPE           keydbl;
  JTYPE           keyint;
  double          inputtime, swifttime;
  EvCol_t         incol, outcol;
  long int        seed;
  unsigned        ncol, jj, OutRows, n, firsttime, FromRow, ReadRows;
  Bintable_t	  outtable, intable;
  FitsHeader_t	  outhead, inhead;
  FitsFileUnit_t  inunit=NULL;       /* pointer to the FITS file, defined in fitsio.h */
  
  TMEMSET0( &outtable, Bintable_t );
  TMEMSET0( &intable, Bintable_t );
  TMEMSET0( &outhead, FitsHeader_t );
  TMEMSET0( &inhead, FitsHeader_t );

  /* Open read only input event file */
  if ((inunit=OpenReadFitsFile(global.par.infile)) == (FitsFileUnit_t )0)
    {
      headas_chat(MUTE, "WriteEventBintable :Error opening '%s' file\n", global.par.infile);
      goto end_func;
    }
  else
    headas_chat(NORMAL, "%s: Info: Processing %s File\n", global.tasknamev, global.par.infile);
  
  /* Move to events extension */ 
  if (fits_movnam_hdu(inunit, ANY_HDU, KWVL_EXTNAME_EVT, 0, &status))
    { 
      headas_chat(MUTE,"WriteEventBintable: Unable to find  '%s' extension  in '%s' file.\n",KWVL_EXTNAME_EVT, global.par.infile);
      goto end_func;
    } 

  inhead=RetrieveFitsHeader(inunit);
  /* Create a new bintable header and get pointer to it */
  outhead = NewBintableHeader(0, &outtable);

  
  /* Define bintable structure */
  ncol=0;
  AddColumn(&outhead, &outtable, CLNM_TIME,CARD_COMM_TIME, "1D", TUNIT, UNIT_SEC, CARD_COMM_PHYSUNIT);
  outcol.TIME=ncol;
  AddColumn(&outhead, &outtable, CLNM_TDHXI, CARD_COMM_TDHXI, "1J", TNONE);
  outcol.TDHXI=++ncol;
  AddColumn(&outhead, &outtable, CLNM_CCDFrame, CARD_COMM_CCDFrame, "1J", TNONE);
  outcol.CCDFrame=++ncol;
  AddColumn(&outhead, &outtable, CLNM_X, CARD_COMM_X, "1I", TUNIT, UNIT_PIXEL, CARD_COMM_PHYSUNIT);
  outcol.X=++ncol;
  AddColumn(&outhead, &outtable, CLNM_Y, CARD_COMM_Y, "1I", TUNIT, UNIT_PIXEL, CARD_COMM_PHYSUNIT);
  outcol.Y=++ncol;
  AddColumn(&outhead, &outtable, CLNM_RAWX, CARD_COMM_RAWX, "1I", TUNIT|TMIN|TMAX, UNIT_PIXEL, CARD_COMM_PHYSUNIT, RAWX_MIN, RAWX_MAX);
  outcol.RAWX=++ncol;
  AddColumn(&outhead, &outtable, CLNM_RAWY, CARD_COMM_RAWY, "1I", TUNIT|TMIN|TMAX, UNIT_PIXEL, CARD_COMM_PHYSUNIT, RAWY_MIN, RAWY_MAX);
  outcol.RAWY=++ncol;
  AddColumn(&outhead, &outtable, CLNM_DETX, CARD_COMM_DETX, "1I", TUNIT|TMIN|TMAX, UNIT_PIXEL, CARD_COMM_PHYSUNIT, DETX_MIN, DETX_MAX);
  outcol.DETX=++ncol;
  AddColumn(&outhead, &outtable, CLNM_DETY, CARD_COMM_DETY, "1I", TUNIT|TMIN|TMAX, UNIT_PIXEL, CARD_COMM_PHYSUNIT, DETY_MIN, DETY_MAX);
  outcol.DETY=++ncol;
  AddColumn(&outhead, &outtable, CLNM_PHAS, CARD_COMM_PHAS, "9I", TUNIT, UNIT_CHANN, CARD_COMM_PHYSUNIT);
  outcol.PHAS=++ncol;

  /* Add extension name */
  AddCard(&outhead, KWNM_EXTNAME, S, KWVL_EXTNAME_EVT, CARD_COMM_EXTNAME);

 /* hduclass, hduclas1, tsortkey */ 
  ADD_CARD_BLOCK_HDU_E(&outhead);

  /* telescop instrume */
  ADD_CARD_BLOCK_COMMON(&outhead);

  if((global.object=GetSVal(&inhead, KWNM_OBJECT)))
    AddCard(&outhead, KWNM_OBJECT, S, global.object, CARD_COMM_OBJECT);

  AddCard(&outhead, KWNM_OBS_MODE, S, KWVL_OBS_MODE_PG, CARD_COMM_OBS_MODE_PG);
  /*keyint=-2147483648;
  AddCard(&outhead, "TZERO3", J, &keyint, CARD_COMM_OBS_MODE_PG);
  */
  AddCard(&outhead, KWNM_OBS_ID, S, KWVL_OBS_ID, CARD_COMM_OBS_ID);    
  AddCard(&outhead, KWNM_DATAMODE, S, KWVL_DATAMODE_PH, CARD_COMM_DATAMODE_PH);
  AddCard(&outhead, KWNM_RADECSYS, S, KWVL_RADECSYS, CARD_COMM_RADECSYS);
  AddCard(&outhead, KWNM_ORIGIN, S, KWVL_ORIGIN, CARD_COMM_ORIGIN);

  keydbl=KWVL_EQUINOX;
  AddCard(&outhead, KWNM_EQUINOX, D, &keydbl, CARD_COMM_EQUINOX);

  /* Copy keywords from input to output file */
  if((keydbl=GetDVal(&inhead, KWNM_RA_PNT)))
    AddCard(&outhead, KWNM_RA_PNT, D, &keydbl,CARD_COMM_RA_PNT);

  if((keydbl=GetDVal(&inhead, KWNM_DEC_PNT)))
    AddCard(&outhead, KWNM_DEC_PNT, D, &keydbl,CARD_COMM_DEC_PNT);

  if((keydbl=GetDVal(&inhead, KWNM_RA_NOM)))
    AddCard(&outhead, KWNM_RA_NOM, D, &keydbl,CARD_COMM_RA_NOM);
  
  if((keydbl=GetDVal(&inhead, KWNM_DEC_NOM)))
    AddCard(&outhead, KWNM_DEC_NOM, D, &keydbl,CARD_COMM_DEC_NOM);
  
  keydbl=KWVL_RA_SCX;
  AddCard(&outhead,  KWNM_RA_SCX, D, &keydbl, CARD_COMM_RA_SCX);    

  keydbl=KWVL_DEC_SCX;
  AddCard(&outhead, KWNM_DEC_SCX, D, &keydbl, CARD_COMM_DEC_SCX);

  keydbl=KWVL_RA_SCY;
  AddCard(&outhead, KWNM_RA_SCY, D, &keydbl, CARD_COMM_RA_SCY);

  keydbl=KWVL_DEC_SCY;
  AddCard(&outhead, KWNM_DEC_SCY, D,&keydbl , CARD_COMM_DEC_SCY);
  
  keydbl=KWVL_RA_SCZ;
  AddCard(&outhead, KWNM_RA_SCZ, D, &keydbl , CARD_COMM_RA_SCZ);

  keydbl=KWVL_DEC_SCZ;
  AddCard(&outhead, KWNM_DEC_SCZ, D,&keydbl , CARD_COMM_DEC_SCZ);

  AddCard(&outhead, KWNM_TCTYP4, S, KWVL_TCTYP4, CARD_COMM_TCTYP4);

  keyint=KWVL_TCRPX4;
  AddCard(&outhead,  KWNM_TCRPX4, J, &keyint, CARD_COMM_TCRPX4);

  keyint=KWVL_TCRVL4;
  AddCard(&outhead,  KWNM_TCRVL4, J, &keyint, CARD_COMM_TCRVL4);

  keydbl=KWVL_TCDLT4;
  AddCard(&outhead,  KWNM_TCDLT4, D, &keydbl, CARD_COMM_TCDLT4);

  AddCard(&outhead,  KWNM_TCUNI4, S, KWVL_TCUNI4, CARD_COMM_TCUNI4);
  AddCard(&outhead,  KWNM_TCTYP5, S, KWVL_TCTYP5, CARD_COMM_TCTYP5);

  keyint=KWVL_TCRPX5;
  AddCard(&outhead,  KWNM_TCRPX5, J,  &keyint, CARD_COMM_TCRPX5);

  keyint=KWVL_TCRVL5;
  AddCard(&outhead,  KWNM_TCRVL5, J, &keyint, CARD_COMM_TCRVL5);

  keydbl=KWVL_TCDLT5;
  AddCard(&outhead,  KWNM_TCDLT5, D, &keydbl, CARD_COMM_TCDLT5);   
  
  AddCard(&outhead,  KWNM_TCUNI5, S, KWVL_TCUNI5, CARD_COMM_TCUNI5);

  keydbl=KWVL_TCROT5;
  AddCard(&outhead, KWNM_TCROT5, D, &keydbl, CARD_COMM_TCROT5);

  keydbl=KWVL_TPC0404;
  AddCard(&outhead,  KWNM_TPC0404, D,  &keydbl, CARD_COMM_TPC0404);

  keydbl=KWVL_TPC0505;
  AddCard(&outhead,  KWNM_TPC0505, D, &keydbl, CARD_COMM_TPC0505);

  keydbl=KWVL_TPC0405;
  AddCard(&outhead,  KWNM_TPC0405, D, &keydbl, CARD_COMM_TPC0405);

  keydbl=KWVL_TPC0504;
  AddCard(&outhead, KWNM_TPC0504, D, &keydbl, CARD_COMM_TPC0504);

  if(AddTimeKeyword(&outhead))
    headas_chat(NORMAL, " WriteEventBinTable: Unable to add time keywords.\n");



     
  EndBintableHeader(&outhead, &outtable);

  /* Finish bintable header */
  FinishBintableHeader(outunit, &outhead, &outtable);  
  
  GetBintableStructure(&outhead, &outtable, BINTAB_ROWS, 0, NULL);
  GetBintableStructure(&inhead, &intable, BINTAB_ROWS, 0, NULL);
  
 /* Get input cols index from name */
  if ((incol.TIME=ColNameMatch(CLNM_TIME, &intable)) == -1)
    {
      headas_chat(MUTE,"WriteEventBintable: Error '%s' column does not exists in '%s' file.\n",CLNM_TIME, global.par.infile);
      goto end_func;
    }
  /*16092002
  if ((incol.RAWX=ColNameMatch(CLNM_RAWX, &intable)) == -1)
    {
      headas_chat(MUTE,"WriteEventBintable: Error '%s' column does not exists in '%s' file.\n",CLNM_RAWX, global.par.infile);
      goto end_func;
    }
  
  if(( incol.RAWY=ColNameMatch(CLNM_RAWY, &intable))==-1)
    {
      headas_chat(MUTE,"WriteEventBintable: Error '%s' column does not exists in '%s' file.\n", CLNM_RAWY, global.par.infile);
      goto end_func;
    }
   
  */
 if ((incol.DETX=ColNameMatch(CLNM_DETX, &intable)) == -1)
    {
      headas_chat(MUTE,"WriteEventBintable: Error '%s' column does not exists in '%s' file.\n",CLNM_DETX, global.par.infile);
      goto end_func;
    }
  
  if(( incol.DETY=ColNameMatch(CLNM_DETY, &intable))==-1)
    {
      headas_chat(MUTE,"WriteEventBintable: Error '%s' column does not exists in '%s' file.\n", CLNM_DETY, global.par.infile);
      goto end_func;
    }
   
  if ((incol.PHAS=ColNameMatch(CLNM_PHAS, &intable))==-1)
    {
      headas_chat(MUTE,"WriteEventBintable: Error '%s' does not exists in '%s' file.\n", CLNM_PHAS, global.par.infile);
      goto end_func;
    }

  /* initialize random number generator */
  get_ran2seed(&seed);

  /* Read bintable */
  FromRow = 1;
  ReadRows=intable.nBlockRows;
  OutRows=0;
  nCols=intable.nColumns;
  swifttime=global.tstart;
  firsttime=1;
  ccdcount=1;

  while(ReadBintable(inunit, &intable, nCols, NULL, FromRow, &ReadRows)==0)   
    {
      for(n=0; n<ReadRows; ++n)
	{
	  if(firsttime)
	    {
	      inputtime=DVEC(intable, n, incol.TIME);
	      firsttime=0;
	    }

	  if (DVEC(intable, n, incol.TIME) != inputtime)
	    {
	      inputtime=DVEC(intable, n, incol.TIME);
	      swifttime+=TIME_GRAN;
	      ccdcount++;
	    }

	  DVEC(outtable, n, outcol.TIME)=swifttime;
	  JVEC(outtable, n, outcol.TDHXI) = 0;
	  JVEC(outtable, n, outcol.CCDFrame) = ccdcount;
	  IVEC(outtable, n, outcol.X) = 0;
	  IVEC(outtable, n, outcol.Y) = 0;
	  
	  
	  /*IVEC(outtable, n, outcol.RAWX)=(int)IVEC(intable, n, incol.RAWY);
	  if (IVEC(outtable, n, outcol.RAWX) == 0)
	    IVEC(outtable, n, outcol.RAWX)=1;
	  if (IVEC(outtable, n, outcol.RAWX) > 600)
	    IVEC(outtable, n, outcol.RAWX)=600;
	    
	  IVEC(outtable, n, outcol.RAWY)=(int)(CCD_ROWS - (IVEC(intable, n, incol.RAWX)));
	  if (IVEC(outtable, n, outcol.RAWY) == 0)
	    IVEC(outtable, n, outcol.RAWY)=1;
	  if (IVEC(outtable, n, outcol.RAWY) > 602)
	    IVEC(outtable, n, outcol.RAWY)=602;
	  */
	  /*16092002
	  IVEC(outtable, n, outcol.RAWX)=(int)(((IVEC(intable, n, incol.RAWX)*607.)/426.+xrt_ran2(&seed) - 7.5));
	  if (IVEC(outtable, n, outcol.RAWX) == 0)
	    IVEC(outtable, n, outcol.RAWX)=1;
	  if (IVEC(outtable, n, outcol.RAWX) > 600)
	    IVEC(outtable, n, outcol.RAWX)=600;
	  
	  IVEC(outtable, n, outcol.RAWY)=(int)(((IVEC(intable, n, incol.RAWY)*602.)/422.+xrt_ran2(&seed) - 0.5));
	  if (IVEC(outtable, n, outcol.RAWY) == 0)
	    IVEC(outtable, n, outcol.RAWY)=1;
	  if (IVEC(outtable, n, outcol.RAWY) > 602)
	    IVEC(outtable, n, outcol.RAWY)=602;
	  
	  IVEC(outtable, n, outcol.DETX)=0;
	  IVEC(outtable, n, outcol.DETY)=0;
	  */
	  /*	  IVEC(outtable, n, outcol.DETX)=(int)((IVEC(outtable, n, outcol.RAWX)*2.+xrt_ran2(&seed)-0.5));
		  IVEC(outtable, n, outcol.DETY)=(int)((IVEC(outtable, n, outcol.RAWY)*2.+xrt_ran2(&seed)-0.5));*/
	  
	  if (IVEC(intable, n, incol.DETX)<340)
	    IVEC(outtable, n, outcol.RAWX)=0;
	  else if((IVEC(intable, n, incol.DETX)>940))
	    IVEC(outtable, n, outcol.RAWX)=600;
	  else 
	    IVEC(outtable, n, outcol.RAWX)=IVEC(intable, n, incol.DETX)-340;

	  if (IVEC(intable, n, incol.DETY)<340)
	    IVEC(outtable, n, outcol.RAWY)=0;
	  else if((IVEC(intable, n, incol.DETY)>940))
	    IVEC(outtable, n, outcol.RAWY)=600;
	  else 
	    IVEC(outtable, n, outcol.RAWY)=940 - IVEC(intable, n, incol.DETY);

	  
	  
	  
	  for (jj=0; jj< PHAS_MOL; jj++)
	    IVECVEC(outtable,n,outcol.PHAS,jj)=IVECVEC(intable,n,incol.PHAS,jj);
	  
	  if(++OutRows>=BINTAB_ROWS)
	    {
	      WriteFastBintable(outunit, &outtable, OutRows, FALSE);
	      OutRows=0;
	    }
	}/* for(n...) */
      FromRow += ReadRows;
      ReadRows = BINTAB_ROWS;

    }/* while(ReadBintable...) */

  WriteFastBintable(outunit, &outtable, OutRows, TRUE);


  global.tstop=swifttime;
  if(CalDateEnd(&outhead))
    headas_chat(CHATTY, "WriteEventBinTable: Unable to add date and time end.\n");

  ADD_CARD_BLOCK_TIME2_E(&outhead); 
  AddCard(&outhead, KWNM_CREATOR, S, global.creator, CARD_COMM_CREATOR);
  AddCard(&outhead, KWNM_LONGSTRN, S,  KWVL_LONGSTRN, CARD_COMM_LONGSTRN);
  
 
  
  if(WriteUpdatedHeader(outunit, &outhead))
    { 
      headas_chat(MUTE,"WriteEventBinTable: Unable to update header.\n");      
      goto end_func;
    }

  TMEMSET0( &outtable, Bintable_t );
  TMEMSET0( &intable, Bintable_t );
 
/* Close bad pixels file */
  if (CloseFitsFile(inunit))
    {
      headas_chat(MUTE, "WriteEventBintable: Unable to close '%s' event fits file\n",global.par.infile);
      goto end_func;
    }
  
  return OK;

 end_func:
  return NOT_OK;

}

/*****************************************************************************
* 
* Function: WriteGTIBinTable 
* 
* Description: Creates a GTI bintable extension 
*      
* 		  
* 
* Author/Date: Barbara Saija September, 2001
* 
* History:
* 
* 	
* Notes:	
* 	
* Arguments: none
* 
* Functions Called:
*         int AddTimeKeyword(fitsfile *);
*         void AddTimeStopKey(fitsfile *, double);
* Library routines called:
*     libcfitsio libcftools:

*    libswxrt:
*                void Addlongstr(fitsfile *, char * , char * , char * );
*     
* Return Status:
*        0: OK
*        1: NOT_OK
*
*****************************************************************************/
int WriteGTIBinTable(FitsFileUnit_t outunit)
{
  
  DTYPE           keydbl;
  unsigned        ncol;
  GTICol_t        outcol;
  Bintable_t	  outtable;  
  FitsHeader_t	  outhead, inhead;

  TMEMSET0( &outtable, Bintable_t );
  TMEMSET0( &outhead, FitsHeader_t );
  TMEMSET0( &inhead, FitsHeader_t );

  /* Create a new bintable header and get pointer to it */
  outhead = NewBintableHeader(0, &outtable);

  /* Add extension name */
  AddCard(&outhead, KWNM_EXTNAME, S, KWVL_EXTNAME_GTI, CARD_COMM_EXTNAME);
  
  /* Define bintable structure */
  ncol=0;
  AddColumn(&outhead, &outtable, CLNM_START,CARD_COMM_TSTART, "1D", TUNIT, UNIT_SEC, CARD_COMM_PHYSUNIT);
  outcol.TSTART=ncol;
  AddColumn(&outhead, &outtable, CLNM_STOP,CARD_COMM_TSTOP, "1D", TUNIT, UNIT_SEC, CARD_COMM_PHYSUNIT);
  outcol.TSTOP=++ncol;

  /* write some optional keywords to the header */
  /* hduclass hduclas1 hduclas2 */
  ADD_CARD_BLOCK_HDU_GTI(&outhead);
  /* telescop instrume */
  ADD_CARD_BLOCK_COMMON(&outhead);
  
  AddCard(&outhead, KWNM_OBJECT, S, global.object, CARD_COMM_OBJECT);
  AddCard(&outhead, KWNM_DATAMODE, S, KWVL_DATAMODE_PH, CARD_COMM_DATAMODE_PH);
    
  keydbl=KWVL_EQUINOX;
  AddCard(&outhead, KWNM_EQUINOX, D, &keydbl, CARD_COMM_EQUINOX);  

  
  /* mtype1, mform1, metype1 */
  ADD_CARD_BLOCK_GTI(&outhead);
  
  AddCard(&outhead, KWNM_LONGSTRN, S,  KWVL_LONGSTRN, CARD_COMM_LONGSTRN);

  if(AddTimeKeyword(&outhead))
    headas_chat(NORMAL,"WriteGTIBintable: Unable to write time information in file.\n");

  if(CalDateEnd(&outhead))
    headas_chat(CHATTY, "WriteEventBinTable: Unable to add date and time end.\n");

  AddCard(&outhead, KWNM_ORIGIN, S, KWVL_ORIGIN, CARD_COMM_ORIGIN);
  AddCard(&outhead, KWNM_CREATOR, S, global.creator, CARD_COMM_CREATOR);
  
  
  EndBintableHeader(&outhead, &outtable);
  
  /* Finish bintable header */
  FinishBintableHeader(outunit, &outhead, &outtable);  

  GetBintableStructure(&outhead, &outtable, BINTAB_ROWS, 0, NULL);
  
  DVEC(outtable, 0, outcol.TSTART)=global.tstart;
  DVEC(outtable, 0, outcol.TSTOP)=global.tstop;
  WriteFastBintable(outunit, &outtable, 1, TRUE);

  TMEMSET0( &outtable, Bintable_t );

  return OK;
}

/*****************************************************************************
* 
* Function: AddTimeKeyword   AddTimeStopKey
* 
* Description:	
* 	          Routine to modify and add time keyword
* 		  
* 
* Author/Date: Barbara Saija September 2001
* 
* History:
* 
* Notes:	
* 
* Arguments: none
* 
* Functions Called:
*       
*        ModJulday2(const struct tm Date, const double FracSecs);            
*        printerror(int, char);
* Library routines called:
*     libcfitsio libcftools:
*        int fits_movabs_hdu(fitsfile *fptr, int hdunum, int *hdutype, 
*                        int *status )
*        int fits_get_colnum(fitsfile *fptr, int casesen, char *templt, 
*                        int *colnum,int *status)
*        int fits_read_col_dbl(fitsfile *fptr, int colnum, long firstrow, 
*                          long firstelem, long nelements, DTYPE nulval, 
*                          DTYPE *array, int * anynull, int *status)
*        int fits_get_num_rows(fitsfile *fptr, long *firstrow, int *status)
*        int fits_str2time(char *datestr, int *year, int *month, int *day,
*                      int *hour, int *minute, double *second, int *status);
*    libswxrt:
*        void AddKeyword(fitsfile *fptr, char *keyname, char *value, 
*                   char * comment); 
*        void AddKeyworddouble(fitsfile *fptr, char *keyname, double *value, 
*                   char * comment);
*        
* 
* Return Status:
*     0:OK
*     1:NOT_OK
*
***************************************************************************/

int AddTimeKeyword(FitsHeader_t * head)
{
  int    year, month, day, hour, minute, status=OK;
  double mjddateobs, FracSec=0, second;


  global.timesys=KWVL_TIMESYS;
  AddCard(head,  KWNM_TIMEREF, S,  KWVL_TIMEREF, CARD_COMM_TIMEREF);
  AddCard(head, KWNM_TIMESYS, D,  &global.timesys, CARD_COMM_TIMESYS);
  AddCard(head,  KWNM_TIMEUNIT, S,  KWVL_TIMEUNIT, CARD_COMM_TIMEUNIT);
 
  if (fits_str2time(MJDREF, &year, &month, &day, &hour, &minute, &second, &status))
    {
      headas_chat(CHATTY, "AddTimeKeyword: Unable to convert string to time.\n");
      goto end_func;
    }

  date_time.tm_year=(year-1900);
  date_time.tm_mon=(month-1);
  date_time.tm_mday=day;
  date_time.tm_hour=hour;
  date_time.tm_min=minute;
  date_time.tm_sec=second;

  global.mjdtimesys=ModJulday2(date_time, FracSec);

  AddCard(head, KWNM_MJDREF, D, &global.mjdtimesys, CARD_COMM_MJDREF);

  if (fits_str2time(global.par.dateobs, &year, &month, &day, &hour, &minute, &second, &status))
    {
      headas_chat(CHATTY,"Unable to convert string to date.\n");
      goto end_func;
    }
  
  date_time.tm_year=(year-1900);
  date_time.tm_mon=(month-1);
  date_time.tm_mday=day;
 
  if (fits_str2time(global.par.timeobs, &year, &month, &day, &hour, &minute, &second, &status))
    {
      headas_chat(CHATTY, "Unable to convert string to time.\n");
      goto end_func;
    }
  
  date_time.tm_hour=hour;
  date_time.tm_min=minute;
  date_time.tm_sec=second;
  
  mjddateobs=ModJulday2(date_time, FracSec);
  global.tstart=((mjddateobs*86400)-(global.mjdtimesys*86400));
  AddCard(head, KWNM_TSTART, D, &global.tstart, CARD_COMM_TSTART);

  AddCard(head, KWNM_MJD_OBS, D, &mjddateobs, CARD_COMM_MJDOBS);
  AddCard(head, KWNM_DATEOBS, S, global.par.dateobs, CARD_COMM_DATEOBS);
  AddCard(head,  KWNM_TIMEOBS, S,  global.par.timeobs, CARD_COMM_TIMEOBS);

  return OK;
 
end_func: 
  headas_chat(NORMAL, "Error in date format.\n");
  return NOT_OK;
}/* AddTimeKeyword */

int CalDateEnd(FitsHeader_t *head)
{
  int    nyear, nmon;
  char   date[11];
  DTYPE  keydbl;
  double FracSecs=0; 


  global.mjdtimesys=(global.mjdtimesys + global.tstop/86400.);

  ModJulday2Inv(global.mjdtimesys, &date_time, &FracSecs);

  nyear=1900+date_time.tm_year;
  nmon=date_time.tm_mon+1;

  sprintf(date,"%u-%02u-%02u",nyear,nmon,date_time.tm_mday);
  AddCard(head, KWNM_DATEEND, S, date, CARD_COMM_DATEEND);

  sprintf(date,"%02u:%02u:%02u",date_time.tm_hour,date_time.tm_min,date_time.tm_sec);
  AddCard(head, KWNM_TIMEEND, S, date, CARD_COMM_TIMEEND);

  AddCard(head, KWNM_TSTOP, D, &global.tstop, CARD_COMM_TSTOP);

  keydbl=(global.tstop - global.tstart);
  AddCard(head, KWNM_TELAPSE, D, &keydbl, CARD_COMM_TELAPSE);
  AddCard(head, KWNM_LIVETIME, D, &keydbl, CARD_COMM_LIVETIME);
  AddCard(head, KWNM_EXPOSURE, D, &keydbl, CARD_COMM_EXPOSURE);
  AddCard(head, KWNM_ONTIME, D, &keydbl, CARD_COMM_ONTIME);

  /* Add creation date */
  GetGMTDateTime(global.date);
  AddCard(head, KWNM_DATE, S, global.date, CARD_COMM_DATE);



  return OK;
}
