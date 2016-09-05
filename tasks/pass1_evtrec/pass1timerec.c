/*
 * 
 *	pass1timerec.c:
 *
 *	INVOCATION:
 *
 *		pass1timerec [parameter=value ...]
 *
 *	DESCRIPTION:
 *   
 *	DOCUMENTATION:
 *
 *	CHANGE HISTORY:
 *        0.1.0 - BS 21/03/2003 - First version
 *        0.2.0 - BS 16/06/2003 - Added routine to handle photodiode mode
 *                              - Deleted routine to compute 3x3 event rec for Windowed Timing 
 *        0.2.1 - FT 26/06/2003 - Changed condition on local maximum. The PHA of the pixel
 *                                must be >= of adiacent pizels (not only >). Burrows
 *                                request (23/06/2003)
 *        0.2.2 - BS 09/09/2003 - Added flag and routine to delete columns
 *                              - Added flag and routine to delete rows where PHA value is NULL 
 *                              - Added routine to insert statistic keywords
 *                              - Get CALDB grades file to assign grades to events
 *       0.2.3  - BS 24/06/2004 - Bug fixed on "fits_update_key" call                 
 * 
 *
 *      NOTE:
 *       
 *
 *	AUTHORS:
 *
 *       ISAC Italian Swift Archive Center (FRASCATI)
 */
#define TOOLSUB pass1timerec  /* headas_main() requires that TOOLSUB be defined first */

/********************************/
/*        header files          */
/********************************/
#include "headas_main.c"
#include "pass1timerec.h" 


/********************************/
/*         definitions          */
/********************************/

#define PASS1TIMEREC_C
#define PASS1TIMEREC_VERSION       "0.2.2"
#define PRG_NAME               "pass1timerec"

/********************************/
/*           globals            */
/********************************/

Global_t global;

/*
 *	pass1timerec_getpar
 *
 *
 *	DESCRIPTION:
 *                 Routine to read input parameters from 
 *                 pass1timerec.par    
 *     
 *
 *	DOCUMENTATION:
 *        
 *
 *      FUNCTION CALL:
 *           int PILGetFname(char *name, char *result); 
 * 	     int PILGetInt(char *name, int *result);  
 *           int headas_chat(int , char *, ...);
 *           void WriteInfoParams(void);
 *           void get_history(int *);
 *	     void ReadInFileNamesFile(char *);
 *           char * strcpy(char *, char *);
 *
 *      CHANGE HISTORY:
 *       
 *        0.1.0: - BS 21/03/2003 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 */
int pass1timerec_getpar()
{
  /*
   *  Get File name Parameters
   */
  
  /* Input Timing Event List Files Name */
  if(PILGetFname(PAR_INFILE, global.par.infiles)) 
    {
      headas_chat(NORMAL, "pass1timerec_getpar: Error getting '%s' parameter.\n",PAR_INFILE);
      goto Error;
    }
  

  /* Output Event List File Name */
  if(PILGetFname(PAR_OUTFILE, global.par.outfile)) 
    {
      headas_chat(NORMAL, "pass1timerec_getpar: Error getting '%s' parameter.\n",PAR_OUTFILE);
      goto Error;	
    }
 
  /* Input CALDB grade file */
  if(PILGetFname(PAR_GRADEFILE, global.par.gradefile)) 
    {
      headas_chat(NORMAL, "pass1_evtrec_getpar: Error getting '%s' parameter.\n",PAR_GRADEFILE);
      goto Error;	
    }
  
  /* Check guard pixels ? */
  if(PILGetBool(PAR_GUARDFLAG, &global.par.guardflag))
    {
      headas_chat(NORMAL,"pass1_evtrec_getpar: Error getting '%s'parameter.\n", PAR_GUARDFLAG);
      goto Error;
    }

  if(global.par.guardflag)
    {
      /* Outer threshold Level */
      if(PILGetInt(PAR_OUTER, &global.par.outer)) 
	{
	  headas_chat(NORMAL, "pass1_evtrec_getpar: Error getting '%s' parameter.\n",PAR_OUTER);
	  goto Error;	
	}
    }

  /* Added columns PHAS and PixsAbove? */ 
  if(PILGetBool(PAR_ADDCOL, &global.par.addcol))
    {
      headas_chat(NORMAL,"pass1_evtrec_getpar: Error getting '%s'parameter.\n", PAR_ADDCOL);
      goto Error;
     }

  /* Delete NULL events? */ 
  if(PILGetBool(PAR_DELNULL, &global.par.delnull))
    {
      headas_chat(NORMAL,"pass1_evtrec_getpar: Error getting '%s'parameter.\n", PAR_DELNULL);
      goto Error;
    }

  /* Split Threshold Level */
  if(PILGetInt(PAR_SPLIT, &global.par.split)) 
    {
      headas_chat(NORMAL, "pass1timerec_getpar: Error getting '%s' parameter.\n",PAR_SPLIT);
      goto Error;	
    }

  /* Event threshold Level */
  if(PILGetInt(PAR_EVENT, &global.par.event)) 
    {
      headas_chat(NORMAL, "pass1_evtrec_getpar: Error getting '%s' parameter.\n",PAR_EVENT);
      goto Error;	
    }


  get_history(&global.hist);
  WriteInfoParams();

  /* Check if input file is an ASCII fits files list */
  if (global.par.infiles[0] == '@')
    {
      ReadInFileNamesFile(global.par.infiles);
      strcpy(global.par.outfile, DF_NONE);
      global.list=TRUE;
    }
  
  else
    global.list=FALSE;
  return OK;

 Error:
  return NOT_OK;
  
} /* pass1timerec_getpar OK */

/*
 *	pass1timerec_work
 *
 *
 *	DESCRIPTION:
 *     
 *
 *	DOCUMENTATION:
 *        
 *
 *      FUNCTION CALL:
 *           char * strtok(char *, char *);
 *           FitsFileUnit_t OpenReadFitsFile(char *name);
 *           int headas_chat(int , char *, ...);
 *           int fits_movnam_hdu(fitsfile *fptr,int hdutype, char *extname, int extver, int *status);
 *           int CloseFitsFile(FitsFileUnit_t file);
 *           BOOL KeyWordMatch( FitsFileUnit_t unit, const char *KeyWord, ValueTag_t vtag, 
 *                              const void *NominalValue, void *ActValue);
 *           FitsHeader_t RetrieveFitsHeader(FitsFileUnit_t unit);
 *           SPTYPE GetSVal(const FitsHeader_t *header, const char *KeyWord);
 *           char * strcpy(char *, char *);
 *           int fits_get_hdu_num(fitsfile *fptr, int *hdunum);
 *           int strcasecmp(const char *s1, const char *s2);
 *           char * SplitFilePath(const char *FilePath, char *DirName, char *BaseName);
 *           char * StripExtension(char * filename);
 *           char *strcat(char *dest, const char *src);
 *           BOOL FileExists(const char *FileName);
 *           FitsFileUnit_t OpenWriteFitsFile(char *name);
 *           int remove(const char *pathname);
 *           int fits_get_num_hdus(fitsfile *fptr, int *hdunum, int *status);
 *           int fits_movabs_hdu(fitsfile *fptr, int hdunum, > int * hdutype,
 *                               int *status ); 
 *           int fits_copy_hdu(fitsfile *infptr, fitsfile *outfptr, int morekeys, int *status); 
 *           int fits_update_key(fitsfile *fptr, int datatype, char *keyname, DTYPE *value,
 *                               char *comment, int *status);  
 *           int AddEventBintable(FitsFileUnit_t in, FitsFileUnit_t out);
 *           int headas_parstamp(fitsfile *fptr, int hdunum)
 *           int ChecksumCalc(FitsFileUnit_t unit);
 * 
 *	
 *      CHANGE HISTORY:
 *
 *        0.1.0: - BS 21/03/2003 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 *
 *      Return Status: 
 *        0: OK
 *        1: NOT_OK
 */
