/*
 * 
 *	xrtimage2cnt.c:
 *
 *	INVOCATION:
 *
 *		xrtimage2cnt [parameter=value ...]
 *
 *	DESCRIPTION:
 *           Task to create count image    
 *	DOCUMENTATION:
 *
 *	CHANGE HISTORY:
 *        0.1.0 - BS 27/10/2003 - First version
 *        0.1.1 -    03/11/2003 - Replaced call to 'headas_parstamp()' 
 *                                with 'HDpar_stamp()'
 *        0.1.2 -    25/11/2003 - Bug fixed
 *        0.1.3 -    07/01/2004 - If coordinate system is RAW or DET exit with error
 *        0.1.4 -    26/03/2004 - Deleted input files list handling
 *
 *
 *      NOTE:
 *       
 *
 *	AUTHORS:
 *
 *       ISAC Italian Swift Archive Center (FRASCATI)
 */

#define TOOLSUB xrtimage2cnt  /* headas_main() requires that TOOLSUB be defined first */


/********************************/
/*        header files          */
/********************************/
#include "headas_main.c"
#include "xrtimage2cnt.h" 

/********************************/
/*         definitions          */
/********************************/

#define XRTIMAGE2CNT_C
#define XRTIMAGE2CNT_VERSION       "0.1.4"
#define PRG_NAME                   "xrtimage2cnt"

/********************************/
/*           globals            */
/********************************/

Global_t global;

/*
 *	xrtimage2cnt_getpar
 *
 *
 *	DESCRIPTION:
 *                 Routine to read input parameters from 
 *                 xrtimage2cnt.par    
 *     
 *
 *	DOCUMENTATION:
 *        
 *
 *      FUNCTION CALL:
 *           int PILGetFname(char *name, char *result); 
 * 	     int PILGetInt(char *name, int *result);  
 *           int headas_chat(int , char *, ...);
 *           void xrtimage2cnt_info(void);
 *           void get_history(int *);
 *	     void ReadInFileNamesFile(char *);
 *           char * strcpy(char *, char *);
 *
 *      CHANGE HISTORY:
 *       
 *        0.1.0: - BS 27/10/2003 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 */
int xrtimage2cnt_getpar()
{
  /*
   *  Get File name Parameters
   */
  
  /* Input IMAGING Files Name */
  if(PILGetFname(PAR_INFILE, global.par.infile)) 
    {
      headas_chat(NORMAL, "xrtimage2cnt_getpar: Error: getting '%s' parameter.\n",PAR_INFILE);
      goto Error;
    }
  
  /* Output IMAGING File Name */
  if(PILGetFname(PAR_OUTFILE, global.par.outfile)) 
    {
      headas_chat(NORMAL, "xrtimage2cnt_getpar: Error: getting '%s' parameter.\n",PAR_OUTFILE);
      goto Error;	
    }
  
  get_history(&global.hist);
  xrtimage2cnt_info();

  return OK;

 Error:
  return NOT_OK;
  
} /* xrtimage2cnt_getpar */

/*
 *
 *	CountImage
 *
 *
 *	DESCRIPTION:
 *                 Routine to process images. 
 *                
 *    
 *     
 *
 *	DOCUMENTATION:
 *        
 *
 *      FUNCTION CALL:
 *
 *           FitsHeader_t	RetrieveFitsHeader(FitsFileUnit_t unit);  
 *	     BOOL               ExistsKeyWord(const FitsHeader_t *header, const char *KeyWord,
 * 					      FitsCard_t **card);
 *           int                headas_chat(int , char *, ...);
 *           int                InitImagingVars ( FitsHeader_t );
 *           void		**ReadImage(FitsFileUnit_t unit,ValueTag_t type, int NullValue,
 *                                          unsigned xmin,unsigned xmax,unsigned ymin,unsigned ymax);
 *           int                AddBadPix(int x, int y, BadPixel_t pix, BadPixel_t *bp_table[CCD_PIXS][CCD_ROWS]);
 *           int                fits_write_img_lng(outunit, 0, nElements*(j-global.im.ymin)+1, nElements,		
 *			                           &image[j][0], &status);
 *	
 *           void               xrtimage2cnt_info(void);
 *
 *
 *
 *
 *      CHANGE HISTORY:
 *       
 *        0.1.0: - BS 27/10/2003 - First version
 *
 */

