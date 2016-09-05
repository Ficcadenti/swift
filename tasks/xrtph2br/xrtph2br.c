/*
 * 
 *	xrtph2br.c:
 *
 *	INVOCATION:
 *
 *		xrtph2pr [parameter=value ...]
 *
 *	DESCRIPTION:
 *                Routine to assign grades to XRT Photon counting
 *                mode data. Based on ASCA 'faint'
 *	DOCUMENTATION:
 *
 *	CHANGE HISTORY:
 *        0.1.0 - BS 01/03/02 - First version
 *        0.1.1 -    02/07/02 - Added GetSWXRTDASVersion call
 *                            - Changed strcmp with strcasecmp to ignore
 *                              case of characters when needed
 *                            - Modified return values when output files exist
 *                              and 'clobber' = no
 *                            - Added 'DeriveFileName' to handle compressed
 *                              file names
 *                            - Added management return values from function
 *                              'remove'.
 *                            - Changed PHA column datatype from "1I" to "1J"
 *        0.1.2 -    12/09/02 - Bug fixed for Alpha platform
 *        0.1.3 - FT 16/10/02 - Added 'B' handle into 'AddEventBintable'
 *        0.1.4 - BS 31/10/02 - Minor changes
 *                            - Modified to read input files name 
 *                              from ASCII file
 *                            - Deleted "aboveflag" and "maxgrade"
 *                              parameters
 *                            - Deleted screening for grade
 *                            - Optimized "AddEventBintable" routine
 *                            - Modified to overwrite input file
 *       0.1.5 - BS 25/02/03  - Added chosen to use user or on-board split threshold
 *                            - Added checks on XRTPHA keyword
 *       0.1.6 -    07/07/03  - Bug fixed on 'xrtph2br_getpar' failure           
 *       0.1.7 -    10/07/03  - Added routine to compute grade using CALDB grades file
 *                              with XMM patterns definition
 *       0.1.8 -    15/07/03  - Modified to use new prototype of 'CalGetFileName'
 *       0.1.9 -    04/08/03  - Bug fixed
 *       
 *      0.1.10 -    10/09/03  - Added compute events percentage
 *      0.1.11 -    23/10/03  - Update TLMIN and TLMAX for GRADE column
 *                            - Added DS keyword
 *      0.1.12 -    03/11/03  - Replaced call to 'headas_parstamp()' with 'HDpar_stamp()'
 *      0.1.13 -    19/11/03  - Used default value for GRADE TLMIN/TLMAX when needed 
 *      0.1.14 -    11/12/03  - Added handling of amplifiers 
 *
 *      NOTE:
 *        Based on the CLASSIFY routine
 *
 *	AUTHORS:
 *
 *       ISAC Italian Swift Archive Center (FRASCATI)
 */
#define TOOLSUB xrtph2br  /* headas_main() requires that TOOLSUB be defined first */

/********************************/
/*        header files          */
/********************************/
#include "headas_main.c"
#include "xrtph2br.h" 


/********************************/
/*         definitions          */
/********************************/

#define XRTPH2BR_C
#define XRTPH2BR_VERSION       "0.1.14"
#define PRG_NAME               "xrtph2br"

/********************************/
/*           globals            */
/********************************/

Global_t global;