int pass1timerec_work()
{
  int            i, status = OK, inExt, outExt, evExt, overwrite=0;   
  char           stem[10] , ext[MAXEXT_LEN] ;
  pid_t          tmp;
  FitsHeader_t   head;
  FitsFileUnit_t inunit=NULL, outunit=NULL;              /* Input and Output fits file pointer */ 

  /* Get input file name */
  global.infile = strtok(global.par.infiles, " \n\t;:");
  global.first=1;

  while (global.infile != CPNULL) {
    
    /* Initialize counters */
    for(i = 0; i < 5; i++)
      {
	global.grade[i]=0;
	global.per_grade[i]=0;
      }

    global.tot_sat=0;

    global.wt=0;
    global.pd=0;


    /* Open readonly input event file */
    if ((inunit=OpenReadFitsFile(global.infile)) <= (FitsFileUnit_t )0)
      {
	headas_chat(NORMAL,"pass1timerec_work: ... Error opening '%s' FITS file.\n", global.infile);
	goto Error;
      }
    
    /* Check readout mode */
    
    /* Move in events extension in input file */
    if (fits_movnam_hdu(inunit, ANY_HDU,KWVL_EXTNAME_EVT, 0, &status))
      { 
	headas_chat(CHATTY,"pass1timerec_work: Error: unable to find  '%s' extension  in '%s' file.\n",KWVL_EXTNAME_EVT, global.infile);      
	if( CloseFitsFile(inunit))
	  {
	    headas_chat(CHATTY, "%s: Error:  unable to close  '%s' file. \n ", global.taskname, global.infile);
	    goto Error;
	  } 
	if(global.list)
	  goto skip_file;
	else
	  {
	    goto Error;
	  }
      }
    
    /* Check whether it is a Swift XRT File */
    ISXRTFILE_WARNING(NORMAL,inunit,global.infile,global.taskname);

    if(KeyWordMatch(inunit, KWNM_DATAMODE, S, KWVL_DATAMODE_PDPU, NULL) || KeyWordMatch(inunit, KWNM_DATAMODE, S, KWVL_DATAMODE_PD, NULL))
      global.pd=1;

    else if(KeyWordMatch(inunit, KWNM_DATAMODE, S, KWVL_DATAMODE_TM, NULL))
      global.wt=1;
    else 
      {
	headas_chat(NORMAL, "pass1_evtrec_work: Warning readout mode in %s file not allowed.\n", global.infile);
	if( CloseFitsFile(inunit))
	  {
	    headas_chat(CHATTY, "%s: Error:  unable to close  '%s' file. \n ", global.taskname, global.infile);
	    goto Error;
	  } 
	if(global.list)
	  goto skip_file;
	else
	  {
	    goto Error;
	  }
      }
    

    head=RetrieveFitsHeader(inunit);    
    
    /* Get Event ext number */
    if (!fits_get_hdu_num(inunit, &evExt))
      {
	headas_chat(CHATTY,"pass1timerec_work: Error unable to find  '%s' extension  in '%s' file.\n",KWVL_EXTNAME_EVT, global.infile);      
	goto Error;
      } 
    
    /* Check if outfile == NONE */    
  if (!(strcasecmp (global.par.outfile, DF_NONE)))
      {
	GetFilenameExtension(global.infile, ext);
	if (!(strcmp(ext, "gz")) || !(strcmp(ext, "Z")))
	  {
	    headas_chat(NORMAL, "%s: Error input file is compressed, cannot update it.\n", global.taskname);
	    if( CloseFitsFile(inunit))
	      {
		headas_chat(CHATTY, "%s: Error:  unable to close  '%s' file. \n ", global.taskname, global.infile);
		goto Error;
	      }
	    if(global.list)
	      goto skip_file;
	    else
	      {
		goto Error;
	      }
	  }
	else
	  /* Overwrite input file */
	  overwrite=1;
	 
      }
    else
      {
	if(FileExists(global.par.outfile))
	  {
	    if(!headas_clobpar)
	      {
		headas_chat(NORMAL, "%s: '%s' file exists, \n\t\t please set the parameter clobber to yes to overwrite it.\n", global.taskname, global.par.outfile);
		goto Error;
	      }
	    else
	      {
		headas_chat(NORMAL, "%s: Warning: parameter 'clobber' set,\n\t\t the '%s' file will be overwritten\n", global.taskname, global.par.outfile);
		if(remove (global.par.outfile) == -1)
		  {
		    headas_chat(NORMAL, "%s: Error removing '%s' file.\n", global.taskname, global.par.outfile);
		    goto Error;
		  }  
	      }
	  }
      }
    /* Derive temporary event filename */
    tmp=getpid(); 
    sprintf(stem, "%dtmp", (int)tmp);
    if (overwrite)
      {
	DeriveFileName(global.infile, global.tmpevtfilename, stem);
	strcpy(global.par.outfile, global.infile);
      }
    else
      DeriveFileName(global.par.outfile, global.tmpevtfilename, stem);

    /* Check if file exists to remove it*/
    if(FileExists(global.tmpevtfilename))
      if(remove (global.tmpevtfilename) == -1)
	{
	  headas_chat(NORMAL, "%s: Error removing '%s' file.\n", global.taskname, global.tmpevtfilename);
	  goto Error;
	}
    
    /* Create output file */
      if ((outunit = OpenWriteFitsFile(global.tmpevtfilename)) <= (FitsFileUnit_t )0)
	{
	  headas_chat(NORMAL, "pass1timerec_work: Error creating '%s' fits file.\n", global.tmpevtfilename);
	  goto Error;
	}
      
    /* Get number of hdus in input events file */
    if (fits_get_num_hdus(inunit, &inExt, &status))
      {
	headas_chat(CHATTY, "pass1timerec_work: Error getting hdus num in '%s' file.\n", global.infile);
	goto Error;
      }
    
    /* Copy all extension before event extension  from input to output file */
    outExt=1;
    
    while(outExt<evExt && status == OK)
      {
	if(fits_movabs_hdu( inunit, outExt, NULL, &status ))
	  headas_chat(CHATTY, "pass1timerec_work: Error: unable to move in hdu %d in  '%s' file. \n", outExt, global.infile);
	if(fits_copy_hdu( inunit, outunit, 0, &status ))
	  headas_chat(CHATTY, "pass1timerec_work: Error: unable to copy hdu %d from input to output file. \n", outExt);
	
	outExt++;
      }
    
    if (status) {
      headas_chat(CHATTY,"pass1timerec_work: Error creating output extentions.\n");
      goto Error;
    }
    
    /* make sure get specified header by using absolute location */
    if(fits_movabs_hdu( inunit, evExt, NULL, &status ))
      {
	headas_chat(CHATTY,"pass1timerec_work: Error: Unable to move to requested extension.\n");
	goto Error;
      }
    headas_chat(NORMAL, "%s: Info: Processing '%s' file.\n", global.taskname, global.par.outfile);

    /* Event recognition */

    if(global.wt)
      {
	if ((WTEventRec(inunit, outunit)))
	  {
	    headas_chat(CHATTY, "pass1timerec_work: Error writing events extension in '%s' file.\n", global.tmpevtfilename);
	    goto Error;
	  }
      }
     else if (global.pd)
      {
	if ((PDPUEventRec(inunit, outunit)))
	  {
	    headas_chat(CHATTY, "pass1_evtrec_work: Error writing events extension in '%s' file.\n", global.tmpevtfilename);
	    goto Error;
	  }
      }

    if(!global.par.addcol)
      {
	if(DelCols(outunit))
	  {
	    headas_chat(NORMAL, "pass1_evtrec_work: Error deleting columns.\n");
	    goto Error;
	  }
      }

    outExt++;
    
    /* copy any extension after the extension to be operated on */
    while ( status == OK && outExt <= inExt) 
      {
	if(fits_movabs_hdu( inunit, outExt, NULL, &status ))
	  {
	    headas_chat(CHATTY, "pass1timerec_work: Error moving in  events extension in '%s' file.\n", global.infile);
	    goto Error;
	  }
	if(fits_copy_hdu ( inunit, outunit, 0, &status ))
	  {
	    headas_chat(CHATTY, "pass1timerec_work: Error copying extensions in '%s' file.\n", global.tmpevtfilename);
	    goto Error;
	  }
	
	outExt++;
      }
    /* Add history if parameter history set */
    headas_parstamp(outunit, evExt);
    
    /* Add checksum and datasum in all extensions */
    if (ChecksumCalc(outunit))
      {
	headas_chat(CHATTY, "%s: Error: unable to update checksum in '%s' file. \n ", global.taskname, global.tmpevtfilename);
	goto Error;
      }
    /* close files */
    if (CloseFitsFile(inunit))
      {
	headas_chat(CHATTY, "%s: Error:  unable to close  '%s' file. \n ", global.taskname, global.infile);
	goto Error;
      }  
    if (!CloseFitsFile(outunit)) 
      headas_chat(CHATTY,"%s: Info: File '%s' successfully written.\n",global.taskname,
		  global.tmpevtfilename);
    
    /* rename temporary file into output event file */
    if (rename (global.tmpevtfilename,global.par.outfile) == -1)
      {
	headas_chat(NORMAL, "%s: Error renaming temporary file.\n", global.taskname);
	goto Error;
      }
    
    headas_chat(NORMAL, "%s: Info: '%s' file successfully written.\n", global.taskname, global.par.outfile);

    PrintStatistics();
    
    /* Get next filename */    
  skip_file:
    status=0;
    strcpy(global.par.outfile, "NONE");
    global.infile += strlen(global.infile)+1;
    global.infile = strtok(global.infile, " \n\t;:");
    
  }

  headas_chat(MUTE, "%s: ... Exit with success.\n", global.taskname);
  
  return OK;
 
 Error:
  
  return NOT_OK;
} /* pass1timerec_work  OK*/
/*
 *	WTEventRec
 *
 *
 *	DESCRIPTION:
 *
 *     
 *	DOCUMENTATION:
 *        
 *
 *      FUNCTION CALL:
 *             FitsHeader_t   RetrieveFitsHeader(FitsFileUnit_t unit);
 *             void GetBintableStructure (FitsHeader_t *header,	Bintable_t *table,
 *     					  const int MaxBlockRows, const unsigned nColumns,
 *  					  const unsigned ActCols[]);
 *             int  ColNameMatch(const char *ColName, const Bintable_t *table);
 *             int headas_chat(int ,char *, ...);
 *             void AddColumn(FitsHeader_t *header, Bintable_t *table, const char *ttype,
 * 			      const char *TypeComment, const char *tform, const unsigned present, ...);
 *             void AddCard(FitsHeader_t *header, const char *KeyWord, const ValueTag_t vtag,
 *                          const void *value, const char *comment);
 *             void DeleteCard(FitsHeader_t *header, const char *KeyWord);
 *             void  GetGMTDateTime(char *);
 *             int sprintf(char *str, const char *format, ...);
 *
 *             void AddHistory(FitsHeader_t *header, const char *Comment);
 *             void EndBintableHeader(FitsHeader_t *header, const Bintable_t *table);
 *             void FinishBintableHeader(const FitsFileUnit_t unit, FitsHeader_t *header, Bintable_t *table);
 *             int  ReadBintable(const FitsFileUnit_t unit, Bintable_t *table, const unsigned nColumns,
 *				 const unsigned ActCols[], const unsigned FromRow, unsigned *nRows);
 *             int WriteFastBintable(FitsFileUnit_t unit, Bintable_t *table, unsigned nRows, BOOL last);
 *             unsigned	ReleaseBintable(FitsHeader_t *head, Bintable_t *table);
 *
 *      CHANGE HISTORY:
 *
 *        0.1.0: - BS 21/03/2003 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 *
 *      Return Status: 
 *        0: OK
 *        1: NOT_OK
 */