int CountImage(FitsFileUnit_t inunit, int ext)
{
  int            i, j;
  long           **image;
  double         sens=0.0000001;
  FitsHeader_t   head;
 

  TMEMSET0( &head, FitsHeader_t );
 
  /* Get Image dimension from header */
  head=RetrieveFitsHeader(inunit);    
  
  if ( InitImagingVars(head) ) 
    goto proc_end;

  
  if(global.first)
    {
      global.count_table=(int **)calloc(global.im.naxis2, sizeof(int *));
      for(i=0; i<global.im.naxis2; i++)
	global.count_table[i]=(int *)calloc(global.im.naxis1, sizeof(int));
      global.first=0;
    }
  else if(global.naxis1 != global.im.naxis1 || global.naxis2 != global.im.naxis2 || global.type != global.im.type
	  || !(global.refvalx >= global.im.refvalx-sens && global.refvalx <= global.refvalx+sens) 
	  || !(global.refvaly >= global.im.refvaly-sens && global.refvaly <= global.refvaly+sens) 
	  || !(global.refx >= global.im.refx-sens && global.refx <= global.refx+sens) 
	  || !(global.refy >= global.im.refy-sens && global.refy <= global.refy+sens) 
	  || !(global.pixdeltx >= global.im.pixdeltx-sens && global.pixdeltx <= global.pixdeltx+sens) 
	  || !(global.pixdelty >= global.im.pixdelty-sens && global.pixdelty <= global.pixdelty+sens)
	  || (global.im.type == 0 && global.amp != global.im.amp))
    {
      headas_chat(NORMAL, " CountImage: Error the ext in %s file are not the same.\n", global.par.infile);
      goto proc_end;
    }
  
  /*
   *	read the map
   */
  
  global.naxis1=global.im.naxis1;
  global.naxis2=global.im.naxis2;
  global.type=global.im.type;
  global.amp=global.im.amp;
  
  
  global.refvalx = global.im.refvalx;
  global.refvaly = global.im.refvaly;
  global.refx = global.im.refx;
  global.refy = global.im.refy;
  global.pixdeltx = global.im.pixdeltx;
  global.pixdelty = global.im.pixdelty;
  if (!(image = (long **)ReadImage(inunit, J, global.im.nullval,
				   global.im.xmin,
				   global.im.xmax,
				   global.im.ymin,
				   global.im.ymax)))
    {
      headas_chat(MUTE, " CountImage: Error: reading image array.\n");
      goto proc_end;
    }
  
  
  for(i=0; i<global.im.naxis1 ; i++)
    {
      for(j=0; j<global.im.naxis2 ; j++)
	{
	  /* Build count image */
	  if (image[j][i] > 0) {
	    global.count_table[j][i]++ ;
	  }
	}
      
    }
  return OK;
  
 proc_end:
  return NOT_OK;
}/* CountImage */

/*
 *
 *	CreateCountImage 
 *
 *
 *	DESCRIPTION:
 *                 Create and open to write count image file 
 *                 
 *    
 *     
 *
 *	DOCUMENTATION:
 *        
 *
 *      FUNCTION CALL:
 *
 *           BOOL FileExists(const char *FileName);
 *           FitsFileUnit_t OpenWriteFitsFile(char *name);
 *           int remove(const char *pathname);
 *
 *
 *      CHANGE HISTORY:
 *       
 *        0.1.0: - RP 27/08/2003 - First version
 */

int CreateCountImage(FitsFileUnit_t *outunit)
{

if(xrtimage2cnt_checkinput())
  goto Error;
 
        
    /* Create output file */
    if ((*outunit = OpenWriteFitsFile(global.tmpcount)) <= (FitsFileUnit_t )0)
      {
	headas_chat(NORMAL, "WriteCountImage: Error: creating '%s' fits file.\n", global.tmpcount);
	goto Error;
      }




  return OK;

 Error:
  return NOT_OK;


}/*CreateCountImage*/
/*
 *	xrtimage2cnt_work
 *
 *
 *	DESCRIPTION:
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
 *           int HDpar_stamp(fitsfile *fptr, int hdunum, int *)
 *           int ChecksumCalc(FitsFileUnit_t unit);
 * 
 *	
 *      CHANGE HISTORY:
 *
 *        0.1.0: - BS 27/10/2003 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 *
 *      Return Status: 
 *        0: OK
 *        1: NOT_OK
 */