/*
 *	xrtph2br_getpar
 *
 *
 *	DESCRIPTION:
 *                 Routine to read input parameters from 
 *                 xrtph2br.par    
 *     
 *
 *	DOCUMENTATION:
 *        
 *
 *      FUNCTION CALL:
 *           int PILGetFname(char *name, char *result); 
 * 	     int PILGetInt(char *name, int *result);
 *           int PILGetBool(char *name, int *result);
 *           int headas_chat(int , char *, ...);
 *           void WriteInfoParams(void);
 *           void get_history(int *);
 *	     void ReadInFileNamesFile(char *);
 *           char * strcpy(char *, char *);
 *
 *      CHANGE HISTORY:
 *       
 *        0.1.0: - BS 08/02/2002 - First version
 *        0.1.2: - BS 09/07/2003 - Added 'gradefile' and 'ascagrade' 
 *                                 parameter
 *                                 
 *
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 */
int xrtph2br_getpar()
{
  /*
   *  Get File name Parameters
   */
  
  /* Input Photon-Counting Event List Files Name */
  if(PILGetFname(PAR_INFILE, global.par.infiles)) 
    {
      headas_chat(NORMAL, "xrtph2br_getpar: Error getting '%s' parameter.\n",PAR_INFILE);
      goto Error;
    }
  
  /* Output Event List File Name */
  if(PILGetFname(PAR_OUTFILE, global.par.outfile)) 
    {
      headas_chat(NORMAL, "xrtph2br_getpar: Error getting '%s' parameter.\n",PAR_OUTFILE);
      goto Error;	
    }

  /* Compute ASCA grades ? */
  if(PILGetBool(PAR_ASCAGRADE, &global.par.ascagrade))
    {
      headas_chat(NORMAL,"xrtph2br_getpar: Error getting '%s'parameter.\n", PAR_ASCAGRADE);
      goto Error;
    }
 
  /* Grade File Name */
  if(PILGetFname(PAR_GRADEFILE, global.par.gradefile)) 
    {
      headas_chat(NORMAL, "xrtph2br_getpar: Error getting '%s' parameter.\n",PAR_GRADEFILE);
      goto Error;	
    }
  
  /*
   *  Get Integer Parameters
   */

  /* Split Threshold Level */
  if(PILGetInt(PAR_SPLIT, &global.par.split)) 
    {
      headas_chat(NORMAL, "xrtph2br_getpar: Error getting '%s' parameter.\n",PAR_SPLIT);
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
  
} /* xrtph2br_getpar */

/*
 *	xrtph2br_work
 *
 *
 *	DESCRIPTION:
 *            This routine reads in the input FITS file and passes each record
 *            to CLASSIFY, the routine which does the actual grade assignments
 *            to the photon counting mode data.
 * 		  
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
 *           int HDpar_stamp(fitsfile *fptr, int hdunum)
 *           int ChecksumCalc(FitsFileUnit_t unit);
 * 
 *	
 *      CHANGE HISTORY:
 *
 *        0.1.0: - BS 08/02/2002 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 *
 *      Return Status: 
 *        0: OK
 *        1: NOT_OK
 */
int xrtph2br_work()
{
  int            status = OK, inExt, outExt, evExt, overwrite=0, logical;   
  long           extno;
  char           stem[10] , ext[MAXEXT_LEN] ;
  BOOL           first=1;
  pid_t          tmp;
  FitsHeader_t   head;
  FitsFileUnit_t inunit=NULL, outunit=NULL;              /* Input and Output fits file pointer */ 

  /* Get input file name */
  global.infile = strtok(global.par.infiles, " \n\t;:");

  while (global.infile != CPNULL) {
    
    extno=-1;
    /* Open readonly input event file */
    if ((inunit=OpenReadFitsFile(global.infile)) <= (FitsFileUnit_t )0)
      {
	headas_chat(NORMAL,"xrtph2br_work: ... Error opening '%s' FITS file.\n", global.infile);
	goto Error;
      }
    
    /* Check readout mode */
    
    /* Move in events extension in input file */
    if (fits_movnam_hdu(inunit, ANY_HDU,KWVL_EXTNAME_EVT, 0, &status))
      { 
	headas_chat(CHATTY,"xrtph2br_work: Error: unable to find  '%s' extension  in '%s' file.\n",KWVL_EXTNAME_EVT, global.infile);      
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


    /* Check readout mode of input events file */
    if(!(KeyWordMatch(inunit, KWNM_DATAMODE, S, KWVL_DATAMODE_PH, NULL)))
      {
	headas_chat(NORMAL, "xrtph2br_work: Warning readout mode in %s file not allowed.\n", global.infile);
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

    /* Retrieve datamode from input events file */    
    head=RetrieveFitsHeader(inunit);    
    
    /* Get Event ext number */
    if (!fits_get_hdu_num(inunit, &evExt))
      {
	headas_chat(CHATTY,"xrtph2br_work: Error unable to find  '%s' extension  in '%s' file.\n",KWVL_EXTNAME_EVT, global.infile);      
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
		headas_chat(NORMAL, "%s: Error: '%s' file exists,\n",global.taskname, global.par.outfile);
		headas_chat(NORMAL, "%s: Error: please set the parameter clobber to yes to overwrite it.\n",global.taskname);
		goto Error;
	      }
	    else
	      {
		headas_chat(NORMAL, "%s: Info: parameter 'clobber' set,\n",global.taskname);
		headas_chat(NORMAL, "%s: Info: the '%s' file will be overwritten\n", global.taskname, global.par.outfile);
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


    if(first)
      {
	
	/********************************
	 *     Get CALDB grades file    *
	 ********************************/
	
	if ( !strcasecmp(global.par.gradefile,DF_CALDB) )
	  {
	    if (CalGetFileName(HD_MAXRET, DF_NOW, DF_NOW, DF_NOW, DF_NOW, KWVL_GRADE_DSET, global.par.gradefile, "datamode.eq.photon", &extno ))
	      {
		headas_chat(NORMAL, "%s: Error getting CALDB grade file.\n", global.taskname);
		goto Error;
	      }
	    extno++;
	  }
	headas_chat(NORMAL, "%s: Info: Processing '%s' file.\n", global.taskname, global.par.gradefile);
	if (GetGrades(extno)) 
	  {
	    headas_chat(CHATTY, "%s: Error processing grade file.\n", global.taskname);
	    goto Error;
	  }
	first=0;
      }
    
    /* Create output file */
      if ((outunit = OpenWriteFitsFile(global.tmpevtfilename)) <= (FitsFileUnit_t )0)
	{
	  headas_chat(NORMAL, "xrtph2br_work: Error creating '%s' fits file.\n", global.tmpevtfilename);
	  goto Error;
	}
      
    /* Get number of hdus in input events file */
    if (fits_get_num_hdus(inunit, &inExt, &status))
      {
	headas_chat(CHATTY, "xrtph2br_work: Error getting hdus num in '%s' file.\n", global.infile);
	goto Error;
      }
    
    /* Copy all extension before event extension  from input to output file */
    outExt=1;
    
    while(outExt<evExt && status == OK)
      {
	if(fits_movabs_hdu( inunit, outExt, NULL, &status ))
	  headas_chat(CHATTY, "xrtph2br_work: Error: unable to move in hdu %d in  '%s' file. \n", outExt, global.infile);
	if(fits_copy_hdu( inunit, outunit, 0, &status ))
	  headas_chat(CHATTY, "xrtph2br_work: Error: unable to copy hdu %d from input to output file. \n", outExt);
	
	outExt++;
      }
    
    if (status) {
      headas_chat(CHATTY,"xrtph2br_work: Error creating output extentions.\n");
      goto Error;
    }
    
    /* make sure get specified header by using absolute location */
    if(fits_movabs_hdu( inunit, evExt, NULL, &status ))
      {
	headas_chat(CHATTY,"xrtph2br_work: Error: Unable to move to requested extension.\n");
	goto Error;
      }


    if( global.par.split < 0)
      {
	if(fits_read_key(inunit, TINT, "SPLITTHR", &global.par.split,  NULL, &status))
	  {
	    headas_chat(NORMAL, "%s: Error split parameter is negative and there is not 'SPLITTHR' keyword in event file.\n", global.taskname);
	    goto Error;
	  }  
      else
	headas_chat(CHATTY, "%s: Info: Using split threshold on-board:  %d.\n", global.taskname, global.par.split);
      } 
    else
      headas_chat(CHATTY, "%s: Info: Using user split threshold: %d.\n", global.taskname, global.par.split);
    
    headas_chat(NORMAL, "%s: Info: Processing '%s' file.\n", global.taskname, global.infile);

    /* Add event extension */
    if ((AddEventBintable(inunit, outunit)))
      {
	headas_chat(CHATTY, "xrtph2br_work: Error writing events extension in '%s' file.\n", global.tmpevtfilename);
	goto Error;
      }
    
    logical=TRUE;
    if(fits_update_key(outunit, TLOGICAL, KWNM_XRTPHA, &logical, CARD_COMM_XRTPHA, &status))
      goto Error;
    
    outExt++;
    
    /* copy any extension after the extension to be operated on */
    while ( status == OK && outExt <= inExt) 
      {
	if(fits_movabs_hdu( inunit, outExt, NULL, &status ))
	  {
	    headas_chat(CHATTY, "xrtph2br_work: Error moving in  events extension in '%s' file.\n", global.infile);
	    goto Error;
	  }
	if(fits_copy_hdu ( inunit, outunit, 0, &status ))
	  {
	    headas_chat(CHATTY, "xrtph2br_work: Error copying extensions in '%s' file.\n", global.tmpevtfilename);
	    goto Error;
	  }
	
	outExt++;
      }
    /* Add history if parameter history set */
    if(HDpar_stamp(outunit, evExt, &status))
      {
	headas_chat(CHATTY, "%s: Error updating history blocks.\n", global.taskname);
	goto Error;
      }
    
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
   
    
    /* Get next filename */    
  skip_file:
    status=0;
    strcpy(global.par.outfile, "NONE");
    global.infile += strlen(global.infile)+1;
    global.infile = strtok(global.infile, " \n\t;:");
    
  }


  headas_chat(MUTE, "%s: ... Exit with success.\n", global.taskname);
  
  free(global.gradevalues);
 
  return OK;

 Error:
  return NOT_OK;
} /* xrtph2br_work */
/*
 *	AddEventBintable
 *
 *
 *	DESCRIPTION:
 *           Routine to compute GRADE, PHA, PixsAbove values and put them
 *           into EVENTS bintable. 
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
 *             external faint_v50_( int *cf_handle, int cf_split, int *cf_sumph,
 *                                int  *cf_type, int *cf_above );
 *             int WriteFastBintable(FitsFileUnit_t unit, Bintable_t *table, unsigned nRows, BOOL last);
 *             unsigned	ReleaseBintable(FitsHeader_t *head, Bintable_t *table);
 *
 *      CHANGE HISTORY:
 *
 *        0.1.0: - BS 08/02/2002 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 *
 *      Return Status: 
 *        0: OK
 *        1: NOT_OK
 */
int AddEventBintable(FitsFileUnit_t evunit,FitsFileUnit_t ounit)
{
  int                jj, ii, qfancy = 0, pha, grade, above, handle[PHAS_MOL], status=OK, totrows, amp=0, tmp=0;
  char		     KeyWord[FLEN_KEYWORD];
  long               split_old, split;
  float              echo= 0;
  double             per_grade[33], per_grade_asca[8], per_sat_event, per_sat_asca;
  BOOL               found=0;
  unsigned           FromRow, ReadRows, n,nCols, OutRows=0;  
  Ev2Col_t           indxcol;
  FitsCard_t         *card;
  Bintable_t	     outtable;
  FitsHeader_t	     head;

 
  TMEMSET0( &outtable, Bintable_t );
  TMEMSET0( &head, FitsHeader_t );
 

  head=RetrieveFitsHeader(evunit);
  
  /* Check if PHA is already computed */
  if(ExistsKeyWord(&head, KWNM_XRTPHA, &card))
    if(card->u.LVal)
      {
	if(ExistsKeyWord(&head, KWNM_SPLIT_TH, &card))
	  {
	    split_old=card->u.JVal;
	    headas_chat(NORMAL, "AddEventBintable: Warning PHA is already computed with split threshold = %d, the column will be overwritten.\n", split_old);
	  }
      }
 
  GetBintableStructure(&head, &outtable, BINTAB_ROWS, 0, NULL);
  nCols=outtable.nColumns;
  totrows=outtable.MaxRows;  
  /* Get cols index from name */
  if ((indxcol.Amp=ColNameMatch(CLNM_Amp, &outtable)) == -1)
    {
      headas_chat(NORMAL, "AddEventBintable: Error '%s' column does not exist in '%s' file. \n", CLNM_Amp, global.par.outfile);
      goto event_end;
    }
  if ((indxcol.PHAS=ColNameMatch(CLNM_PHAS, &outtable)) == -1)
    {
      headas_chat(NORMAL, "AddEventBintable: Error '%s' column does not exist in '%s' file. \n", CLNM_PHAS, global.par.outfile);
      goto event_end;
    }
  
  /* Add new columns if needed */
  if((indxcol.PHA=ColNameMatch(CLNM_PHA, &outtable)) == -1)
    {
      AddColumn(&head, &outtable, CLNM_PHA, CARD_COMM_PHA, "1J", TUNIT, UNIT_CHANN, CARD_COMM_PHYSUNIT);
      indxcol.PHA=ColNameMatch(CLNM_PHA, &outtable);
    }

  if((indxcol.GRADE=ColNameMatch(CLNM_GRADE, &outtable)) == -1)
    {
      AddColumn(&head, &outtable,CLNM_GRADE, CARD_COMM_GRADE, "1I",TNONE);    
      indxcol.GRADE=ColNameMatch(CLNM_GRADE, &outtable);    
    }

  if(global.grademin!=-1)
    global.grademin=PHO_GRADE_MIN;
  sprintf(KeyWord, "TLMIN%d", (indxcol.GRADE+1)); 
  if(ExistsKeyWord(&head, KeyWord, NULL))
    DeleteCard(&head, KeyWord);
  
  AddCard(&head, KeyWord, J, &global.grademin, CARD_COMM_TLMIN);
  
  if(global.grademax!=-1)
    global.grademax=PHO_GRADE_MAX;
  sprintf(KeyWord, "TLMAX%d", (indxcol.GRADE+1));
  if(ExistsKeyWord(&head, KeyWord, NULL))
    DeleteCard(&head, KeyWord); 
  AddCard(&head, KeyWord, J, &global.grademax, CARD_COMM_TLMAX);
  


  if((indxcol.ABOVE=ColNameMatch(CLNM_ABOVE, &outtable)) == -1)
    {
      AddColumn(&head, &outtable,CLNM_ABOVE,CARD_COMM_ABOVE , "1I",TNONE);    
      indxcol.ABOVE=ColNameMatch(CLNM_ABOVE, &outtable);
    }

  if(global.par.ascagrade)
    {
      if((indxcol.ASCAGRADE=ColNameMatch("ASCAGRADE", &outtable)) == -1)
	{
	  AddColumn(&head, &outtable,"ASCAGRADE", CARD_COMM_GRADE, "1I",TNONE);    
	  indxcol.ASCAGRADE=ColNameMatch("ASCAGRADE", &outtable);    
	}
      if((indxcol.ASCAPHA=ColNameMatch("ASCAPHA", &outtable)) == -1)
	{
	  AddColumn(&head, &outtable, "ASCAPHA", CARD_COMM_PHA, "1J", TUNIT, UNIT_CHANN, CARD_COMM_PHYSUNIT);
	  indxcol.ASCAPHA=ColNameMatch("ASCAPHA", &outtable);
	}
      /* Cast for faintv50 */
      split=(long)global.par.split;
    }

  /* Add history */
  GetGMTDateTime(global.date);
 if(global.hist)
    {
      sprintf(global.strhist, "File modified by '%s' (%s) at %s: calcolated PHA from PHAS value. ", global.taskname, global.swxrtdas_v,global.date );
      AddHistory(&head, global.strhist);
    }

  /* Add or update DSVAL1 keywords */
 sprintf(KeyWord, "%d:%d",(int) global.grademin,(int) global.grademax); 
 
 ManageDSKeywords(&head, "GRADE", "I", KeyWord); 


 EndBintableHeader(&head, &outtable); 
 
  /* write out new header to new file */
  FinishBintableHeader(ounit, &head, &outtable);
  
  FromRow = 1;
  ReadRows=outtable.nBlockRows;
  OutRows = 0;

  /* Check on PHAS datatype */
  if (outtable.Multiplicity[indxcol.PHAS] != 9)
    {
      headas_chat(NORMAL,"AddEventBintable: Error wrong PHAS multiplicity.\n"); 
      goto event_end;
    }

  for(jj=0; jj<33;jj++)
    global.grade_tot[jj]=0;

  for(jj=0; jj<8;jj++)
    global.grade_asca[jj]=0;

  global.sat_event=0;
  global.sat_asca=0;

  /* Read input bintable */
  while(ReadBintable(evunit, &outtable, nCols, NULL,FromRow,&ReadRows) == 0 )
    {
      for(n=0; n<ReadRows; ++n)
	{
	  amp=BVEC(outtable, n, indxcol.Amp);
	  if(global.par.ascagrade)
	    {
	      /* Compute PHA and GRADES with ASCA routine */
	      /* Get phas values */
	      for (jj=0; jj< PHAS_MOL; jj++)
		handle[jj] = IVECVEC(outtable,n,indxcol.PHAS,jj);
	  
	      if(amp == 2)
		{
		  tmp=handle[1];
		  handle[1]=handle[3];
		  handle[3]=tmp;

		  tmp=handle[4];
		  handle[4]=handle[5];
		  handle[5]=tmp;

		  tmp=handle[6];
		  handle[6]=handle[8];
		  handle[8]=tmp;
		}
	      /* Calculate grade and pha */
	      faint_v50_(handle, &split, &echo, &qfancy, &pha, &grade,&above);
	      /* Put values in table */
	      
	      IVEC(outtable, n, indxcol.ASCAGRADE) = grade;
	      JVEC(outtable, n, indxcol.ASCAPHA) = pha;
	      if(pha >= PHA_MAX)
		global.sat_asca++;
	      global.grade_asca[grade]++;
	    }
	  
	  pha=0;
	  above=0;
	  /* Assign XMM patterns */
	  for (jj=0; jj< PHAS_MOL; jj++)
	    {
	      if(IVECVEC(outtable,n,indxcol.PHAS,jj) >= global.par.split)
		{
		  above++;
		  pha+=IVECVEC(outtable,n,indxcol.PHAS,jj);
		  handle[jj]=1;
		}
	      else
		handle[jj]=2;
	    }
	  
	  found=0;
	  grade=KWVL_PCGRADENULL;
	  for(ii=0; ii<global.maxrows && grade==KWVL_PCGRADENULL; ii++)
	    {
	      if(amp == global.gradevalues[ii].amp)
		{
		  for(jj=0; jj<PHAS_MOL && found==0; jj++) 
		    if((handle[jj]!=global.gradevalues[ii].grade[jj]) && (global.gradevalues[ii].grade[jj]))
		      found=1;
		  
		  if(!found)
		    grade=global.gradevalues[ii].gradeid;
		  else
		    found=0;
		}
	    }
	  
	  IVEC(outtable, n, indxcol.GRADE) = grade;
	  JVEC(outtable, n, indxcol.PHA) = pha;
	  if(pha>=PHA_MAX)
	    global.sat_event++;
	  IVEC(outtable, n, indxcol.ABOVE) = above;
		
	  if(grade == KWVL_PCGRADENULL)
	    global.grade_tot[32]++;
	  else
	    global.grade_tot[grade]++;

	  if(++OutRows>=BINTAB_ROWS)
	    {
	      WriteFastBintable(ounit, &outtable, OutRows, FALSE);
	      OutRows=0;
	    }
	  
	}
      
      FromRow += ReadRows;
      ReadRows = BINTAB_ROWS;
    }/* while */ 
  
  WriteFastBintable(ounit, &outtable, OutRows, TRUE);
  ReleaseBintable(&head, &outtable);
  
  /* Write grade events statistics */
  headas_chat(NORMAL, "\t\t XMM GRADES\n");
  headas_chat(NORMAL, "# of total events: %d\n",totrows);

  per_sat_event = 100.*(double)global.sat_event/(double)(totrows);
  headas_chat(NORMAL, "# of saturated events: %d\t\t\t\t%f\n", global.sat_event, per_sat_event);
  
  for(jj=0; jj<33;jj++)
    {
     per_grade[jj] = 100.*(double)global.grade_tot[jj]/(double)(totrows);
    }


  for(jj=0; jj<33;jj++)
    {
      headas_chat(NORMAL, "# of grade %d events: %d\t\t\t\t%f\n", jj, global.grade_tot[jj], per_grade[jj]);
    }

  
  if(global.par.ascagrade)
    {
     
      
      for(jj=0; jj<8;jj++)
	{
	  per_grade_asca[jj] = 100.*(double)global.grade_asca[jj]/(double)(totrows);
	}
      headas_chat(NORMAL, "\t\t ASCA GRADES\n");


      headas_chat(NORMAL, "# of total events: %d\n",totrows);
 
      per_sat_asca = 100.*(double)global.sat_asca/(double)(totrows);
      headas_chat(NORMAL, "# of saturated events: %d\t\t\t\t%f\n", global.sat_asca, per_sat_asca);
      
      for(jj=0; jj<8;jj++)
	{
	  headas_chat(NORMAL, "# of grade %d events: %d\t\t\t\t%f\n", jj, global.grade_asca[jj],per_grade_asca[jj]);
	}
    }

  /* Add or update SPLIT_TH keywords */
  if(fits_update_key(ounit, TINT, KWNM_SPLIT_TH, &global.par.split, CARD_COMM_SPLIT_TH, &status))
    goto event_end;


  

  return OK;

 event_end:
  if (head.first)
    ReleaseBintable(&head, &outtable);
  
  return NOT_OK;
  
} /* AddEventBintable */

/*
 *	xrtph2br
 *
 *
 *	DESCRIPTION:
 *        
 *                Routine to assign grades to faint mode data
 * 		  Based on the CLASSIFY program
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
 *             void xrtph2br_getpar(void);
 *             void WriteInfoParams(void); 
 * 	       void xrtph2br_work(void);
 *
 *          
 *	CHANGE HISTORY:
 *        0.1.0: - BS 08/02/2002 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 */
int xrtph2br()
{
  /* set HEADAS globals */
  set_toolname(PRG_NAME);
  set_toolversion(XRTPH2BR_VERSION);

  get_toolnamev(global.taskname);
  
  GetSWXRTDASVersion(global.swxrtdas_v);

  /* Get parameter values */ 
  if ( xrtph2br_getpar() == OK ) {
    
    if ( xrtph2br_work() ) {
      headas_chat(MUTE, "%s: ... Exit with Error.\n", global.taskname);
      goto ph2br_end;
    }
  }
  else
    goto ph2br_end;
  
  return OK;

 ph2br_end:
  if (FileExists(global.tmpevtfilename))
    remove (global.tmpevtfilename);
  return NOT_OK;


} /* xrtph2br */
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
  headas_chat(NORMAL,"Name of GRADE  file              :'%s'\n",global.par.gradefile);

  headas_chat(NORMAL,"Split Threshold                  : %d\n",global.par.split);
  
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
 *      GetGrades
 *
 */
int GetGrades(long extno)
{
  int            n, status=OK, jj;
  char		 KeyWord[FLEN_KEYWORD];
  unsigned       FromRow, ReadRows, OutRows, nCols, i; 
  GradeCol_t     gradecol; 
  Bintable_t     table;                /* Bintable pointer */  
  FitsHeader_t   head;                 /* Extension pointer */
  FitsCard_t     *card;
  FitsFileUnit_t gradeunit=NULL;        /* Bias file pointer */

  
  TMEMSET0( &table, Bintable_t );
  TMEMSET0( &head, FitsHeader_t );
  
  global.grademin=-1;
  global.grademax=-1;

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
      if (fits_movnam_hdu(gradeunit,ANY_HDU,KWVL_EXTNAME_PCGRADES,0,&status))
	{ 
	  headas_chat(NORMAL,"GetGrades: Unable to find  '%s' extension  in '%s' file\n",KWVL_EXTNAME_PCGRADES, global.par.gradefile);
	  goto get_end;
	} 
    }
  
  head = RetrieveFitsHeader(gradeunit);
  
  /* Read grade bintable */
  GetBintableStructure(&head, &table, BINTAB_ROWS, 0, NULL);
  nCols=table.nColumns;
  
  /* Get columns index from name */
  if ((gradecol.Amp = GetColNameIndx(&table, CLNM_Amp)) == -1 )
    {
      headas_chat(NORMAL, "GetGrades:Error '%s' column not found in '%s' file. \n", CLNM_Amp, global.par.gradefile);
      goto get_end;
    }

  if ((gradecol.GRADEID = GetColNameIndx(&table, CLNM_GRADEID)) == -1 )
    {
      headas_chat(NORMAL, "GetGrades:Error '%s' column not found in '%s' file. \n", CLNM_GRADEID, global.par.gradefile);
      goto get_end;
    }
  
  sprintf(KeyWord, "TLMIN%d", (gradecol.GRADEID+1));
  if (ExistsKeyWord(&head, KeyWord, &card))
    global.grademin=card->u.JVal;
  else
    headas_chat(NORMAL, "Warning GRADE tlmin value not found in grade caldb file.\n"); 

  sprintf(KeyWord, "TLMAX%d", (gradecol.GRADEID+1));
  if (ExistsKeyWord(&head, KeyWord, &card))
    global.grademax=card->u.JVal;
  else
    headas_chat(NORMAL, "Warning GRADE tlmax value not found in grade caldb file.\n"); 

  if ((gradecol.GRADE = GetColNameIndx(&table, CLNM_GRADE)) == -1 )
    {
      headas_chat(NORMAL, "GetGrades:Error '%s' column not found in '%s' file. \n", CLNM_GRADEID, global.par.gradefile);
      goto get_end;
    }
  
  global.maxrows=table.MaxRows;
  /* Allocate memory to storage all coefficients */
  global.gradevalues=(GradeRow_t *)calloc(table.MaxRows, sizeof(GradeRow_t));
  
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
	  global.gradevalues[n].amp=BVEC(table,n,gradecol.Amp);
	  global.gradevalues[n].gradeid=IVEC(table,n,gradecol.GRADEID);
	   /* Get grades */
	  for (jj=0; jj< PHAS_MOL; jj++)
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