int WTEventRec(FitsFileUnit_t evunit,FitsFileUnit_t ounit)
{
  int                status=0, start, qq=0;
  int                blockn = 0, logical;
  int                next_ccdframe=0, next_rawy=0, ccdframe=0, rawy=0;
  long               extno=-1;
  BOOL               stop_flag=0;
  char		     KeyWord[FLEN_KEYWORD];
  unsigned           FromRow, ReadRows, n=0,nCols=0, OutRows=0, WriteRows=0;  
  Ev2Col_t           indxcol;
  Bintable_t	     intable;
  FitsCard_t         *card;
  FitsHeader_t	     head;
 
  TMEMSET0( &intable, Bintable_t );
  TMEMSET0( &head, FitsHeader_t );
 
  if(global.first)
    {
      /********************************
       *     Get CALDB grades file    *
       ********************************/
      
      if ( !strcasecmp(global.par.gradefile,DF_CALDB) )
	{
	  if (CalGetFileName(HD_MAXRET, DF_NOW, DF_NOW, DF_NOW, DF_NOW, KWVL_GRADE_DSET, global.par.gradefile, "datamode.eq.windowed", &extno ))
	    {
	      headas_chat(NORMAL, "%s: Error getting CALDB grade file.\n", global.taskname);
	      goto event_end;
	    }
	  extno++;
	}
      headas_chat(NORMAL, "%s: Info: Processing '%s' file.\n", global.taskname, global.par.gradefile);
      
      if (GetGrades(extno)) 
	{
	  headas_chat(CHATTY, "%s: Error processing grade file.\n", global.taskname);
	  goto event_end;
	}
      global.first=0;
    }

  head=RetrieveFitsHeader(evunit);
  
  /* Check if Events are ......  */
  if(ExistsKeyWord(&head, KWNM_XRTEVREC, &card))
    global.evtrec=card->u.LVal;
  else
    global.evtrec=FALSE;
    
  if(global.evtrec)
    {
      headas_chat(NORMAL, "WTEventRec: Warning event recognition is already done.\n");
      goto event_end;
    }


  /* Get event and split threshold */
  if(global.par.event < 0)
    {
      if(ExistsKeyWord(&head, KWNM_LLD, &card))
	global.onboardevent=card->u.JVal;
      else
	global.onboardevent=DEFAULT_LLD;
    }
  else
    global.onboardevent=global.par.event;

  if(global.par.split < 0)
    {
      if(ExistsKeyWord(&head, KWNM_LLD, &card))
	global.onboardsplit=card->u.JVal;
      else
	global.onboardsplit=DEFAULT_LLD;
    }
  else
    global.onboardsplit=global.par.split;

  /* If 'guardflag' parameter set to yes, get outer threshold */ 
  if(global.par.guardflag)
    {
      if(global.par.outer < 0)
	{
	  if(ExistsKeyWord(&head, KWNM_LLD, &card))
	    global.onboardouter=card->u.JVal;
	  else
	    global.onboardouter=DEFAULT_LLD;
	}
      else
	global.onboardouter=global.par.outer;
    }

  headas_chat(NORMAL, "%s: Info using split threshold %d and event threshold %d.\n", global.taskname,global.onboardsplit,global.onboardevent);
  
  if(global.par.guardflag)
    headas_chat(NORMAL, "%s: Info using outer threshold %d.\n", global.taskname,global.onboardouter);

  blockn=BINTAB_ROWS;
  if (blockn < 650)
    blockn=650;

  /* Initialize bntable with number of rows = blockn*BINTAB_ROWS */
  InitBintable(&head, &intable, &nCols, &indxcol, blockn);
  
  EndBintableHeader(&head, &intable); 
 
  /* write out new header to new file */
  FinishBintableHeader(ounit, &head, &intable);
 
  FromRow = 1;
  global.totrows=global.maxrows;

  ReadRows=intable.nBlockRows;
  OutRows=0;
 
 
 /* Read input bintable */
 while(ReadBintable(evunit, &intable, nCols, NULL,FromRow,&ReadRows) == 0 && ReadRows!=0 )
   {
     for(n=0; n<ReadRows; ++n)
       {

 	 stop_flag=0;
	 start=n;
		 
	 while(n+1 < ReadRows && !stop_flag && (OutRows+1) < blockn)
	   {
	     ccdframe=JVEC(intable, n, indxcol.CCDFrame);
	     next_ccdframe=JVEC(intable, (n+1), indxcol.CCDFrame);

	     rawy=IVEC(intable, n, indxcol.RAWY);
	     next_rawy=IVEC(intable, (n+1), indxcol.RAWY);
	     
	     if (ccdframe == next_ccdframe &&  rawy == next_rawy)
	       n++;
	     else
	       stop_flag=1;
	   }/* End while((n+1) < ReadRows ...)*/ 
	      	     
	 
	 if (stop_flag)
	   {
	     for (qq=start; qq <= n; qq++) 
	       EVEC(intable, qq, indxcol.EVTPHA)=EVEC(intable, qq, indxcol.PHA); 
	     
	     if(WTEventRec3x1(start, n, intable, indxcol))
	       {
		 headas_chat(NORMAL, "WTEventRec: Error: impossible to assign grade to events.\n");
		 goto event_end;
	       }
	     OutRows = n+1;
	     global.totrows -= (n - start + 1);
	   }
	      
	 else
	   {
	     
	     if ((global.totrows - (n - start + 1)) == 0)
	       {
		 for (qq=start; qq <= n; qq++)
		   EVEC(intable, qq, indxcol.EVTPHA)=EVEC(intable, qq, indxcol.PHA);

		 if(WTEventRec3x1(start, n, intable, indxcol))
		   {
		     headas_chat(NORMAL, "WTEventRec: Error: impossible to assign grades. \n");
		     goto event_end;
		   }
		 OutRows = n+1;
		 global.totrows -= (n - start + 1);
	       }
	   }
	     
       }
     
     WriteRows=OutRows;

     if(global.par.delnull)
       {
	 
	 if(RemoveNullEvents( &intable, &WriteRows, indxcol.PHA))
	   {  
	     headas_chat(NORMAL, "WTEventRec: Error removing events with PHA set to NULL.\n");
	     goto event_end;
	   }
       }
     
     WriteFastBintable(ounit, &intable, WriteRows, FALSE);
     FromRow += OutRows;
     OutRows=0;
     ReadRows = blockn;
   }
 
 if (OutRows > 0 )
   {
     WriteRows=OutRows;
     if(global.par.delnull)
       {
	 
	 if(RemoveNullEvents( &intable, &WriteRows, indxcol.PHA))
	   {  
	     headas_chat(NORMAL, "WTEventRec: Error removing events with PHA set to NULL.\n");
	     goto event_end;
	   }
       }
     WriteFastBintable(ounit, &intable, WriteRows, TRUE);
   }

 else
   WriteFastBintable(ounit, &intable, 0, TRUE);
 /*  ReleaseBintable(&head, &intable);  */
 
 if (!global.evtrec)
   {
     logical=TRUE;
     if(fits_update_key(ounit, TLOGICAL, KWNM_XRTEVREC, &logical, CARD_COMM_XRTEVREC, &status))
       goto event_end;
   }
 /* Update column comment */
 sprintf(KeyWord, "TTYPE%d", (indxcol.PHA+1));
 if (fits_modify_comment(ounit, KeyWord, "Photon reconstructed PHA", &status))
   {
     headas_chat(NORMAL, "Error updating comment for %s column.\n", CLNM_PHA);
     goto event_end;
   }
 /* Add statitistic keywords into events hdu */
 if(AddStatKeywords(&ounit))
   {
     headas_chat(NORMAL, "WTEventRec: Error adding keywords.\n");
     goto event_end;
   }
 
 return OK;
 
 event_end:
 if (head.first)
   ReleaseBintable(&head, &intable);
 
 return NOT_OK;
 
} /* WTEventRec OK */
/*
 *	WTEventRec3x1
 *
 *
 *	DESCRIPTION:
 *        
 *
 *	DOCUMENTATION:
 *
 *
 *      FUNCTION CALL:
 *             void set_toolversion(char *);
 *             void set_toolname(char *);   
 *             void get_toolname(char *);
 *             void get_toolnamev(char *);
 *             int headas_chat(int ,char *, ...);
 *
 *             void xrtevtrec_getpar(void);
 *             void WriteInfoParams(void); 
 * 	       void xrtevtrec_work(void);
 *
 *          
 *	CHANGE HISTORY:
 *        0.1.0: - BS 31/03/2003 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 */