int xrtimage2cnt_work()
{
  int                   status = OK, inExt=0, outExt=0, countext=0,y = 0;
  FitsHeader_t          head;
  FitsFileUnit_t        inunit=NULL,  countunit=NULL;     /* Input and Output fits file pointer */
  
  TMEMSET0( &head, FitsHeader_t ); 

  GetGMTDateTime(global.date);
  /* Get input file name */
  global.first=1;
    
  if(CreateCountImage(&countunit))
    {
      headas_chat(CHATTY, "xrtimage2cnt_work: Error: create count image file.\n");
      goto Error;
    }
  

  /* Open readonly input event file */
  if ((inunit=OpenReadFitsFile(global.par.infile)) <= (FitsFileUnit_t )0)
    {
      headas_chat(NORMAL,"xrtimage2cnt_work: ... Error: opening '%s' FITS file.\n", global.par.infile);
      goto Error;
    }
    
  /* Check whether it is a Swift XRT File */
  ISXRTFILE_WARNING(NORMAL,inunit,global.par.infile,global.taskname);
        
  /* Get number of hdus in input events file */
  if (fits_get_num_hdus(inunit, &inExt, &status))
    {
      headas_chat(CHATTY, "xrtimage2cnt_work: Error: getting hdus num in '%s' file.\n", global.par.infile);
      goto Error;
    }
    
  outExt=1;
  /* Process all image extensions */
  while(outExt <= inExt && status == OK)
    {
      
      if(fits_movabs_hdu( inunit, outExt, NULL, &status ))
	{
	  headas_chat(CHATTY, "xrtimage2cnt_work: Error: unable to move in hdu %d in  '%s' file. \n", outExt, global.par.infile);
	  goto Error;
	}
      if(outExt == 1)
	{
	  /* Copy primary empty hdu */
	  if(fits_copy_header(inunit, countunit,&status))
	    { 
	      headas_chat(CHATTY, "xrtimage2cnt_work: Error: copying primary header to count image file.\n");
	      goto Error; 
	    } 
	  
	  head=RetrieveFitsHeader(inunit);
	  if (ExistsKeyWord(&head, KWNM_NAXIS1, NULLNULLCARD) && ExistsKeyWord(&head, KWNM_NAXIS2, NULLNULLCARD))
	    {
	      headas_chat(NORMAL, "%s: Error: Primary HDU is not empty.\n", global.taskname);
	      goto Error;
	    }
	  
	}
      else
	{
	  /* Check readout mode of input events file */
	  if(!(KeyWordMatch(inunit, KWNM_DATAMODE, S, KWVL_DATAMODE_IM, NULL)) && !(KeyWordMatch(inunit, KWNM_DATAMODE, S, KWVL_DATAMODE_IMS, NULL)) && !(KeyWordMatch(inunit, KWNM_DATAMODE, S, KWVL_DATAMODE_IML, NULL)))
	    headas_chat(NORMAL, "xrtimage2cnt_work: Warning readout mode in %d extension not allowed.\n", outExt);
	  else
	    {
	      
	      /* Process Images */
	      if (CountImage(inunit, outExt))
		{
		  headas_chat(MUTE, "%s: Error: processing image array.\n", global.taskname);
		  goto Error;
		}
	      else
		countext++;
	    }
	}
      outExt++;
    }
  
  if(countext == 0)
    {
      headas_chat(NORMAL, "%s: Error no image extension processed.\n", global.taskname);
      goto Error;
    }
  
  /* Write history */
  if(global.hist)
    {
      sprintf(global.strhist, "File created  by %s (%s) at %s", global.taskname, global.swxrtdas_v,global.date );
      fits_write_history(countunit, global.strhist, &status);
      
    }/* If (global.hist)*/
  /* Add history if parameter history set */
  if(HDpar_stamp(countunit, 0, &status))
    {
      headas_chat(CHATTY, "%s: Error updating history blocks.\n", global.taskname);
      goto Error;
    }
  /* Add keywords in output file */
  if(WriteCntKey(countunit))
    {
      headas_chat(NORMAL, "%s: Error adding necessary keywords.\n", global.taskname);
      goto Error;
    }
  /* Write array count image*/
  
  for(y=global.im.ymin; y<=global.im.ymax ; ++y)
    {
      fits_write_img_int(countunit, 0, global.im.naxis1*(y-global.im.ymin)+1, global.im.naxis1, &global.count_table[y][0], &status);
      if ( status ) 
	{
	  headas_chat(NORMAL, "%s: Error: writing count image.\n", global.taskname);
	  goto Error;
	}
      
    }

  /* Update checksum and datasum */    
  if(ChecksumCalc(countunit))
    {
      headas_chat(NORMAL, "Error updating checksum and datasum in output count image.\n");
      goto Error;
    }
    
  /* close count image files */
  if (CloseFitsFile(countunit))
    {
      headas_chat(CHATTY, "%s: Error:  unable to close  '%s' file. \n ", global.taskname, global.tmpcount);
      goto Error;
    } 
    
      
  /* close files */
  if (CloseFitsFile(inunit))
    {
      headas_chat(CHATTY, "%s: Error:  unable to close  '%s' file. \n ", global.taskname, global.par.infile);
      goto Error;
    }  
  
  /* rename temporary file into output count image file */
  if (rename (global.tmpcount,global.par.outfile) == -1)
    {
      headas_chat(NORMAL, "%s: Error: renaming temporary file %s .\n", global.taskname,global.tmpcount);
      goto Error;
    }
  
  headas_chat(NORMAL, "%s: Info: '%s' file successfully written.\n", global.taskname, global.par.outfile);
    
  
  
  
  /* Get next filename */    

  headas_chat(MUTE,"---------------------------------------------------------------------\n");
  headas_chat(MUTE, "%s: ... Exit with success.\n", global.taskname);
  headas_chat(MUTE,"---------------------------------------------------------------------\n");

  for (y=0; y < global.im.naxis2; y++)
    free(global.count_table[y]);
  free(global.count_table);
  
  return OK;

 Error:
  return NOT_OK;
  
} /* xrtimage2cnt_work */
/*
 *	xrtimage2cnt
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
 *             void xrtimage2cnt_getpar(void);
 *             void xrtimage2cnt_info(void); 
 * 	       void xrtimage2cnt_work(void);
 *
 *          
 *	CHANGE HISTORY:
 *        0.1.0: - BS 08/02/2002 - First version
 *
 *	AUTHORS:
 *
 *        ISAC Italian Swift Archive Center (FRASCATI)
 */
int xrtimage2cnt()
{
  /* set HEADAS globals */
  set_toolname(PRG_NAME);
  set_toolversion(XRTIMAGE2CNT_VERSION);

  get_toolnamev(global.taskname);
  
  GetSWXRTDASVersion(global.swxrtdas_v);

  /* Get parameter values */ 
  if ( xrtimage2cnt_getpar() == OK ) {
    
    if ( xrtimage2cnt_work() ) {
      headas_chat(MUTE, "%s: ... Exit with Error.\n", global.taskname);
      goto image_end;
    }
  }
  else
    goto image_end;
  return OK;

 image_end:
  if (FileExists(global.tmpcount))
    remove (global.tmpcount);

  return NOT_OK;
} /* xrtimage2cnt */
/*
 *	xrtimage2cnt_info
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
void xrtimage2cnt_info(void)
{
  headas_chat(MUTE,"---------------------------------------------------------------------\n");
  headas_chat(MUTE," \t\tRunning '%s'\n",global.taskname);
  headas_chat(MUTE,"---------------------------------------------------------------------\n");
  headas_chat(NORMAL,"\t\tList of input parameters: \n");
  headas_chat(NORMAL,"Name of input Event file               :'%s'\n",global.par.infile);
  headas_chat(NORMAL,"Name of output counts image file       :'%s'\n",global.par.outfile);
  if (global.hist)
    headas_chat(CHATTY,"Add history                            : yes\n");
  else
    headas_chat(CHATTY,"Add history                            : no\n");
  if (headas_clobpar)
    headas_chat(CHATTY,"Clobber                                : yes\n");
  else
    headas_chat(CHATTY,"Clobber                                : no\n");

  headas_chat(NORMAL,"---------------------------------------------------------------------\n");
} /* xrtimage2cnt_info */