int WTEventRec3x1( int start, int stop, Bintable_t intable, Ev2Col_t indxcol)
{

  int        totrow, j, jj,  i, count, q, grade=0, above=0;
  float      handle[PHAS3_MOL], pha=0, next_pha=0;
  int        left_n =0, right_n = 0, left = 0, right = 0, flag_left = 0, flag_right=0, null_count=0;
  int        left_gn =0, right_gn = 0, gleft = 0, gright = 0, flag_gleft = 0, flag_gright=0;
  BOOL       found=0, cont_flag=0, flag_out=0;
  Ev2Info_t  **rowinfo;

  totrow=stop - start + 1;
  
  if (stop == start)
    { 
      /* Event is single */
      handle[0]=EVEC(intable, start, indxcol.EVTPHA);
      if (handle[0] > global.onboardevent)
	{
	  for (j=1; j < PHAS3_MOL; j++)
	    handle[j]= KWVL_PHANULL;
	  
	  if(wtfaint3x1(handle, &pha, &grade, &above))
	    {
	      headas_chat(NORMAL, "WTEventRec3x1: Error computing PHA.\n");
	      goto evrec_end;
	    }

	  /* Fill columns */
	  /* Put PHA values in PHAS column? */ 
	  if(global.par.addcol)
	    {
	      for (j=0; j < PHAS3_MOL; j++)
		EVECVEC(intable,start,indxcol.PHAS,j)=handle[j];
	      IVEC(intable, start, indxcol.ABOVE) = above;
	    }

	  EVEC(intable, start, indxcol.PHA) = pha;
	  IVEC(intable, start, indxcol.GRADE) = grade;
	 
	  global.grade[grade]++;
	}
      else
	SetNullEvent(intable, start, indxcol);
	
    }
  
  else
    {/* Do event reconstruction */
      /* Allocate memory to store information about rows of the table */
      rowinfo=(Ev2Info_t **)calloc(totrow, sizeof(Ev2Info_t *)); 
      
      for (j=0; j < totrow;j++)
	{	
	  rowinfo[j]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
	  rowinfo[j]->nrow=0;
	  rowinfo[j]->flag=0;
	  rowinfo[j]->rawx=0;
	}

      count=start;

      /*
       * Sort pha values 
       */
      
      /* Put first value in array */
      rowinfo[0]->nrow=count;
      count++;
      
      /* Put and order next values */
      
      for (j=1; count <= stop; count++, j++)
	{
	  found=0;
	  next_pha = EVEC(intable, count, indxcol.EVTPHA);

	  for (i=j; i>=0 && !found ; i--)
	    {
	      if (i == 0)
		rowinfo[i]->nrow = count;
	      else
		{
		  pha = EVEC(intable, rowinfo[i-1]->nrow, indxcol.EVTPHA);
		  
		  if ( pha > next_pha )
		    {
		      rowinfo[i]->nrow = count;
		      found=1;
		    }
		  else
		    rowinfo[i]->nrow = rowinfo[i-1]->nrow;
		}
	    }	
	}
      
      /* Put rawx values in the array */
      for (j=0; j < totrow; j++)
	rowinfo[j]->rawx = IVEC(intable, rowinfo[j]->nrow, indxcol.RAWX);


      null_count = totrow;

      /* Loop on all pha values */

      for (j=0; j < totrow; j++) 
	{
	  for (q=0; q < PHAS3_MOL ; q++)
	    handle[q]=KWVL_PHANULL;
	  cont_flag=1;
	   
	  if (!rowinfo[j]->flag)
	    {/* Central event */
	      flag_left=0;
	      flag_right=0;

	      left = 0;
	      right = 0;
	      /* Get pha value */
	      handle[0]=EVEC(intable, rowinfo[j]->nrow, indxcol.EVTPHA);

	      /* If pha > event threshold check neighbors */
	      if (handle[0] > global.onboardevent)
		{
		  /* Check neighbours */
		
		  for (jj = 0; jj < null_count ; jj++)
		    {
		      if (rowinfo[jj]->rawx == ( rowinfo[j]->rawx - 1))
			    {
			      left_n = jj;
			      left = 1;
			      flag_left = rowinfo[jj]->flag;
			      if (right)
				jj=null_count;
			    }
		      else if(rowinfo[jj]->rawx == (rowinfo[j]->rawx + 1))
			    {
			      right_n = jj;
			      right = 1;
			      flag_right = rowinfo[jj]->flag;
			      if (left)
				jj=null_count;
			    }
		    }
		  
		  if(right)
		    {/* if right neigh is flagged go to next pha values */
		      if(flag_right)
			cont_flag=0;
		    }

		  if(left && cont_flag)
		    {/* if left neigh is flagged go to next pha values */
		      if(flag_left)
			cont_flag=0;
		    }

		  if(global.par.guardflag && cont_flag)
		    {
		      /* Check outer pixels if 'guardflag' set to 'yes' */
		      flag_gleft=0;
		      flag_gright=0;
		      gleft=0;
		      gright=0;
		      
		      for (jj = 0; jj < null_count ; jj++)
			{
			  if (rowinfo[jj]->rawx == ( rowinfo[j]->rawx - 2))
			    {
			      left_gn = jj;
			      gleft = 1;
			      flag_gleft = rowinfo[jj]->flag;
			      if (gright)
				jj=null_count;
			    }
			  else if(rowinfo[jj]->rawx == (rowinfo[j]->rawx + 2))
			    {
			      right_gn = jj;
			      gright = 1;
			      flag_gright = rowinfo[jj]->flag;
			      if (gleft)
				jj=null_count;
			    }
			}
		      
		      if(gright && gleft)
			{
			  if(!(handle[0] > EVEC(intable, rowinfo[left_gn]->nrow, indxcol.EVTPHA)
			       && handle[0] > EVEC(intable, rowinfo[right_gn]->nrow, indxcol.EVTPHA)
			       && EVEC(intable, rowinfo[left_gn]->nrow, indxcol.EVTPHA) < global.onboardouter
			       && EVEC(intable, rowinfo[right_gn]->nrow, indxcol.EVTPHA) < global.onboardouter
			       && !flag_gright && !flag_gleft))
			    {
			      cont_flag = 0;
			    }
			}
		      else if (gright)
			{
			  
			  if(!( handle[0] > EVEC(intable, rowinfo[right_gn]->nrow, indxcol.EVTPHA)
				&& EVEC(intable, rowinfo[right_gn]->nrow, indxcol.EVTPHA) < global.onboardouter
				&& !flag_gright))
			    {
			      cont_flag = 0;
			    }
			}
		      else if(gleft)
			{
			  if(!(handle[0] > EVEC(intable, rowinfo[left_gn]->nrow, indxcol.EVTPHA)
			       && EVEC(intable, rowinfo[left_gn]->nrow, indxcol.EVTPHA) < global.onboardouter
			       && !flag_gleft))
			    {
			      cont_flag = 0;
			    }
			}
		    }
		  /* if neighbour and outer pixels are not flagged assign grades */
		  if (cont_flag)
		    {
		      flag_out=0;
		      /* Assign grade */
		      if (left && right)
			{
			  /* Get neighbour pha value */
			  handle[1]=EVEC(intable, rowinfo[left_n]->nrow, indxcol.EVTPHA);
			  handle[2]=EVEC(intable, rowinfo[right_n]->nrow, indxcol.EVTPHA);
			  if (handle[0] >= handle[1] && handle[0] >= handle[2])
			    {
			      rowinfo[left_n]->flag = 1;
			      SetNullEvent(intable, rowinfo[left_n]->nrow, indxcol);
			      
			      handle[2]=EVEC(intable, rowinfo[right_n]->nrow, indxcol.EVTPHA);
			      rowinfo[right_n]->flag = 1;
			      SetNullEvent(intable, rowinfo[right_n]->nrow, indxcol);
			      
			      /* Compute total pha, grade and pixsabove */
			      
			      if(wtfaint3x1(handle, &pha, &grade, &above))
				{
				  headas_chat(NORMAL, "WTEventRec3x1: Error computing PHA.\n");
				  goto evrec_end;
				}
				  
			      if(global.par.addcol)
				{
				  for (q=0; q < PHAS3_MOL ; q++)
				    EVECVEC(intable,rowinfo[j]->nrow,indxcol.PHAS,q) = handle[q];
				  
				  IVEC(intable, rowinfo[j]->nrow, indxcol.ABOVE) = above;
				}
				  
			      EVEC(intable,rowinfo[j]->nrow , indxcol.PHA) = pha;
			      IVEC(intable,rowinfo[j]->nrow, indxcol.GRADE) = grade;
				  
			      rowinfo[j]->flag = 1;
			      global.grade[grade]++;
			    }/* End if(handle[0]>handle[1] && handle[0]>handle[2]) */
			      
			}/* End if (left && right) */
		      else if (left )
			{
			  /* Get neighbour pha value */
			  handle[1]=EVEC(intable, rowinfo[left_n]->nrow, indxcol.EVTPHA);
			  if (handle[0] >= handle[1])
			    {
			      rowinfo[left_n]->flag = 1;
			      SetNullEvent(intable, rowinfo[left_n]->nrow, indxcol);
			      
			      handle[2]=KWVL_PHANULL;
			      
			      /* Compute total pha, grade and pixsabove */
			      if(wtfaint3x1(handle, &pha, &grade, &above))
				{
				  headas_chat(NORMAL, "WTEventRec3x1: Error computing PHA.\n");
				  goto evrec_end;
				}
			      if(global.par.addcol)
				{
				  for (q=0; q < PHAS3_MOL ; q++)
				    EVECVEC(intable,rowinfo[j]->nrow,indxcol.PHAS,q) = handle[q];
				  IVEC(intable, rowinfo[j]->nrow, indxcol.ABOVE) = above;
				}
			      
			      EVEC(intable,rowinfo[j]->nrow , indxcol.PHA) = pha;
			      
			      IVEC(intable,rowinfo[j]->nrow, indxcol.GRADE) = grade;
			      
			      rowinfo[j]->flag = 1;
			      global.grade[grade]++;
			      rowinfo=(Ev2Info_t **)realloc(rowinfo, ((sizeof(Ev2Info_t *))*(null_count+1))); 
			      /* Added right neighbor to the list */
			      rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
			      rowinfo[null_count]->nrow=0;
			      rowinfo[null_count]->rawx = (rowinfo[j]->rawx + 1); 
			      rowinfo[null_count]->flag=1;
			      null_count++;
			    }/* End if(handle[0] > handle[1]) */
			}/* Enf if(left && !flag) */
		      else if (right)
			{
			  /* Get neighbor pha value */
			  handle[2]=EVEC(intable, rowinfo[right_n]->nrow, indxcol.EVTPHA);
			  if (handle[0] >= handle[2])
			    {
			      rowinfo[right_n]->flag = 1;
			      SetNullEvent(intable, rowinfo[right_n]->nrow, indxcol);
			      handle[1]=KWVL_PHANULL;
			      if(wtfaint3x1(handle, &pha, &grade, &above))
				{
				  headas_chat(NORMAL, "WTEventRec3x1: Error computing PHA.\n");
				  goto evrec_end;
				}
			      
			      EVEC(intable,rowinfo[j]->nrow , indxcol.PHA) = pha;
			      
			      IVEC(intable,rowinfo[j]->nrow, indxcol.GRADE) = grade;
			      
			      /* Compute total pha, grade and pixsabove */
			      if(global.par.addcol){
				for (q=0; q < PHAS3_MOL ; q++)
				  EVECVEC(intable,rowinfo[j]->nrow,indxcol.PHAS,q) = handle[q];
				IVEC(intable, rowinfo[j]->nrow, indxcol.ABOVE) = above;
			      }
			      rowinfo[j]->flag = 1;
			      global.grade[grade]++;
			      
			      rowinfo=(Ev2Info_t **)realloc(rowinfo, ((sizeof(Ev2Info_t *))*(null_count+1))); 
			      /* Added left neighbor to the list */
			      rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
			      rowinfo[null_count]->nrow=0;
			      rowinfo[null_count]->flag=1;
			      rowinfo[null_count]->rawx=(rowinfo[j]->rawx - 1);
			      null_count++;
			    }/* End if(handle[0] > handle[2] */
			}/*  else if (right && !flag_right) */
		      else if (!left && !right)
			{
			  for (q=1; q < PHAS3_MOL; q++)
			    handle[q]=KWVL_PHANULL;
			  
			  if(wtfaint3x1(handle, &pha, &grade, &above))
			    {
			      headas_chat(NORMAL, "WTEventRec3x1: Error computing PHA.\n");
			      goto evrec_end;
			    }
			  
			  if(global.par.addcol)
			    {
			      for (q=0; q < PHAS3_MOL ; q++)
				EVECVEC(intable,rowinfo[j]->nrow,indxcol.PHAS,q) = handle[q];
			      IVEC(intable, rowinfo[j]->nrow, indxcol.ABOVE) = above;
			    }
			  
			  EVEC(intable,rowinfo[j]->nrow , indxcol.PHA) = pha;
			  
			  IVEC(intable,rowinfo[j]->nrow, indxcol.GRADE) = grade;
			  
			  rowinfo[j]->flag = 1;
			  
			  global.grade[grade]++;
			  
			  rowinfo=(Ev2Info_t **)realloc(rowinfo, ((sizeof(Ev2Info_t *))*(null_count+2))); 
			  /* Added left and right neighbors to the list */
			  rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
			  rowinfo[null_count]->nrow=0;
			  rowinfo[null_count]->flag=1;
			  rowinfo[null_count]->rawx=(rowinfo[j]->rawx - 1);
			  null_count++;
			  
			  rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
			  rowinfo[null_count]->nrow=0;
			  rowinfo[null_count]->flag=1;
			  rowinfo[null_count]->rawx=(rowinfo[j]->rawx + 1);
			  null_count++;
			  
			}/* End if (!left && !right ) */
		      

		      if(global.par.guardflag)
			{/* If 'guardflag' parameter set to 'yes' flag outer pixels */
			  if(!gleft)
			    {
			      rowinfo=(Ev2Info_t **)realloc(rowinfo, ((sizeof(Ev2Info_t *))*(null_count+1))); 
			      rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
			      rowinfo[null_count]->nrow=0;
			      rowinfo[null_count]->flag=1;
			      rowinfo[null_count]->rawx=(rowinfo[j]->rawx - 2 );
			      
			      null_count++;
			    }
			  else
			    {
			      
			      SetNullEvent(intable, rowinfo[left_gn]->nrow, indxcol);
			      rowinfo[left_gn]->flag = 1;
			    }
			  if(!gright)
			    {
			      rowinfo=(Ev2Info_t **)realloc(rowinfo, ((sizeof(Ev2Info_t *))*(null_count+1))); 
			      rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
			      rowinfo[null_count]->nrow=0;
			      rowinfo[null_count]->flag=1;
			      rowinfo[null_count]->rawx=(rowinfo[j]->rawx + 2 );
			      
			      null_count++;
			    }
			  else
			    {
			      
			      SetNullEvent(intable, rowinfo[right_gn]->nrow, indxcol);
			      rowinfo[right_gn]->flag = 1;
			    }
			  
			}
		    }/* End if(cont_flag) */
		  
		}/* End if (handle[0] > eventthreshold) */
	      else
		j = totrow+1;
	    }
	
      
	      
    }/* End loop on pha values */
	
      for (j=0; j < totrow ; j++)
	{
	  if(!rowinfo[j]->flag)
	    SetNullEvent(intable, rowinfo[j]->nrow, indxcol);
	}
      
      /*  Free memory allocated */
      for (j=0; j < null_count; j++ ) 
	{

	  free(rowinfo[j]);
	}
      free(rowinfo);
    }
  return OK;
  
 evrec_end:
  return NOT_OK;
  
  
}/* WTEventRec3x1 */
/*
  wtfaint3x1
*/

int wtfaint3x1(float handle[PHAS3_MOL], float *pha, int *grade, int *above)
{
  int ii=0, jj=0, pattern[PHAS3_MOL], found;
  
  found=0;
  *grade=KWVL_GRADENULL;

  /* Assign grade */
  
  /* Central event */
  *pha=handle[0];
  pattern[0]=1;
  *above=1;

  /* neighbor */
  for (jj=1; jj< PHAS3_MOL; jj++)
    {
      if(handle[jj] >= global.onboardsplit)
	{
	  *above+=1;
	  *pha+=handle[jj];
	  pattern[jj]=1;
	}
      else
	pattern[jj]=2;
    }
  
  for(ii=0; ii<global.graderows && *grade==KWVL_GRADENULL; ii++)
    {
      for(jj=0; jj<PHAS3_MOL && found==0; jj++) 
	if((pattern[jj]!=global.gradevalues[ii].grade[jj]) && (global.gradevalues[ii].grade[jj]))
	  found=1;
      
      if(!found)
	*grade=global.gradevalues[ii].gradeid;
      else
	found=0;
      
    }


  return OK;
  
}/* wtfaint3x1*/

/*
 *	EventRec3x1
 *
 *
 *	DESCRIPTION:
 *        
 *
 *	DOCUMENTATION:
 *
 *
 *      FUNCTION CALL:
 *             void set_toolversion(char *);
 *             void set_toolname(char *);   
 *             void get_toolname(char *);
 *             void get_toolnamev(char *);
 *             int headas_chat(int ,char *, ...);
 *
 *             void pass1timerec_getpar(void);
 *             void WriteInfoParams(void); 
 * 	       void pass1timerec_work(void);
 *
 *          
 *	CHANGE HISTORY:
 *        0.1.0: - BS 31/03/2003 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 */