int InitImagingVars(  FitsHeader_t   head )
{

  FitsCard_t *card;

  /* Get Naxis1 */
  if(ExistsKeyWord(&head, KWNM_NAXIS1, &card))
    global.im.naxis1 =card->u.JVal;
  else
    {
      headas_chat(NORMAL, "InitImaginsVars: Error: %s keyword not found.\n", KWNM_NAXIS1);
      goto Error;
    }


/* Get Naxis1 */
  if(ExistsKeyWord(&head, KWNM_NAXIS2, &card))
    global.im.naxis2 =card->u.JVal;
  else
    {
      headas_chat(NORMAL, "InitImaginsVars: Error: %s keyword not found.\n", KWNM_NAXIS2);
      goto Error;
    }

  global.im.xmax = global.im.naxis1 - 1;
  global.im.ymax = global.im.naxis2 - 1;
  global.im.xmin = 0;
  global.im.ymin = 0;


  global.im.type = GetCoordinateType( head );
  if ( global.im.type == -1 ) 
    {
      return NOT_OK;
    }
  else if ( global.im.type == RAW || global.im.type == DET )
    { 
      headas_chat(NORMAL,"InitImagingVars: Error: Cannot process Image with %s coordinate system\n", 
		  FitsImageTypeString[global.im.type]); 
      return NOT_OK; 
    } 
  
  
  if(ExistsKeyWord(&head, KWNM_AMP, &card))
    global.im.amp =card->u.JVal;

  if ( ExistsKeyWord(&head,KWNM_BLANK, &card) ){
	global.im.nullval = card->u.JVal;
  }
  else {
    global.im.nullval = FitsImageNullvalDef[global.im.type];
    headas_chat(CHATTY,"InitImagingVars: Warning: %s keyword not found, set to %f\n",
		KWNM_BLANK, global.im.nullval);
  }

  if ( ExistsKeyWord(&head,KWNM_CRVAL1, &card) ){
    DValCust( card, &global.im.refvalx);
  }
  else {
    global.im.refvalx = FitsImageRefvalxDef[global.im.type];
    headas_chat(CHATTY,"InitImagingVars: Warning: %s keyword not found, set to %f\n",
		KWNM_CRVAL1, global.im.refvalx);
  }

  if ( ExistsKeyWord(&head,KWNM_CRVAL2, &card) ){
    DValCust( card, &global.im.refvaly);
  }
  else {
    global.im.refvaly = FitsImageRefvalyDef[global.im.type];
    headas_chat(CHATTY,"InitImagingVars: Warning: %s keyword not found, set to %f\n",
		KWNM_CRVAL2, global.im.refvaly);
  }

  if ( ExistsKeyWord(&head,KWNM_CRPIX1, &card) ){
    DValCust( card, &global.im.refx);
  }
  else {
    global.im.refx = FitsImageRefxDef[global.im.type];;
    headas_chat(CHATTY,"InitImagingVars: Warning: %s keyword not found, set to %f\n",
		KWNM_CRPIX1, global.im.refx);
  }

  if ( ExistsKeyWord(&head,KWNM_CRPIX2, &card) ){
    DValCust( card, &global.im.refy);
  }
  else {
    global.im.refy = FitsImageRefyDef[global.im.type];
    headas_chat(CHATTY,"InitImagingVars: Warning: %s keyword not found, set to %f\n",
		KWNM_CRPIX2, global.im.refy);
  }

  if ( ExistsKeyWord(&head,KWNM_CDELT1, &card) ){
    DValCust( card, &global.im.pixdeltx);
  }
  else {
    global.im.pixdeltx = FitsImagePixdeltxDef[global.im.type];
    headas_chat(CHATTY,"InitImagingVars: Warning: %s keyword not found, set to %f\n",
		KWNM_CDELT1, global.im.pixdeltx);
  }

  if ( ExistsKeyWord(&head,KWNM_CDELT2, &card) ){
    DValCust( card, &global.im.pixdelty);
  }
  else {
    global.im.pixdelty = FitsImagePixdeltyDef[global.im.type];
    headas_chat(CHATTY,"InitImagingVars: Warning: %s keyword not found, set to %f\n",
		KWNM_CDELT2, global.im.pixdelty);
  }

  headas_chat(CHATTY,"InitImagingVars: Info: Processing Image with %s Coordinate system\n",
	      FitsImageTypeString[global.im.type]);
  headas_chat(CHATTY,"InitImagingVars: Info: Reference Pixel: %f %f\n",global.im.refx,global.im.refy);
  headas_chat(CHATTY,"InitImagingVars: Info: Reference Value: %f %f\n",global.im.refvalx,global.im.refvaly);
  headas_chat(CHATTY,"InitImagingVars: Info: Delta Pixel    : %f %f\n",global.im.pixdeltx,global.im.pixdelty);

  return OK;

 Error:
  return NOT_OK;

}