int EventRec3x1( int start, int stop, Bintable_t intable, Ev2Col_t indxcol)
{

  int        totrow, j, jj,  i, count, q, grade=0, above=0;
  float      handle[PHAS3_MOL], pha=0, next_pha=0;
  int        left_n =0, right_n = 0, left = 0, right = 0, flag_left = 0, flag_right=0, null_count=0;
  BOOL       found=0;
  Ev2Info_t  **rowinfo;

  totrow=stop - start + 1;
  
  if (stop == start)
    { /* Event is single */
      handle[0]=EVEC(intable, start, indxcol.EVTPHA);
      if(handle[0] > global.onboardevent)
	{
	  for (j=1; j <  PHAS3_MOL; j++)
	    handle[j]=KWVL_PHANULL;

	  /* Put PHA values in PHAS column? */ 
	  if(global.par.addcol)
	    {
	      for (j=0; j < PHAS3_MOL; j++)
		{
		  EVECVEC(intable,start,indxcol.PHAS,j)=handle[j];
		  IVEC(intable, start, indxcol.ABOVE) = 1;
		}
	    }
	  EVEC(intable, start, indxcol.PHA) = handle[0];
	  IVEC(intable, start, indxcol.GRADE) = 0;
	 
	  global.grade[0]++;
	}
      else
	{
	  SetNullEvent(intable, start, indxcol);
	}
    }
  
  else
    {/* Do event reconstruction */
      /* Allocate memory to store information about rows of the table */
      rowinfo=(Ev2Info_t **)calloc(totrow, sizeof(Ev2Info_t *)); 

      
      for (j=0; j < totrow;j++)
	{	
	  rowinfo[j]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
	  rowinfo[j]->nrow=0;
	  rowinfo[j]->flag=0;
	  rowinfo[j]->rawx=0;
	}

      count=start;

      /* Sort pha values */ 
      /* Put first value in array */
      rowinfo[0]->nrow=count;
      count++;
      
      /* Put and order next values */
      
      for (j=1; count <= stop; count++, j++)
	{
	  found=0;
	  next_pha = EVEC(intable, count, indxcol.EVTPHA);

	  for (i=j; i>=0 && !found ; i--)
	    {
	      if (i == 0)
		rowinfo[i]->nrow = count;
	      else
		{
		  pha = EVEC(intable, rowinfo[i-1]->nrow, indxcol.EVTPHA);
		  
		  if ( pha > next_pha )
		    {
		      rowinfo[i]->nrow = count;
		      found=1;
		    }
		  else
		    rowinfo[i]->nrow = rowinfo[i-1]->nrow;
		}
	    }	
	}

      /* Put rawx values in the array */
      for (j=0; j < totrow; j++)
	rowinfo[j]->rawx = IVEC(intable, rowinfo[j]->nrow, indxcol.RAWX);
       
      null_count = totrow;

      /* Loop on all pha values */
      
      for (j=0; j < totrow; j++)
	{
	   for (q=0; q < PHAS3_MOL ; q++)
	     handle[q]=KWVL_PHANULL;
	   
	  if (!rowinfo[j]->flag)
	    {/* Central event */
	      left = 0;
	      right = 0;
	      handle[0]=EVEC(intable, rowinfo[j]->nrow, indxcol.EVTPHA);
	      
	      /* If pha > event threshold check neighbors */
	      if (handle[0] > global.onboardevent)
		{

		  /* Check neighbours */
		
		  for (jj = 0; jj < null_count ; jj++)
		    {
		      if (rowinfo[jj]->rawx == ( rowinfo[j]->rawx - 1))
			{
			  left_n = jj;
			  left = 1;
			  flag_left = rowinfo[jj]->flag;
			  if (right)
			    jj=null_count;
			}
		      else if(rowinfo[jj]->rawx == (rowinfo[j]->rawx + 1))
			{
			  right_n = jj;
			  right = 1;
			  flag_right = rowinfo[jj]->flag;
			  if (left)
			    jj=null_count;
			}
		    }
		  
		  if (left && right)
		    {
		      if (!flag_right && !flag_left)
			{
			  /* Get neighbor pha value */
			  handle[1]=EVEC(intable, rowinfo[left_n]->nrow, indxcol.EVTPHA);
			  handle[2]=EVEC(intable, rowinfo[right_n]->nrow, indxcol.EVTPHA);
			  if (handle[0] >= handle[1] && handle[0] >= handle[2])
			    {
			      rowinfo[left_n]->flag = 1;
			      SetNullEvent(intable, rowinfo[left_n]->nrow, indxcol);
			      
			      handle[2]=EVEC(intable, rowinfo[right_n]->nrow, indxcol.EVTPHA);
			      rowinfo[right_n]->flag = 1;
			      SetNullEvent(intable, rowinfo[right_n]->nrow, indxcol);
			      
			      /* Compute total pha, grade and pixsabove */
			      if(global.par.addcol)
				{
				  for (q=0; q < PHAS3_MOL ; q++)
				    EVECVEC(intable,rowinfo[j]->nrow,indxcol.PHAS,q) = handle[q];
				}
			      if(faint3x1(handle, &pha, &grade, &above))
				{
				  headas_chat(NORMAL, "EventRec3x1: Error computing PHA.\n");
				  goto evrec_end;
				}
			      
			      EVEC(intable,rowinfo[j]->nrow , indxcol.PHA) = pha;
			      
			      IVEC(intable,rowinfo[j]->nrow, indxcol.GRADE) = grade;
			      if(global.par.addcol)
				IVEC(intable, rowinfo[j]->nrow, indxcol.ABOVE) = above;
			      rowinfo[j]->flag = 1;
			    }/* End if(handle[0]>handle[1] && handle[0]>handle[2]) */

			}/* End if (!flag_left ...) */
		    }/* End if (left && right) */
		  else if (left && !rowinfo[left_n]->flag)
		    {
		      /* Get neighbor pha value */
		      handle[1]=EVEC(intable, rowinfo[left_n]->nrow, indxcol.EVTPHA);
		      if (handle[0] >= handle[1])
			{
			  rowinfo[left_n]->flag = 1;
			  SetNullEvent(intable, rowinfo[left_n]->nrow, indxcol);
			  
			  handle[2]=KWVL_PHANULL;
			  
			  /* Compute total pha, grade and pixsabove */
			  
			  if(global.par.addcol)
			    {
			      for (q=0; q < PHAS3_MOL ; q++)
				EVECVEC(intable,rowinfo[j]->nrow,indxcol.PHAS,q) = handle[q];
			    }
			  if(faint3x1(handle, &pha, &grade, &above))
			    {
			      headas_chat(NORMAL, "EventRec3x1: Error computing PHA.\n");
			      goto evrec_end;
			    }
			  
			  EVEC(intable,rowinfo[j]->nrow , indxcol.PHA) = pha;
			  
			  IVEC(intable,rowinfo[j]->nrow, indxcol.GRADE) = grade;
			  if(global.par.addcol)
			    IVEC(intable, rowinfo[j]->nrow, indxcol.ABOVE) = above;
			  rowinfo[j]->flag = 1;
			  
			  rowinfo=(Ev2Info_t **)realloc(rowinfo, ((sizeof(Ev2Info_t *))*(null_count+1))); 
			  /* Added right neighbor to the list */
			  rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
			  rowinfo[null_count]->nrow=0;
			  rowinfo[null_count]->rawx = (rowinfo[j]->rawx + 1); 
			  rowinfo[null_count]->flag=1;
			  null_count++;
			}/* End if(handle[0] > handle[1]) */
		    }/* Enf if(left && !flag) */
		  else if (right && !flag_right)
		    {
		      /* Get neighbor pha value */
		      handle[2]=EVEC(intable, rowinfo[right_n]->nrow, indxcol.EVTPHA);
		      if (handle[0] >= handle[2])
			{
			  rowinfo[right_n]->flag = 1;
			  SetNullEvent(intable, rowinfo[right_n]->nrow, indxcol);
			 
			  
			  handle[1]=KWVL_PHANULL;
			  
			  /* Compute total pha, grade and pixsabove */
			  if(global.par.addcol)
			    {
			      for (q=0; q < PHAS3_MOL ; q++)
				EVECVEC(intable,rowinfo[j]->nrow,indxcol.PHAS,q) = handle[q];
			    }
			  if(faint3x1(handle, &pha, &grade, &above))
			    {
			      headas_chat(NORMAL, "EventRec3x1: Error computing PHA.\n");
			      goto evrec_end;
			    }
			  
			  EVEC(intable,rowinfo[j]->nrow , indxcol.PHA) = pha;
			  
			  IVEC(intable,rowinfo[j]->nrow, indxcol.GRADE) = grade;
			  if(global.par.addcol)
			    IVEC(intable, rowinfo[j]->nrow, indxcol.ABOVE) = above;
			  rowinfo[j]->flag = 1;
			  
			  rowinfo=(Ev2Info_t **)realloc(rowinfo, ((sizeof(Ev2Info_t *))*(null_count+1))); 
			  /* Added left neighbor to the list */
			  rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
			  rowinfo[null_count]->nrow=0;
			  rowinfo[null_count]->flag=1;
			  rowinfo[null_count]->rawx=(rowinfo[j]->rawx - 1);
			  null_count++;
			}/* End if(handle[0] > handle[2] */
		    }/*  else if (right && !flag_right) */
		  else if (!left && !right)
		    {
		      if(global.par.addcol)
			{
			  EVECVEC(intable,rowinfo[j]->nrow,indxcol.PHAS,0) = handle[0];
		     
			  for (q=1; q < PHAS3_MOL ; q++)
			    EVECVEC(intable,rowinfo[j]->nrow,indxcol.PHAS,q) = KWVL_PHANULL;
			  IVEC(intable, rowinfo[j]->nrow, indxcol.ABOVE) = 1;
			}  
		      EVEC(intable,rowinfo[j]->nrow , indxcol.PHA) = handle[0];
		      
		      IVEC(intable,rowinfo[j]->nrow, indxcol.GRADE) = 0;
		    
		      rowinfo[j]->flag = 1;
		      
		      global.grade[0]++;
		      rowinfo=(Ev2Info_t **)realloc(rowinfo, ((sizeof(Ev2Info_t *))*(null_count+2))); 
		      /* Added left and right neighbors to the list */
		      rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
		      rowinfo[null_count]->nrow=0;
		      rowinfo[null_count]->flag=1;
		      rowinfo[null_count]->rawx=(rowinfo[j]->rawx - 1);
		      null_count++;
		      
		      rowinfo[null_count]=(Ev2Info_t *)malloc(sizeof(Ev2Info_t)) ;
		      rowinfo[null_count]->nrow=0;
		      rowinfo[null_count]->flag=1;
		      rowinfo[null_count]->rawx=(rowinfo[j]->rawx + 1);
		      null_count++;
		      
		    }/* End if (!left && !right ) */
		  
		}
	      else
		j = totrow+1;
	    }
	}/* End loop on pha values */
      
      for (j=0; j < totrow ; j++)
	{
	  if(!rowinfo[j]->flag)
	    {
	      SetNullEvent(intable, rowinfo[j]->nrow, indxcol);
	    }
	}
      

      /* Free memory allocated */
      for (j=0; j < null_count; j++)
	{	
	  free((Ev2Info_t *)rowinfo[j]);
	}
      free(rowinfo);
    }
  return OK;
  
 evrec_end:
  return NOT_OK;
  
  
}/* EventRec3x1 OK */

/*
  faint3x1
*/

int faint3x1(float handle[PHAS3_MOL], float *pha, int *grade, int *above)
{
  int i=0;
  
  *pha=handle[0];
  *above=1;
  *grade=0;

  for(i=1; i<PHAS3_MOL ; i++)
    if (handle[i] >= global.onboardsplit)
      *pha+=handle[i];

  if (*pha >= SAT_VAL)
    {
      *pha=SAT_VAL;
      global.tot_sat++;
    }
  
  
  if(handle[1] >= global.onboardsplit && handle[2] < global.onboardsplit)
    {
      *grade=1;
      *above+=1;
      global.grade[1]++;
    }

  else if(handle[1] < global.onboardsplit && handle[2] >= global.onboardsplit)
    {
      *grade=2;
      *above+=1;
      global.grade[2]++;
    }
  
  else if(handle[1] >= global.onboardsplit && handle[2] >= global.onboardsplit)
     {
      *grade=3;
      *above+=2;
      global.grade[3]++;
     }

  if ( *grade == 0)
    global.grade[0]++;
     
  return OK;
  
}/* faint3x1 OK */