int DValCust( FitsCard_t *card, double *cardval) {

  switch(card->ValueTag) {
  case J: /* integer */
    *cardval = ( double)card->u.JVal;
    break;
  case E:
  case F:	/* float */
    *cardval = ( double)card->u.EVal;
    break;
  case D:
  case G: /* double */
    *cardval = ( double)card->u.DVal;
    break;
  default:
    Error("InsertCard", "Unkown tag field detected");
    return NOT_OK;
  }
  return OK;
}

int JValCust( FitsCard_t *card, int *cardval) {
  switch(card->ValueTag) {
  case J:    /* integer */
    *cardval = ( int)card->u.JVal;
    break;
  case E:
  case F:    /* float */
    *cardval = ( int)card->u.EVal;
    headas_chat(NORMAL,"JValCust: Warning: Keyword %s with value %f customized into an integer value %d\n",card->KeyWord, card->u.EVal, *cardval );
    break;
  case D: 
 case G:    /* double */
    *cardval = ( int)card->u.DVal;
    headas_chat(NORMAL,"JValCust: Warning: Keyword %s with value %f customized into an integer value %d\n",card->KeyWord, card->u.DVal, *cardval );
    break;
  default:
    Error("InsertCard", "Unkown tag field detected");
    return NOT_OK;
  }
  return OK;
}

int GetCoordinateType ( FitsHeader_t   head ) {

  int        type, typex= -1, typey = -1;
  FitsCard_t *card;


  if ( ExistsKeyWord(&head,KWNM_CTYPE1, &card) ){
    if ( !strcmp( card->u.SVal, KWVL_CTYPE1_RAW ) ) typex = RAW;
    if ( !strcmp( card->u.SVal, KWVL_CTYPE1_DET ) ) typex = DET;
    if ( !strcmp( card->u.SVal, KWVL_CTYPE1_SKY ) ) typex = SKY;
  }
  else {
    headas_chat(CHATTY,"GetCoordinateType: Warning: Keyword %s not found. Using X RAW coordinate\n",KWNM_CTYPE1);
    typex = RAW;
  }

  if ( ExistsKeyWord(&head,KWNM_CTYPE2, &card) ){
    if ( !strcmp( card->u.SVal, KWVL_CTYPE2_RAW ) ) typey = RAW;
    if ( !strcmp( card->u.SVal, KWVL_CTYPE2_DET ) ) typey = DET;
    if ( !strcmp( card->u.SVal, KWVL_CTYPE2_SKY ) ) typey = SKY;
  }
  else {
    headas_chat(CHATTY,"GetCoordinateType: Warning: Keyword %s not found. Using Y RAW coordinate\n",KWNM_CTYPE2);
    typey = RAW;
  }

  if ( typex == typey ) {
    type = typex;
  }
  else {
    headas_chat(CHATTY,"GetCoordinateType: Error: '%s' and '%s' keywords are not consistent\n",KWNM_CTYPE1,KWNM_CTYPE2);
    return -1;
  }

  return type;
}

/*
 *	WriteCntKey:
 *
 *
 *	DESCRIPTION:
 *          Routine to copy keywords from events to bad pixels file
 *
 *	DOCUMENTATION:
 *        `
 *
 *      FUNCTION CALL:
 *             headas_chat(int ,char *, ...);
 *
 *             int fits_movnam_hdu(fitsfile *fptr,int hdutype, char *extname, int extver, int *status);
 *
 *             FitsHeader_t   RetrieveFitsHeader(FitsFileUnit_t unit);
 *             void AddCard(FitsHeader_t *header, const char *KeyWord, const ValueTag_t vtag, 
 *	
 *       CHANGE HISTORY:
 *        0.1.0: - BS 22/07/2003 - First version
 *
 *	AUTHORS:
 *
 *        Italian Swift Archive Center (FRASCATI)
 */
int WriteCntKey(FitsFileUnit_t countunit)
{
  int            naxis=0, status=0;
  
  naxis=2;
  fits_update_key(countunit, TINT, "NAXIS", &naxis, "number of data axes", &status);
  fits_insert_record(countunit, 4, KWNM_NAXIS1 , &status);
  fits_update_key(countunit, TINT, KWNM_NAXIS1, &global.im.naxis1, CARD_COMM_NAXIS1 , &status);
  fits_insert_record(countunit, 5, KWNM_NAXIS2 , &status);
  fits_update_key(countunit, TINT, KWNM_NAXIS2, &global.im.naxis2, CARD_COMM_NAXIS2 , &status);
  if(global.type == 0)
    {
      fits_update_key(countunit, TINT, KWNM_AMP,&global.im.amp , "Amplifier number" , &status);
      fits_update_key(countunit, TSTRING, KWNM_CTYPE1,KWVL_CTYPE1_RAW , CARD_COMM_CTYPE1 , &status);
      fits_update_key(countunit, TSTRING, KWNM_CTYPE2,KWVL_CTYPE2_RAW , CARD_COMM_CTYPE2 , &status);
    }
  else if(global.type == 1)
    {
      fits_update_key(countunit, TSTRING, KWNM_CTYPE1,KWVL_CTYPE1_DET , CARD_COMM_CTYPE1 , &status);
      fits_update_key(countunit, TSTRING, KWNM_CTYPE2,KWVL_CTYPE2_DET , CARD_COMM_CTYPE2 , &status);
    }
  else
    {
      fits_update_key(countunit, TSTRING, KWNM_CTYPE1,KWVL_CTYPE1_SKY , CARD_COMM_CTYPE1 , &status);
      fits_update_key(countunit, TSTRING, KWNM_CTYPE2,KWVL_CTYPE2_SKY , CARD_COMM_CTYPE2 , &status);
    }
  
  fits_update_key(countunit, TDOUBLE, KWNM_CRVAL1,&global.refvalx , CARD_COMM_CRVAL1 , &status);
  fits_update_key(countunit, TDOUBLE, KWNM_CRVAL2,&global.refvaly, CARD_COMM_CRVAL2 , &status);
  
  fits_update_key(countunit, TDOUBLE, KWNM_CRPIX1,&global.refx , CARD_COMM_CRPIX1 , &status);
  fits_update_key(countunit, TDOUBLE, KWNM_CRPIX2,&global.refy, CARD_COMM_CRPIX2 , &status);
  
  fits_update_key(countunit, TDOUBLE, KWNM_CDELT1,&global.pixdeltx , CARD_COMM_CDELT1 , &status);
  fits_update_key(countunit, TDOUBLE, KWNM_CDELT2,&global.pixdelty, CARD_COMM_CDELT2 , &status);

  if(status == 0)
    return OK;
  else
    {
      headas_chat(NORMAL, "WriteCntKey: Error writing keywords in output image file.\n");
      return NOT_OK;
    }
  
}/* WriteCntKey */

int xrtimage2cnt_checkinput(void)
{
  
  char                  stem[10],BaseName[MAXFNAME_LEN];
  pid_t                 tmp;
 

  /* If global.par.outfile == DEFAULT build filename */ 
  if (!(strcasecmp (global.par.outfile, "DEFAULT")))
    {
      SplitFilePath(global.par.infile, NULL, BaseName);
      StripExtension(BaseName);
      strncpy(global.par.outfile, BaseName, STEMLEN);
      strcat(global.par.outfile, COUNTFITSEXT);
      headas_chat(CHATTY, "%s: Default output count file name set to: '%s'\n", global.taskname, global.par.outfile);
    }


  /* Derive temporary event filename */
  tmp=getpid(); 
  sprintf(stem, "%dtmp", (int)tmp);
  DeriveFileName(global.par.outfile, global.tmpcount, stem);

  if(FileExists(global.par.outfile))
    {
      if(!headas_clobpar)
	{
	  headas_chat(NORMAL, "%s: Error: '%s' file exists,\n", global.taskname, global.par.outfile);
	  headas_chat(NORMAL, "%s: Error: please set the parameter clobber to yes to overwrite it.\n", global.taskname);
	  goto check_end;
	}
      else
	{
	  headas_chat(NORMAL, "%s: Warning: parameter 'clobber' set,\n", global.taskname);
	  headas_chat(NORMAL, "%s: Warning: the '%s' file will be overwritten\n", global.taskname, global.par.outfile);
	  if(remove (global.par.outfile) == -1)
	    {
	      headas_chat(NORMAL, "%s: Error: removing '%s' file.\n", global.taskname, global.par.outfile);
	      goto check_end;
	    } 
	}
    }




  /* Check if tmp file exists to remove it*/
  if(FileExists(global.tmpcount))
    if(remove (global.tmpcount) == -1)
	{
	  headas_chat(NORMAL, "%s: Error: removing '%s' file.\n", global.taskname, global.tmpcount);
	  goto check_end;
	}

  return OK;

 check_end:
  return NOT_OK;

}/* xrtimage2cnt */