/*
 *	pass1timerec
 *
 *
 *	DESCRIPTION:
 *        
 *
 *	DOCUMENTATION:
 *
 *
 *      FUNCTION CALL:
 *             void set_toolversion(char *);
 *             void set_toolname(char *);   
 *             void get_toolname(char *);
 *             void get_toolnamev(char *);
 *             int headas_chat(int ,char *, ...);
 *
 *             void pass1timerec_getpar(void);
 *             void WriteInfoParams(void); 
 * 	       void pass1timerec_work(void);
 *
 *          
 *	CHANGE HISTORY:
 *        0.1.0: - BS 20/03/2003 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 */
int pass1timerec()
{
  /* set HEADAS globals */
  set_toolname(PRG_NAME);
  set_toolversion(PASS1TIMEREC_VERSION);

  get_toolnamev(global.taskname);
  
  GetSWXRTDASVersion(global.swxrtdas_v);

  /* Get parameter values */ 
  if ( pass1timerec_getpar() == OK ) {
    
    if ( pass1timerec_work() ) {
      headas_chat(MUTE, "%s: ... Exit with Error.\n", global.taskname);
      goto pass1_end;
    }
  }
  else
    goto pass1_end;
   

  return OK;
 pass1_end:
  if (FileExists(global.tmpevtfilename))
    remove (global.tmpevtfilename);
  return NOT_OK;   
} /* pass1timerec */
/*
 *	WriteInfoParams: 
 *
 *
 *	DESCRIPTION:
 *         Display information about input parameter.
 *     
 *
 *	DOCUMENTATION:
 *        
 *
 *      FUNCTION CALL:
 *               int headas_chat(int, char *, ...);
 *
 *	CHANGE HISTORY:
 *        0.1.0: - BS 08/02/2002 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 */
void WriteInfoParams ()
{
  headas_chat(MUTE,"---------------------------------------------------------------------\n");
  headas_chat(MUTE," \t\tRunning '%s'\n",global.taskname);
  headas_chat(MUTE,"---------------------------------------------------------------------\n");
  headas_chat(NORMAL,"\t\tList of input parameters: \n");
  headas_chat(NORMAL,"Name of input Event file         :'%s'\n",global.par.infiles);
  headas_chat(NORMAL,"Name of output Event file        :'%s'\n",global.par.outfile);
  headas_chat(NORMAL,"Name of input grade   file           :'%s'\n",global.par.gradefile);
  if (global.par.addcol)
    headas_chat(NORMAL,"Added 'PHAS' and 'PixsAbove colums?  : yes\n");
  else
    headas_chat(NORMAL,"Added 'PHAS' and 'PixsAbove colums?  : no\n");
  
  if (global.par.delnull)
    headas_chat(NORMAL,"Delete NULL events                   : yes\n");
  else
    headas_chat(NORMAL,"Delete NULL events                   : no\n");

  if (global.hist)
    headas_chat(CHATTY,"Add history                      : yes\n");
  else
    headas_chat(CHATTY,"Add history                      : no\n");
  if (headas_clobpar)
    headas_chat(CHATTY,"Clobber                          : yes\n");
  else
    headas_chat(CHATTY,"Clobber                          : no\n");

  headas_chat(NORMAL,"---------------------------------------------------------------------\n");
} /* WriteInfoParams */
/*
 *
 */

void SetNullEvent(Bintable_t intable, int n, Ev2Col_t indxcol)
{
  int q;
  if(global.par.addcol)
    {
      for (q=0; q < PHAS3_MOL ; q++)
	EVECVEC(intable, n,indxcol.PHAS,q) = KWVL_PHANULL;
      IVEC(intable, n, indxcol.ABOVE) = 0;
   }
  
  EVEC(intable, n, indxcol.PHA) = KWVL_PHANULL;
  IVEC(intable, n, indxcol.GRADE) = KWVL_GRADENULL;
  global.grade[4]++;
}

int InitBintable(FitsHeader_t *head, Bintable_t *intable, unsigned *nCols, Ev2Col_t *indxcol, int blockn)
{

  char		     KeyWord[FLEN_KEYWORD];
  ETYPE              null_pha;
  JTYPE              null_grade;

  null_pha=KWVL_PHANULL;
  null_grade=KWVL_GRADENULL;


  GetBintableStructure(head, intable, blockn, 0, NULL);

  if(*nCols == 0)
    {
      *nCols=intable->nColumns;
      global.maxrows=intable->MaxRows;
    }

  /* Get cols index from name */
   if(global.wt)
    {
      if((indxcol->RAWX=ColNameMatch(CLNM_RAWX, intable)) == -1)
	{
	  headas_chat(NORMAL, "AddEventBintable: Error %s column does not exist.\n", CLNM_RAWX);
	  goto event_end; 
	}
      
      if((indxcol->RAWY=ColNameMatch(CLNM_RAWY, intable)) == -1)
	{
	  headas_chat(NORMAL, "AddEventBintable: Error %s column does not exist.\n", CLNM_RAWY);
	  goto event_end; 
	}
    }
   /* Columns needed for all modes */
   if((indxcol->CCDFrame=ColNameMatch("CCDFRAME", intable)) == -1)
    {
      headas_chat(NORMAL, "AddEventBintable: Error %s column does not exist.\n", "CCDFRAME");
      goto event_end; 
    }

  if ((indxcol->EVTPHA=ColNameMatch(CLNM_EVTPHA, intable)) == -1)
    { /* if EVTPHA column does not exist add it */
      AddColumn(head, intable,CLNM_EVTPHA, CARD_COMM_PHA, "1E", TUNIT|TMIN|TMAX, UNIT_CHANN, CARD_COMM_PHYSUNIT, PHA_MIN, PHA_MAX);
      indxcol->EVTPHA=ColNameMatch(CLNM_EVTPHA, intable);
    }
  
  if((indxcol->PHA=ColNameMatch(CLNM_PHA, intable)) == -1)
    {
      headas_chat(NORMAL, "AddEventBintable: Error %s column does not exist.\n", CLNM_PHA);
      goto event_end; 
    }

  if(global.par.addcol)
    {

      if ((indxcol->PHAS=ColNameMatch(CLNM_PHAS, intable)) == -1)
	{ /* if PHAS column does not exist add it */
	  AddColumn(head, intable, CLNM_PHAS, CARD_COMM_PHAS, "3E", TUNIT|TMIN|TMAX, UNIT_CHANN, CARD_COMM_PHYSUNIT,PHA_MIN, PHA_MAX);
	  indxcol->PHAS=ColNameMatch(CLNM_PHAS, intable);
	}

      if((indxcol->ABOVE=ColNameMatch(CLNM_ABOVE, intable)) == -1)
	{
	  AddColumn(head, intable,CLNM_ABOVE,CARD_COMM_ABOVE , "1I",TNONE);    
	  indxcol->ABOVE=ColNameMatch(CLNM_ABOVE, intable);
	}


    }

  if((indxcol->GRADE=ColNameMatch(CLNM_GRADE, intable)) == -1)
    {
      AddColumn(head, intable,CLNM_GRADE, CARD_COMM_GRADE, "1I",TNULL,KWVL_GRADENULL);   
      indxcol->GRADE=ColNameMatch(CLNM_GRADE, intable);    
    }

  else
    {
      sprintf(KeyWord, "TNULL%d", (indxcol->GRADE+1));
      if (!ExistsKeyWord(head, KeyWord, NULLNULLCARD))
	AddCard(head, KeyWord, J, &null_grade, CARD_COMM_TNULL);
    }






 if (global.pd)
   {
     if((indxcol->OFFSET=ColNameMatch(CLNM_OFFSET, intable)) == -1)
       {
	 headas_chat(NORMAL, "InitBintable: Error %s column does not exist.\n", CLNM_OFFSET);
	 goto event_end; 
       }
   }

 return OK;
 
 event_end:
 if (head->first)
   ReleaseBintable(head, intable);
 
 return NOT_OK;
  
} /* InitBintable */
void PrintStatistics(void)
{
  int i;
 
  headas_chat(NORMAL, "---------------------------------------------------\n", global.maxrows);

  headas_chat(NORMAL, " Total events number: \t\t\t%d\n", global.maxrows);

  headas_chat(NORMAL, " Total saturated events number: \t%d\t\t%f\n", global.tot_sat, global.per_tot_sat);

  for (i=0; i<4; i++)
    headas_chat(NORMAL, " Total # %d grade events: \t\t%d\t\t%f \n", i, global.grade[i], global.per_grade[i]);
  
  headas_chat(NORMAL, " Total NULL grade events: \t\t%d\t\t%f \n", global.grade[4], global.per_grade[4]);

}
int DelCols(FitsFileUnit_t evunit)
{
  int colnum=0, status=OK;

  /* Check if PHAS column exists and get the number */
  if(!fits_get_colnum(evunit, CASEINSEN, CLNM_PHAS, &colnum, &status))
    if(fits_delete_col(evunit, colnum, &status))
      {
	headas_chat(NORMAL, "DelCols: Error deleting %s column.\n", CLNM_PHAS);
	return NOT_OK;
      }

  /* Check if PixsAbove column exists and get the number */
  if(!fits_get_colnum(evunit, CASEINSEN, CLNM_ABOVE, &colnum, &status))
    if(fits_delete_col(evunit, colnum, &status))
      {
	headas_chat(NORMAL, "DelCols: Error deleting %s column.\n", CLNM_ABOVE);
	return NOT_OK;
      }


  return OK;
}/*DelCols*/

int RemoveNullEvents(Bintable_t *table, unsigned *nRows, int colnum)
{
  unsigned n,ActRow;
  ActRow=0;
  for (n=0; n<*nRows; ++n) 
    if(EVEC(*table, n, colnum)!= KWVL_PHANULL) 
      if(CopyRows(table, ActRow++, n))
	{
	  headas_chat(NORMAL, "RemoveNullEvents: Error copying rows.\n");
	  return NOT_OK;
	}

  *nRows = ActRow;

  return OK;
} /*RemoveNullEvents*/

/*
 *
 *
 */
int AddStatKeywords(FitsFileUnit_t *ounit)
{
  int                status=OK, i;
  char		     KeyWord[FLEN_KEYWORD];

  GetGMTDateTime(global.date);
  /* EVENT THRESHOLD */
  if(fits_update_key(*ounit, TINT, KWNM_EVENT_TH, &global.onboardevent , CARD_COMM_EVENT_TH, &status))
    goto add_end;

  /* SPLIT THRESHOLD */
  if(fits_update_key(*ounit, TINT, KWNM_SPLIT_TH,&global.onboardsplit ,CARD_COMM_SPLIT_TH , &status))
    goto add_end;

  if(fits_update_key(*ounit, TINT, KWNM_TOTEV, &global.maxrows, CARD_COMM_TOTEV, &status))
    goto add_end;


  
  for(i=0; i<5; i++)
    {
      
      global.per_grade[i] = 100.*(double)global.grade[i]/(double)global.maxrows;
      sprintf(KeyWord, "%.2f%%",global.per_grade[i] );
      if(fits_update_key(*ounit, TSTRING, KWNM_NULLEVPER, &KeyWord,CARD_COMM_NULLEVPER , &status))
	goto add_end;
    }

  global.per_tot_sat = 100.*(double)global.tot_sat/(double)global.maxrows;
  sprintf(KeyWord, "%.2f%%",global.per_tot_sat);
  if(fits_update_key(*ounit, TSTRING, KWNM_SATEVPER, &KeyWord,CARD_COMM_SATEVPER, &status))
    goto add_end;

  if(global.hist)
    {
      sprintf(global.strhist, "File modified by '%s' (%s) at %s: performed event recostrunction for windowed timing mode. ", global.taskname, global.swxrtdas_v,global.date );
      if(fits_write_history(*ounit, global.strhist, &status))
	{
	  headas_chat(CHATTY, "%s: Error writing history block.\n", global.taskname);
	  goto add_end;
	}
      sprintf(global.strhist, "Used %s CALDB grade fits file.", global.par.gradefile);
      if(fits_write_history(*ounit, global.strhist, &status))
	{
	  headas_chat(CHATTY, "%s: Error writing history block.\n", global.taskname);
	  goto add_end;
	}
      if(global.par.addcol)
	{
	  sprintf(global.strhist, "Added  %s and %s columns.", CLNM_PHAS, CLNM_ABOVE);
	  if(fits_write_history(*ounit, global.strhist, &status))
	    {
	      headas_chat(CHATTY, "%s: Error writing history block.\n", global.taskname);
	      goto add_end;
	    }
	}
      if(global.par.delnull)
	{
	  sprintf(global.strhist, "Deleted  rows with PHA set to NULL.");
	  if(fits_write_history(*ounit, global.strhist, &status))
	    {
	      headas_chat(CHATTY, "%s: Error writing history block.\n", global.taskname);
	      goto add_end;
	    }
	  
	}

      
    }

  return OK;
 add_end:
  return NOT_OK;

}/* AddStatKeywords*/

/*
 *      GetGrades
 *
 */
int GetGrades(long extno)
{
  int            n, status=OK, jj;
  unsigned       FromRow, ReadRows, OutRows, nCols, i; 
  GradeCol_t     gradecol; 
  Bintable_t     table;                /* Bintable pointer */  
  FitsHeader_t   head;                 /* Extension pointer */
  FitsFileUnit_t gradeunit=NULL;        /* Bias file pointer */

  
  TMEMSET0( &table, Bintable_t );
  TMEMSET0( &head, FitsHeader_t );
  
  /* Open read only bias file */
  if ((gradeunit=OpenReadFitsFile(global.par.gradefile)) <= (FitsFileUnit_t )0)
    {
      headas_chat(NORMAL, "GetGrades: Error opening '%s' file\n",global.par.gradefile);
      goto get_end;
    }
  else
    headas_chat(CHATTY, "GetGrades: Info: reading '%s' file.\n", global.par.gradefile);
  
  /* Check whether it is a Swift XRT File */
  ISXRTFILE_WARNING(NORMAL, gradeunit, global.par.gradefile, global.taskname);
  
  if(extno != -1)
    {
      /* move to GRADES extension */
      if (fits_movabs_hdu(gradeunit,(int)(extno), NULL,&status))
	{ 
	  headas_chat(NORMAL,"GetGrades: Unable to find  '%d' extension  in '%s' file\n",extno, global.par.gradefile);
	  goto get_end;
	} 
    }
  else
    {
      if (fits_movnam_hdu(gradeunit,ANY_HDU,KWVL_EXTNAME_WTGRADES,0,&status))
	{ 
	  headas_chat(NORMAL,"GetGrades: Unable to find  '%s' extension  in '%s' file\n",KWVL_EXTNAME_WTGRADES, global.par.gradefile);
	  goto get_end;
	} 
    }
  
  head = RetrieveFitsHeader(gradeunit);
  
  /* Read grade bintable */
  GetBintableStructure(&head, &table, BINTAB_ROWS, 0, NULL);
  nCols=table.nColumns;
  
  /* Get columns index from name */
  if ((gradecol.GRADEID = GetColNameIndx(&table, CLNM_GRADEID)) == -1 )
    {
      headas_chat(NORMAL, "GetGrades:Error '%s' column not found in '%s' file. \n", CLNM_GRADEID, global.par.gradefile);
      goto get_end;
    }
  
  if ((gradecol.GRADE = GetColNameIndx(&table, CLNM_GRADE)) == -1 )
    {
      headas_chat(NORMAL, "GetGrades:Error '%s' column not found in '%s' file. \n", CLNM_GRADE, global.par.gradefile);
      goto get_end;
    }
  
  global.graderows=table.MaxRows;
  /* Allocate memory to storage all coefficients */
  global.gradevalues=(WTGradeRow_t *)calloc(table.MaxRows, sizeof(WTGradeRow_t));
  
  /* Read blocks of bintable rows from input bias file */
  FromRow=1; 
  ReadRows = table.nBlockRows;
  OutRows=0;
  i=0;
  while (ReadBintable(gradeunit, &table, nCols, NULL, FromRow, &ReadRows) == 0)
    {
      for (n=0; n<ReadRows; ++n)
	{
	  /* get columns value */
	  global.gradevalues[n].gradeid=IVEC(table,n,gradecol.GRADEID);
	   /* Get grades */
	  for (jj=0; jj< PHAS3_MOL; jj++)
	    global.gradevalues[n].grade[jj] = IVECVEC(table,n,gradecol.GRADE,jj);
	}
      FromRow += ReadRows;
      ReadRows = BINTAB_ROWS;
    }
  /* Free memory allocated with bintable data */
  ReleaseBintable(&head, &table);
  
  /* Close bias file */
  if (CloseFitsFile(gradeunit))
    {
      headas_chat(NORMAL,"GetGrades: Error: unable to close '%s' CALDB grades file.\n",global.par.gradefile);
      goto get_end;
    }
  
  return OK;
  
 get_end:
  
  return NOT_OK;
  
}/* GetGrades */
/*
 *	PDPUEventRec
 *
 *
 *	DESCRIPTION:
 *
 *     
 *	DOCUMENTATION:
 *        
 *
 *      FUNCTION CALL:
 *             FitsHeader_t   RetrieveFitsHeader(FitsFileUnit_t unit);
 *             void GetBintableStructure (FitsHeader_t *header,	Bintable_t *table,
 *     					  const int MaxBlockRows, const unsigned nColumns,
 *  					  const unsigned ActCols[]);
 *             int  ColNameMatch(const char *ColName, const Bintable_t *table);
 *             int headas_chat(int ,char *, ...);
 *             void AddColumn(FitsHeader_t *header, Bintable_t *table, const char *ttype,
 * 			      const char *TypeComment, const char *tform, const unsigned present, ...);
 *             void AddCard(FitsHeader_t *header, const char *KeyWord, const ValueTag_t vtag,
 *                          const void *value, const char *comment);
 *             void DeleteCard(FitsHeader_t *header, const char *KeyWord);
 *             void  GetGMTDateTime(char *);
 *             int sprintf(char *str, const char *format, ...);
 *
 *             void AddHistory(FitsHeader_t *header, const char *Comment);
 *             void EndBintableHeader(FitsHeader_t *header, const Bintable_t *table);
 *             void FinishBintableHeader(const FitsFileUnit_t unit, FitsHeader_t *header, Bintable_t *table);
 *             int  ReadBintable(const FitsFileUnit_t unit, Bintable_t *table, const unsigned nColumns,
 *				 const unsigned ActCols[], const unsigned FromRow, unsigned *nRows);
 *             int WriteFastBintable(FitsFileUnit_t unit, Bintable_t *table, unsigned nRows, BOOL last);
 *             unsigned	ReleaseBintable(FitsHeader_t *head, Bintable_t *table);
 *
 *      CHANGE HISTORY:
 *
 *        0.1.0: - BS 27/05/2003 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 *
 *      Return Status: 
 *        0: OK
 *        1: NOT_OK
 */
int PDPUEventRec(FitsFileUnit_t evunit,FitsFileUnit_t ounit)
{
  int                found=0, status=0, grade=KWVL_GRADENULL, above, qq=0, logical;
  int                OutRows=0, q=0, count=0, offset_count=0;
  int                ccdframe_next=0, ccdframe_old=0, ccdframe=0, ccdframe_count=0, offset_next=0, offset_old=0, offset=0;
  char		     KeyWord[FLEN_KEYWORD];
  BOOL               stop_flag=0, without_old=0, without_next=0, first_time=0, check=0;
  unsigned           FromRow, ReadRows, n=0, nCols=0, WriteRows=0;  
  float              pha=KWVL_PHANULL, pha_old=KWVL_PHANULL,pha_next=KWVL_PHANULL,handle[PHAS3_MOL];
  Ev2Col_t           indxcol;
  Bintable_t	     intable;
  FitsCard_t         *card, *card_2;
  FitsHeader_t	     head;


  TMEMSET0( &intable, Bintable_t );
  TMEMSET0( &head, FitsHeader_t );

  head=RetrieveFitsHeader(evunit);
	
  /* Check if PHA is already computed */
  /*  if(!ExistsKeyWord(&head, KWNM_XRTPHACO, NULLNULLCARD) || !GetLVal(&head, KWNM_XRTPHACO) ) */
/*      { */
/*        headas_chat(NORMAL, "PDPhaCorrect: Warning  column PHA is empty, PHA bias subtraction not performed .\n"); */
/*        headas_chat(CHATTY, "Use 'xrtpdcorr' task to compute bias subtracted PHA values.\n"); */
/*        goto event_end;  */
/*      } */
   
  /* Check if Events are already reconstructed  */
  if(ExistsKeyWord(&head, KWNM_XRTEVREC, &card))
    global.evtrec=card->u.LVal;
  else
    global.evtrec=FALSE;
  
  if(global.evtrec)
    {
      headas_chat(NORMAL, "PDPUEventRec: Warning event recognition is already done.\n");
      goto event_end;
    }

 /* Get event and split threshold */
  if(global.par.event < 0)
    {
      if(ExistsKeyWord(&head, KWNM_LLD, &card) && ExistsKeyWord(&head, KWNM_BIASVAL, &card_2))
	global.onboardevent=(card->u.JVal - card_2->u.JVal);
      else
	global.onboardevent=DEFAULT_LLD;
    }
  else
    global.onboardevent=global.par.event;
  
  if(global.par.split < 0)
    {
      if(ExistsKeyWord(&head, KWNM_LLD, &card) && ExistsKeyWord(&head, KWNM_BIASVAL, &card_2))
	global.onboardsplit=(card->u.JVal - card_2->u.JVal);
      else
	global.onboardsplit=DEFAULT_LLD;
    }
  else
    global.onboardsplit=global.par.split;
  

  headas_chat(NORMAL, "%s: Info using split threshold %d and event threshold %d.\n", global.taskname,global.onboardsplit,global.onboardevent);

  /* Initialize bntable with number of rows = blockn*BINTAB_ROWS */
  InitBintable(&head, &intable, &nCols, &indxcol, BINTAB_ROWS);
  

  EndBintableHeader(&head, &intable); 
 
  /* write out new header to new file */
  FinishBintableHeader(ounit, &head, &intable);
 
  FromRow = 1;
  
  global.totrows=global.maxrows;
  
  ReadRows=intable.nBlockRows;
  OutRows=0;
  
  /* Read input bintable */
  while(ReadBintable(evunit, &intable, nCols, NULL,FromRow,&ReadRows) == 0 && ReadRows!=0)
    {
      for(n=0; n<ReadRows; ++n)
	{
	  check=0;
	  stop_flag=0;
	  found=0;
	  without_next=0;
	  without_old=0;
	  
	  if ((n+2) < ReadRows && (OutRows + 2) < BINTAB_ROWS) 
	    {
	      
	      if(!global.evtrec) 
  		{ 
  		  count=n; 
  		  for (q=0; q<3; q++, count++) 
  		    EVEC(intable, count, indxcol.EVTPHA)=EVEC(intable, count, indxcol.PHA); 
		} 

	     
	     
	     ccdframe_old=JVEC(intable, n, indxcol.CCDFrame);
	     offset_old=JVEC(intable, n, indxcol.OFFSET);
	     pha_old=EVEC(intable, n, indxcol.EVTPHA);
	     
	     ccdframe=JVEC(intable, (n+1), indxcol.CCDFrame);
	     offset=JVEC(intable, (n+1), indxcol.OFFSET); 
	     pha=EVEC(intable, (n+1), indxcol.EVTPHA);
		 
	     ccdframe_next=JVEC(intable, (n+2), indxcol.CCDFrame);
	     offset_next=JVEC(intable, (n+2), indxcol.OFFSET);
	     pha_next=EVEC(intable, (n+2), indxcol.EVTPHA);
		
	    
	     /* check if pha(n-1) is already used */
	     if (!first_time)
	       {
		 check=1;
		 first_time=1;
	       }
	     else if (offset_count == LAST_OFFSET && offset_old == (FIRST_OFFSET+1) && ccdframe_old == (ccdframe_count+1))
	       check=1;
	     else if (offset_old > (offset_count + 1) && ccdframe_old == ccdframe_count)
	       check=1;


	     /*
	      * Check on CCDFrame and offset 
	      */

	     /* Check on n and n+1 elements */
	     /* ccdframe values are the same? */
	     if(ccdframe_old == ccdframe)
	       {/* offset values are consecutive? */
		 if(offset != (offset_old+1))
		   {
		     without_old=1;
		     stop_flag=1;
		   }
	       }
	     /* ccdframe and offset values are consecutive */ 
	     else if (ccdframe == (ccdframe_old+1))
	       {
		 if(offset != FIRST_OFFSET || offset_old != LAST_OFFSET)
		   {
		     stop_flag=1;
		     without_old=1;
		   }
	       }
	     else
	       stop_flag=1;

	     /* Check on (n+1) and (n+2) elements */
	     /* ccdframe values are the same? */
	     if(ccdframe_next == ccdframe)
	       {/* offset values are consecutive? */
		 if(offset != (offset_next-1))
		   {
		     stop_flag=1;
		     without_next=1;
		   }
	       }
	     /* ccdframe and offset values are consecutive */ 
	     else if (ccdframe_next == (ccdframe+1))
	       {
		 if(offset_next != FIRST_OFFSET || offset != LAST_OFFSET)
		   {			   
		     stop_flag=1;
		     without_next=1;
		   }
	       }
	     else
	       {
		 stop_flag=1;
		 without_next=1;
	       }

	     /* Check if pha_old could be the central event */
	     if ( check && pha_old >= global.onboardevent && pha_old != SAT_VAL)
	       {
		 
		 if (without_old)
		   {
		     found=1;
		     if(global.par.addcol)
		       {
			 EVECVEC(intable, n,indxcol.PHAS,0)=pha_old;
		     
			 for (q=1; q < PHAS3_MOL ; q++)
			   EVECVEC(intable, n,indxcol.PHAS,q) = KWVL_PHANULL;
			  IVEC(intable, n, indxcol.ABOVE) = 1;
		       }

		     EVEC(intable, n, indxcol.PHA) = pha_old;
		     IVEC(intable, n, indxcol.GRADE) = 0;
		    
		     global.grade[0]++;
		     
		     global.totrows--;
		     OutRows++;

		     if (offset_old != LAST_OFFSET)
		       {
			 offset_count = (offset_old+1);
			 ccdframe_count = ccdframe_old;
		       }
		     else
		       {
			 offset_count = FIRST_OFFSET;
			 ccdframe_count++;
		       }
		   }/* if (without_old) */

		 else if (pha_old >= pha)
		   {
		     found = 1;
		     handle[0]=pha_old;
		     handle[1]=KWVL_PHANULL;
		     handle[2]=pha;
		     
		     /* Compute total pha, grade and pixsabove */
		     if(faint3x1(handle, &pha, &grade, &above))
		       {
			 headas_chat(NORMAL, "PDPUEventRec3x1: Error computing PHA.\n");
			 goto event_end;
			 
		       }
		     /* Fill columns */
		     /* Central event */
		     if(global.par.addcol)
		       {
			 /* PHAS */
			 for (q=0; q < PHAS3_MOL ; q++)
			   EVECVEC(intable, n,indxcol.PHAS,q) = handle[q];
			 IVEC(intable, n, indxcol.ABOVE) = above;
		       }
		     /* PHA, GRADE and PixsAbove */
		     EVEC(intable, n, indxcol.PHA) = pha;
		     IVEC(intable, n, indxcol.GRADE) = grade;
		    
		     
		     n++;
		     SetNullEvent(intable, n, indxcol);
		    		     
		     global.totrows-=2;
		     OutRows+=2;
		     offset_count=offset;
		     ccdframe_count=ccdframe;
		   }
	       }
	     

	     /* 
	      * Is central event PHA bigger than event threshold ? 
	      */
	     if (!found && pha >= global.onboardevent && pha != SAT_VAL )
	       {
		 /* We have three consecutive values, check if pha is a local maximum */
		 if (pha >= pha_next && pha >= pha_old && !stop_flag)
		   {/* Found a central event */
		     found=1;
		     handle[0]=pha;
		     handle[1]=pha_old;
		     handle[2]=pha_next;
		     /* Compute total pha, grade and pixsabove */
		     if(faint3x1(handle, &pha, &grade, &above))
		       {
			 headas_chat(NORMAL, "PDPUEventRec3x1: Error computing PHA.\n");
			 goto event_end;
			 
		       }
		     /* Fill columns */
		 
		     SetNullEvent(intable, n, indxcol);
		
		     n++;
		     /* Central event */
		     if(global.par.addcol)
		       {
			 
			 /* PHAS */
			 for (q=0; q < PHAS3_MOL ; q++)
			   EVECVEC(intable, n,indxcol.PHAS,q) = handle[q];
			 IVEC(intable, n, indxcol.ABOVE) = above;
		       }

		     /* PHA, GRADE and PixsAbove */
		     EVEC(intable, n, indxcol.PHA) = pha;
		     IVEC(intable, n, indxcol.GRADE) = grade;

		     
		     n++;
		     
		     SetNullEvent(intable, n, indxcol);
		     

		     global.totrows-=3;
		     OutRows+=3;
		     offset_count=offset_next;
		     ccdframe_count=ccdframe_next;
		   }/* End if (pha>=pha_next... && !stop_flag)  */
		 
		 /* We have not three consecutive events, check if there is only one */
		 else if (without_next && without_old)
		   {
		     found=1;
		     
		     /* set the element 'n' to null */ 
		     SetNullEvent(intable, n, indxcol);
		    
		     n++;
		     if(global.par.addcol)
		       {
			 EVECVEC(intable, n,indxcol.PHAS,0) = pha;
			 
			 for (q=1; q < PHAS3_MOL ; q++)
			   EVECVEC(intable, n,indxcol.PHAS,q) = KWVL_PHANULL;
			 IVEC(intable, n, indxcol.ABOVE) = 1;
		       } 
		     /* PHA, GRADE and PixsAbove */
		     EVEC(intable, n, indxcol.PHA) = pha;
		     IVEC(intable, n, indxcol.GRADE) = 0;
		    
		     global.grade[0]++;
		     
		     if (offset != LAST_OFFSET)
		       {
			 offset_count = offset+1;
			 ccdframe_count=ccdframe;
		       }
		     else
		       {
			 offset_count = FIRST_OFFSET;
			 ccdframe_count=ccdframe+1;
		       }
		     
		     global.totrows-=2;
		     OutRows+=2;
		   }/* End if (without_count && without_next) */
		 
		 
	     /* We have only two consecutive events without the last, reconstructin the event */
		 else if (without_next && pha >= pha_old )
		   {
		     found=1;
		    
		     handle[0]=pha;
		     handle[1]=pha_old;
		     handle[2]=KWVL_PHANULL;
		     /* Compute total pha, grade and pixsabove */
		     if(faint3x1(handle, &pha, &grade, &above))
		       {
			 headas_chat(NORMAL, "PDPUEventRec3x1: Error computing PHA.\n");
			 goto event_end;
			 
		       }
		     /* Fill columns */
		     SetNullEvent(intable, n, indxcol);
		    

		     n++;
		     /* Central event */
		     /* PHAS */
		     if(global.par.addcol)
		       {
			 for (q=0; q < PHAS3_MOL ; q++)
			   EVECVEC(intable, n,indxcol.PHAS,q) = handle[q];
			 IVEC(intable, n, indxcol.ABOVE) = above;
		       }
		     /* PHA, GRADE and PixsAbove */
		     EVEC(intable, n, indxcol.PHA) = pha;
		     IVEC(intable, n, indxcol.GRADE) = grade;
		    
		     
		     if (offset != LAST_OFFSET)
		       {
			 offset_count = (offset+1);
			 ccdframe_count = ccdframe;
		       }
		     else
		       {
			 offset_count = FIRST_OFFSET;
			 ccdframe_count = ccdframe+1;
		       }

		     global.totrows-=2;
		     OutRows+=2;

		   }/* End if ( without_next && pha >= pha_old ) */
		 
	       }/* pha >= event threshold */
	 
	    
	     /* Set pha to NULL */
	     if(!found)
	       {
		 
		 SetNullEvent(intable, n, indxcol);
		 
		 offset_count=offset_old;
		 ccdframe_count=ccdframe_old;
		 global.totrows--;
		 OutRows++;
	       }
	     
	   }/* n+1 < ReadRows */
	 else 
	   {
	     if (global.totrows < 3)
	       {
		 for (; global.totrows > 0; global.totrows--)
		   {

		     if (!global.evtrec)
		       EVEC(intable, n, indxcol.EVTPHA)=EVEC(intable, n, indxcol.PHA);
		     SetNullEvent(intable, n, indxcol);
		     n++;
		     OutRows++;
		   }
	       }
	     else 
  	       ReadRows=n; 
	   }
	  
	 
	 	 
       }/* for(n) */
      WriteRows=OutRows;
      if(global.par.delnull)
	{
	  
	  if(RemoveNullEvents( &intable, &WriteRows, indxcol.PHA))
	    {  
	      headas_chat(NORMAL, "PDPUEventRec: Error removing NULL events.\n");
	      goto event_end;
	    }
	}
      

     WriteFastBintable(ounit, &intable, WriteRows, FALSE);
     FromRow += OutRows;
     OutRows=0;
     ReadRows = BINTAB_ROWS;
      
   }/* while ReadBintable */ 
  if (OutRows > 0 )
    {
      WriteRows=OutRows;
      
      if(global.par.delnull)
	{
	  if(RemoveNullEvents( &intable, &WriteRows, indxcol.PHA))
	    {  
	      headas_chat(NORMAL, "PDPUEventRec: Error removing NULL events.\n");
	      goto event_end;
	    }
	}
      
      WriteFastBintable(ounit, &intable, WriteRows, TRUE);
    }
  else
    WriteFastBintable(ounit, &intable, 0, TRUE);

  if(AddStatKeywords(&ounit))
    {
      headas_chat(NORMAL, "WTEventRec: Error adding keywords.\n");
      goto event_end;
    }

 /*  ReleaseBintable(&head, &intable);  */

  if (!global.evtrec)
    {
      logical=TRUE;
      if(fits_update_key(ounit, TLOGICAL, KWNM_XRTEVREC, &logical, CARD_COMM_XRTEVREC, &status))
	goto event_end;
    }
  /* Update column comment */
  sprintf(KeyWord, "TTYPE%d", (indxcol.PHA+1));
  if (fits_modify_comment(ounit, KeyWord, "Photon reconstructed PHA", &status))
    {
      headas_chat(NORMAL, "Error updating comment for %s column.\n", CLNM_PHA);
      goto event_end;
    }
  return OK;
  
 event_end:
  if (head.first)
    ReleaseBintable(&head, &intable);
  
  return NOT_OK;
  
} /* PDPUEventRec */
