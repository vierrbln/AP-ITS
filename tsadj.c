
/*******************************************************************************/
/**
@file tsadjus.c
 *
@brief all functions to control teststand control panel @ compactTSVP
 *
@version 1.0.0.0
 *
@author Robert Renner <A HREF=k"mailto:robert.renner@spectral.de">robert.renner@spectral.de</A>\n
&copy; Spectral Electronic Forschungs- und Produktions GmbH - Berlin \n \n
language: ANSI-C ISO/IEC9899:1990
 *
<b>History:</b>
- <b>23.11.2007 R. Renner</b>
- Initial revision

 ******************************************************************************
 *****************************************************************************/

/* INCLUDE FILES **************************************************************/

/* EXPORT */
#include "tsadj.h"

/* GLOBAL CONSTANT DEFINITIONS ************************************************/


#ifndef COINIT_MULTITHREADED
   #define COINIT_MULTITHREADED  0x0      // OLE calls objects on any thread.
#endif



/* GLOBAL VARIABLES DEFINITION ************************************************/
CRITICAL_SECTION CriticalSection;
int giAdjustmentPanelHandle = 0;
int giMenuBarHandle = 0;
int iThreadCount = 0;
HANDLE hThreadReady = 0;
HANDLE hThreadHandle = 0;



/* LOCAL DEFINES *************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/

/* LOCAL CONSTANT DEFINITIONS *************************************************/
static const char LIBRARY_NAME[] = "TSADJ"; /* Library Version String */
static const char LIB_VERSION[] = "TSADJ 1.1.0.4"; /* Library Version String */
static char *cDLLPATH;

/* Error code to message reference table */
static GTSL_ERROR_TABLE errorTable =
{
   /* library specific error codes and messages */
   {
      TSPAN_ERR_NOT_A_BENCH, "The given resource is not a bench"
   }
   ,
   {
      TSPAN_ERR_THREADWASNOTSTARTET,
         "Thread was not started. Possible cause: UIR file was not found."
   }
   ,                                                                           
   {
      TSPAN_ERR_WRONGFORMATYTE,
         "Format type is not supported."
   }     
  ,
   /* include common GTSL error codes and messages */
   GTSL_ERROR_CODES_AND_MESSAGES,                         
                                                           
   /* this must be the last entry ! */
   {
      0, NULL
   }
};
                                                             

/* LOCAL TYPE DEFINITIONS *****************************************************/

typedef struct
{
   int iOwner; /* memory block owner                          */
   int iSimulation; /* driver simulation                           */
   int iDemoMode; /* driver simulation                           */
   int iActualPanelHandle;
   DWORD gThreadID;
   
} BENCH_STRUCT;

typedef struct threadDataRec
{
   CAObjHandle execution;
   CAObjHandle ThisContext;
} ThreadData;


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/
WINOLEAPI CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);
int StartThreadForAdjustmentPanel(void *data); 
void CVICALLBACK QuitThread(void *callbackData);
static void formatError(char buffer[], int code, long resId, char *benchDevice);

/* EXPORTED FUNCTION DEFINITIONS **********************************************/

int __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   switch (fdwReason)
   {
      case DLL_PROCESS_ATTACH:

         if (InitCVIRTE(hinstDLL, 0, 0) == 0)
         /* Needed if linking in external compiler; harmless otherwise */
         {
            return 0;
         }
         /* out of memory */
         InitializeCriticalSection(&CriticalSection);
         hThreadReady = CreateEvent(NULL, TRUE, FALSE, NULL);
         //GetModuleDir(__CVIUserHInst, cDLLPATH);
         if (hThreadReady == NULL)
         {
            return FALSE;
         }
         break;

      case DLL_PROCESS_DETACH:

         if (!CVIRTEHasBeenDetached())
         /* Do not call CVI functions if cvirte.dll has already been detached.    */
         {
            CloseCVIRTE(); /* Needed if linking in external compiler; harmless
               */
            //otherwise
         }
         break;
   }

   return 1;
}

////////////////////////////////////////////////////////////////////////////////
int __stdcall DllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID
   lpvReserved)
{
   /* Included for compatibility with Borland    */

   return DllMain(hinstDLL, fdwReason, lpvReserved);
}



void __stdcall tsadj_Setup(CAObjHandle sequenceContext, char *pBenchName, long
   *pResourceId, short *pErrorOccurred, long *pErrorCode, char errorMessage[])
{
   char cSectionName[1024];
   char cTraceBuffer[1024];
   char cTempBuffer[1024];
   char cTempBuffer2[1024];
   char cSystemID[1024];
   
   short sLicenseOK;
   
   int idx;

   long lBytesWritten;
   long lMatched;
   long lResourceType;
   long lTrace;

   BENCH_STRUCT *pBench = NULL;
   
   ERRORINFO pTSErrorInfo;
   
   /*---------------------------------------------------------------------/
   /   Allocate the resource:
   /     Check whether "pBenchName" can be found in the INI files and
   /     return a resource ID for the bench. This resource ID is the
   /     "ticket" for any subsequent action dealing with this bench.
   /---------------------------------------------------------------------*/
   RESMGR_Alloc_Resource(sequenceContext, pBenchName, pResourceId,
      pErrorOccurred, pErrorCode, errorMessage);

   /*---------------------------------------------------------------------/
   /  Check for trace flag:
   /    The "Trace" key in the bench section is searched and its
   /    value is checked. The result is recorded in the static
   /    variable trace
   /---------------------------------------------------------------------*/
   if (!*pErrorOccurred)
   {
      RESMGR_Compare_Value(sequenceContext,  *pResourceId, "", RESMGR_KEY_TRACE,
         "1", &lTrace, pErrorOccurred, pErrorCode, errorMessage);
   }
   if (!*pErrorOccurred)
   {
      RESMGR_Set_Trace_Flag(*pResourceId, lTrace);

      if (lTrace)
      {
         RESMGR_Trace(">>TSADJ_Setup begin");
         RESMGR_Trace("Tracing for tsadj.dll enabled");
         sprintf(cTraceBuffer, "Bench name %s -> Resource ID %ld", pBenchName,
            *pResourceId);
         RESMGR_Trace(cTraceBuffer);
      }
   }

   /*---------------------------------------------------------------------/
   /   Check the resource type:
   /     The tspan library requires that pBenchName refers to a bench,
   /     not to a single device. It is always recommended to use a bench
   /     instead of a device, because a bench makes it easy to add
   /     a device in future and to work with alternative devices
   /     like a ....
   /---------------------------------------------------------------------*/
   if (! *pErrorOccurred)
   {
      RESMGR_Get_Resource_Type(sequenceContext,  *pResourceId, &lResourceType,
         pErrorOccurred, pErrorCode, errorMessage);
      if (!*pErrorOccurred)
      {
         if (lResourceType != RESMGR_TYPE_BENCH)
         {
            *pErrorOccurred = TRUE;
            *pErrorCode = TSPAN_ERR_NOT_A_BENCH;
            formatError(errorMessage,  *pErrorCode,  *pResourceId, NULL);
         }
      }
   }
   
   /*---------------------------------------------------------------------/
   /   Check the license:
   /      If correct license file not found, the tspan library runs in 
   /      demo mode
   /---------------------------------------------------------------------*/
   /*if (!*pErrorOccurred)
   {
       RESMGR_Read_ROM (sequenceContext, cSystemID, pErrorOccurred, pErrorCode, errorMessage);
       sLicenseOK = lc_CheckLicense ("SW-ITS", cSystemID);
   } */
   
   /*---------------------------------------------------------------------/
   /   Allocate a memory block:
   /     The tspan library uses a structure to store some private
   /     information along with the resource ID. This information can
   /     be retrieved in subsequent calls to the measurement functions.
   /---------------------------------------------------------------------*/
   if (!*pErrorOccurred)
   {
      RESMGR_Alloc_Memory(sequenceContext,  *pResourceId, sizeof(BENCH_STRUCT),
         (void **)(&pBench), pErrorOccurred, pErrorCode, errorMessage);
      if (! *pErrorOccurred)
      {
         /* set the memory block owner field to a unique value
         which identifies the ENOCEAN library as the owner of the memory.
          */
         pBench->iOwner = TSPAN_ERR_BASE;

         /* set default values */
         /*if (sLicenseOK) 
         {
            sprintf(cTraceBuffer, "Valid license found! (System ID: %s)", cSystemID);
            pBench->iDemoMode = FALSE; 
         }
         if (!sLicenseOK) 
         {
             sprintf(cTraceBuffer, "Demo mode is enabled, not valid license found! (System ID: %s)", cSystemID);
             pBench->iDemoMode = FALSE;
         }		*/
		 pBench->iDemoMode = FALSE;
         RESMGR_Trace(cTraceBuffer);
         pBench->iSimulation = FALSE;
         pBench->iActualPanelHandle = 0;
      }
   }

   /*---------------------------------------------------------------------/
   /   Check for simulation flag:
   /     The "Simulation" key in the bench section is searched and its
   /     value is checked. The result is recorded in the memory block.
   /     Any other library-specific information like calibration,
   /     path for calibration files etc. may be handled the same way
   /     (not shown in the example).
   /---------------------------------------------------------------------*/
   if (!*pErrorOccurred)
   {
      RESMGR_Compare_Value(sequenceContext,  *pResourceId, "",
         RESMGR_KEY_SIMULATION, "1", &lMatched, pErrorOccurred, pErrorCode,
         errorMessage);
      if (! *pErrorOccurred)
      {
         if (lMatched)
         {
            pBench->iSimulation = TRUE;
            if (lTrace)
            {
               RESMGR_Trace("Simulation is enabled!");
            }
         }
      }
   }
   
   
   /*---------------------------------------------------------------------/
   /   Cleanup and error handling
   /---------------------------------------------------------------------*/
   if (lTrace)
   {
      if (*pErrorOccurred)
      {
         sprintf(cTraceBuffer, "Error %ld : %s",  *pErrorCode, errorMessage);
         RESMGR_Trace(cTraceBuffer);
      }
      RESMGR_Trace("<<TSADJ_Setup end");
   }

}




void __stdcall tsadj_DisplayAdjustmentPanel (CAObjHandle sequenceContext,
                                            long pResourceId, char nameOfStep[], char buttonText[],
                                            char unit[], char compType[],
                                            char format[], double lowerLimit,
                                            double upperLimit, short AlwaysOnTop,
                                            short *pErrorOccurred,
                                            long *pErrorCode, char errorMessage[])
{
   char cTraceBuffer[1024];
   char cTempBuffer[1024]; 
   char cValue[1024]; 
   int iTabHandle;
   
   int iFindFormat;
   int iFindComaHa;
   int iCanConvert;
   char cTemp[4];
   int iTemp;
   int idx;
   char cPrecision[4];
   int iPrecision;

   long lTrace;

   BENCH_STRUCT *pBench = NULL;

   CAObjHandle execution;
   ERRORINFO errorInfo;
   HRESULT hResult;
   static ThreadData threadDataItem =
   {
      0
   };
   CAObjHandle tmpExecutionObjHandle = 0;
   LPDISPATCH tmpExecutionDispPtr = NULL;
   CAObjHandle tmpStationGlobalsObjHandle = 0;
   LPDISPATCH tmpStationGlobalsDispPtr = NULL;


   lTrace = RESMGR_Get_Trace_Flag(pResourceId);

   if (lTrace)
   {
      RESMGR_Trace(">>TSPAN_DisplayAdjustmentPanel begin");
   }
   /*---------------------------------------------------------------------/
   /   Retrieve the memory pointer:
   /     Get a pointer to the memory block to check the configuration
   /---------------------------------------------------------------------*/
   RESMGR_Get_Mem_Ptr(sequenceContext, pResourceId, (void **)(&pBench),
      pErrorOccurred, pErrorCode, errorMessage);
   /*---------------------------------------------------------------------/
   /   Check for memory block owner:
   /     To be sure that the given resource ID belongs to the ENOCEAN
   /     library, we check the "owner" field of the memory block if it
   /     contains the "magic number" we have stored there in the
   /     ENOCEAN_Setup function
   /---------------------------------------------------------------------*/
   if (! *pErrorOccurred)
   {
      if (pBench->iOwner != TSPAN_ERR_BASE)
      {
         *pErrorOccurred = TRUE;
         *pErrorCode = GTSL_ERR_WRONG_RESOURCE_ID;
         formatError(errorMessage,  *pErrorCode, pResourceId, NULL);
      }
   }

   if (! *pErrorOccurred)
   {
      if (lTrace)
      {
         RESMGR_Trace("InterlockedIncrement");
      }

      // Ensure only 1 thread updates thread_count at a time
      InterlockedIncrement(&iThreadCount);

      if (lTrace)
      {
         sprintf(cTraceBuffer, "ThreadCount: %d", iThreadCount);
         RESMGR_Trace(cTraceBuffer);
         RESMGR_Trace("EnterCriticalSection");
      }

      //InitializeCriticalSection(&CriticalSection);

      EnterCriticalSection(&CriticalSection);

      if (iThreadCount > 1)
      {
         LeaveCriticalSection(&CriticalSection);
      }
      else
      {

         if (lTrace)
         {
            RESMGR_Trace("TS_SeqContextGetProperty");
         }

         //Get teststand executon reference
         hResult = TS_SeqContextGetProperty(sequenceContext, &errorInfo,
            TS_SeqContextExecution, CAVT_OBJHANDLE, &execution);
         if (hResult < 0)
         {
            *pErrorOccurred = TRUE;
            *pErrorCode = hResult;
            CA_GetAutomationErrorString(hResult, errorMessage, 1024);
         }

      }

   }

   if (lTrace)
   {
      RESMGR_Trace("Add a reference to the execution activeX automation object")
         ;
   }
   // add a reference to the execution activeX automation object
   if (! *pErrorOccurred)
   {
      hResult = CA_GetDispatchFromObjHandle(execution, &tmpExecutionDispPtr);
      if (hResult < 0)
      {
         *pErrorOccurred = TRUE;
         *pErrorCode = hResult;
         CA_GetAutomationErrorString(hResult, errorMessage, 1024);
      }
   }

   if (! *pErrorOccurred)
   {
      hResult = CA_CreateObjHandleFromIDispatch(tmpExecutionDispPtr, 1,
         &tmpExecutionObjHandle);
      if (hResult < 0)
      {
         *pErrorOccurred = TRUE;
         *pErrorCode = hResult;
         CA_GetAutomationErrorString(hResult, errorMessage, 1024);
      }
   }

   threadDataItem.execution = tmpExecutionObjHandle;
   if (lTrace)
   {
      RESMGR_Trace("Add a reference to the station globals activeX automation object");
   }
   // add a reference to the ThisContext activeX automation object
   if (! *pErrorOccurred)
   {
      hResult = CA_GetDispatchFromObjHandle(sequenceContext,
         &tmpStationGlobalsDispPtr);
      if (hResult < 0)
      {
         *pErrorOccurred = TRUE;
         *pErrorCode = hResult;
         CA_GetAutomationErrorString(hResult, errorMessage, 1024);
      }
   }
   if (! *pErrorOccurred)
   {
      hResult = CA_CreateObjHandleFromIDispatch(tmpStationGlobalsDispPtr, 1,
         &tmpStationGlobalsObjHandle);
      if (hResult < 0)
      {
         *pErrorOccurred = TRUE;
         *pErrorCode = hResult;
         CA_GetAutomationErrorString(hResult, errorMessage, 1024);
      }
   }
   if (! *pErrorOccurred)
   {
      threadDataItem.ThisContext = tmpStationGlobalsObjHandle;

      if (lTrace)
      {
         RESMGR_Trace("Create new thread for UIR control");
      }
	  
      hThreadReady = 0;
      //Create new thread for UIR control
      hThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)
         StartThreadForAdjustmentPanel, (void*) &threadDataItem, 0, &pBench
         ->gThreadID);

      // Wait until thread starts
      if (WaitForSingleObject(hThreadReady, WAITTMO) == WAIT_TIMEOUT)
      {
         *pErrorOccurred = TRUE;
         *pErrorCode = TSPAN_ERR_THREADWASNOTSTARTET;
         formatError(errorMessage,  *pErrorCode, pResourceId, NULL);
      }

   }
   
   if (lTrace)
   {
      RESMGR_Trace("LeaveCriticalSection");
   }
   
   LeaveCriticalSection(&CriticalSection);

   if (execution)
   {
      CA_DiscardObjHandle(execution);
      execution = 0;
   }		  

   Delay(1.0);
   
   SetCtrlVal(giAdjustmentPanelHandle, ADJUSTMENT_TEXT, nameOfStep); 
   
   FormatValues (cValue, format, lowerLimit); 
   sprintf(cTempBuffer, "LL: %s %s", cValue, unit);
   SetCtrlVal(giAdjustmentPanelHandle, ADJUSTMENT_LL, cTempBuffer);
   
   FormatValues (cValue, format, upperLimit);   
   sprintf(cTempBuffer, "UL: %s %s", cValue, unit);
   SetCtrlVal(giAdjustmentPanelHandle, ADJUSTMENT_UL, cTempBuffer); 
   
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_UL, ATTR_TEXT_BGCOLOR, VAL_TRANSPARENT); 
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_LL, ATTR_TEXT_BGCOLOR, VAL_TRANSPARENT); 
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_TEXT, ATTR_TEXT_BGCOLOR, VAL_TRANSPARENT); 
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_UNIT, ATTR_TEXT_BGCOLOR, VAL_TRANSPARENT); 
   SetCtrlVal(giAdjustmentPanelHandle, ADJUSTMENT_UNIT, unit); 
    
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_MIN_VALUE, lowerLimit);
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_MAX_VALUE, upperLimit);
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_FILL_COLOR, VAL_BLACK);
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_FILL_HOUSING_COLOR, VAL_WHITE);
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_FILL_HOUSING_COLOR, VAL_WHITE);
   
   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_FORMAT, VAL_DECIMAL_FORMAT);
   iFindFormat = FindPattern (format, 0, strlen(format), "f", 0, 0);
   if (iFindFormat != -1) 
   {
	   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_FORMAT, VAL_FLOATING_PT_FORMAT);
   } else 
   {
   	iFindFormat = FindPattern (format, 0, strlen(format), "e", 0, 0);
   	if (iFindFormat != -1) SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_FORMAT, VAL_SCIENTIFIC_FORMAT);
   }
   
   if (iFindFormat != -1)
   {
	   iFindComaHa = FindPattern (format, 0, strlen(format), ".", 0, 0);
	   if (iFindComaHa == -1) iFindComaHa = FindPattern (format, 0, strlen(format), ",", 0, 0);
	   if (iFindComaHa != -1) 
	   {
		   iCanConvert = 1; idx = 0;
		   while (iCanConvert)
		   {
				iFindComaHa++;
				cTemp[0] = format[iFindComaHa];
				cTemp[1] = '\0';
				iCanConvert = StrToInt(cTemp, &iTemp);
				if (iCanConvert) 
				{
					cPrecision[idx] = format[iFindComaHa];
					idx++;
				}
		   }
		   cPrecision[idx] = '\0'; 
	   }
	   iCanConvert = StrToInt(cPrecision, &iPrecision);
	   SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_PRECISION, iPrecision);
   
   }
   
   
   
   if (strlen(buttonText) > 8)
   {
   		cTempBuffer[0] = buttonText[0];
	    cTempBuffer[1] = buttonText[1];
	    cTempBuffer[2] = buttonText[2];
	    cTempBuffer[3] = buttonText[3];
	    cTempBuffer[4] = buttonText[4];
	    cTempBuffer[5] = buttonText[5];
	    cTempBuffer[6] = buttonText[6];
	    cTempBuffer[7] = buttonText[7];
	    cTempBuffer[8] = buttonText[8];
	    cTempBuffer[9] = '.';
	    cTempBuffer[10] = '.';
	    cTempBuffer[11] = '.';
	    SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_OK, ATTR_LABEL_TEXT, cTempBuffer);
   } else {
   		SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_OK, ATTR_LABEL_TEXT, buttonText);
   }
   
   if (pBench->iDemoMode)
   {
   		SetCtrlVal(giAdjustmentPanelHandle, ADJUSTMENT_TEXT, "Adjustment panel in demo mode");  
		SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_OK, ATTR_LABEL_TEXT, "DEMO"); 
   }
   
   Delay(0.6);
   
   ProcessDrawEvents();
   
   if (lTrace)
   {
      if (*pErrorOccurred)
      {
         sprintf(cTraceBuffer, "Error %ld : %s",  *pErrorCode, errorMessage);
         RESMGR_Trace(cTraceBuffer);
      }
      RESMGR_Trace("<<TSPAN_DisplayAdjustmentPanel end");
   }
   
    
}

void __stdcall tsadj_SetValueAdjustmentPanel (CAObjHandle sequenceContext,
                                             long pResourceId, double value,
                                             short *pErrorOccurred,
                                             long *pErrorCode,
                                             char errorMessage[])
{

   char cTraceBuffer[1024];
   double lowerLimit;
   double upperLimit;
   long lTrace;

   BENCH_STRUCT *pBench = NULL;

   lTrace = RESMGR_Get_Trace_Flag(pResourceId);

   if (lTrace)
   {
      RESMGR_Trace(">>TSPAN_SetValueAdjustmentPanel begin");
   }
   /*---------------------------------------------------------------------/
   /   Retrieve the memory pointer:
   /     Get a pointer to the memory block to check the configuration
   /---------------------------------------------------------------------*/
   RESMGR_Get_Mem_Ptr(sequenceContext, pResourceId, (void **)(&pBench),
      pErrorOccurred, pErrorCode, errorMessage);
   /*---------------------------------------------------------------------/
   /   Check for memory block owner:
   /     To be sure that the given resource ID belongs to the ENOCEAN
   /     library, we check the "owner" field of the memory block if it
   /     contains the "magic number" we have stored there in the
   /     ENOCEAN_Setup function
   /---------------------------------------------------------------------*/
   if (! *pErrorOccurred)
   {
      if (pBench->iOwner != TSPAN_ERR_BASE)
      {
         *pErrorOccurred = TRUE;
         *pErrorCode = GTSL_ERR_WRONG_RESOURCE_ID;
         formatError(errorMessage,  *pErrorCode, pResourceId, NULL);
      }
   }

   if (! *pErrorOccurred)
   {
	  
	  GetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_MIN_VALUE, &lowerLimit);
      GetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_INDICATOR, ATTR_MAX_VALUE, &upperLimit);
      SetCtrlVal(giAdjustmentPanelHandle,ADJUSTMENT_INDICATOR,value);
	  if ((value < lowerLimit) || (value > upperLimit))
      {
		SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_BACKGROUND, ATTR_TEXT_BGCOLOR, VAL_RED);
	  } else {
	  	SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_BACKGROUND, ATTR_TEXT_BGCOLOR, VAL_GREEN);
	  }
	  
	  if (pBench->iDemoMode)
      {
   		Delay(0.5);
		SetCtrlVal(giAdjustmentPanelHandle, ADJUSTMENT_TEXT, "Adjustment panel in demo mode");  
		SetCtrlAttribute (giAdjustmentPanelHandle, ADJUSTMENT_BACKGROUND, ATTR_TEXT_BGCOLOR, VAL_MAGENTA);
		SetCtrlVal(giAdjustmentPanelHandle,ADJUSTMENT_INDICATOR,-1.0);
      }
   
   }
   
   
   if (lTrace)
   {
      if (*pErrorOccurred)
      {
         sprintf(cTraceBuffer, "Error %ld : %s",  *pErrorCode, errorMessage);
         RESMGR_Trace(cTraceBuffer);
      }
      RESMGR_Trace("<<TSPAN_SetValueAdjustmentPanel end");
   }

}

void __stdcall tsadj_HideAdjustmentPanel (CAObjHandle sequenceContext,
                                         long pResourceId, short *pErrorOccurred,
                                         long *pErrorCode, char errorMessage[])
{

   char cTraceBuffer[1024];

   long lTrace;

   BENCH_STRUCT *pBench = NULL;

   lTrace = RESMGR_Get_Trace_Flag(pResourceId);

   if (lTrace)
   {
      RESMGR_Trace(">>TSPAN_HideAdjustmentPanel begin");
   }
   /*---------------------------------------------------------------------/
   /   Retrieve the memory pointer:
   /     Get a pointer to the memory block to check the configuration
   /---------------------------------------------------------------------*/
   RESMGR_Get_Mem_Ptr(sequenceContext, pResourceId, (void **)(&pBench),
      pErrorOccurred, pErrorCode, errorMessage);
   /*---------------------------------------------------------------------/
   /   Check for memory block owner:
   /     To be sure that the given resource ID belongs to the ENOCEAN
   /     library, we check the "owner" field of the memory block if it
   /     contains the "magic number" we have stored there in the
   /     ENOCEAN_Setup function
   /---------------------------------------------------------------------*/
   if (! *pErrorOccurred)
   {
      if (pBench->iOwner != TSPAN_ERR_BASE)
      {
         *pErrorOccurred = TRUE;
         *pErrorCode = GTSL_ERR_WRONG_RESOURCE_ID;
         formatError(errorMessage,  *pErrorCode, pResourceId, NULL);
      }
   }

   if (! *pErrorOccurred)
   {
      if (lTrace)
      {
         RESMGR_Trace("InterlockedDecrement");
      }
      // Ensure only 1 thread updates thread_count at a time
      InterlockedDecrement(&iThreadCount);

      if (lTrace)
      {
         RESMGR_Trace("EnterCriticalSection");
      }

      EnterCriticalSection(&CriticalSection);
      if (iThreadCount > 0)
      {
         LeaveCriticalSection(&CriticalSection);
         return ;
      }

      if (lTrace)
      {
         RESMGR_Trace("LeaveCriticalSection");
      }

      LeaveCriticalSection(&CriticalSection);

      if (lTrace)
      {
         RESMGR_Trace("PostDeferredCallToThread");
      }

      // Call function in new thread to quit new thread
      PostDeferredCallToThread(QuitThread, 0, pBench->gThreadID);

      if (hThreadHandle != NULL)
      {
         // wait for the thread to complete.
         WaitForSingleObject(hThreadHandle, INFINITE);
         CloseHandle(hThreadHandle);
      }
	  
	  DiscardPanel(giAdjustmentPanelHandle);
	  
   

   }
   
   
   if (lTrace)
   {
      if (*pErrorOccurred)
      {
         sprintf(cTraceBuffer, "Error %ld : %s",  *pErrorCode, errorMessage);
         RESMGR_Trace(cTraceBuffer);
      }
      RESMGR_Trace("<<TSPAN_HideAdjustmentPanel end");
   }

}

void __stdcall tsadj_Cleanup(CAObjHandle sequenceContext, long pResourceId,
   short *pErrorOccurred, long *pErrorCode, char errorMessage[])
{
   char cTraceBuffer[1024];

   long lTrace;

   BENCH_STRUCT *pBench = NULL;

   lTrace = RESMGR_Get_Trace_Flag(pResourceId);

   if (lTrace)
   {
      RESMGR_Trace(">>TSADJ_Cleanup begin");
   }
   /*---------------------------------------------------------------------/
   /   Retrieve the memory pointer:
   /     Get a pointer to the memory block to check the configuration
   /---------------------------------------------------------------------*/
   RESMGR_Get_Mem_Ptr(sequenceContext, pResourceId, (void **)(&pBench),
      pErrorOccurred, pErrorCode, errorMessage);
   /*---------------------------------------------------------------------/
   /   Check for memory block owner:
   /     To be sure that the given resource ID belongs to the ENOCEAN
   /     library, we check the "owner" field of the memory block if it
   /     contains the "magic number" we have stored there in the
   /     ENOCEAN_Setup function
   /---------------------------------------------------------------------*/
   if (! *pErrorOccurred)
   {
      if (pBench->iOwner != TSPAN_ERR_BASE)
      {
         *pErrorOccurred = TRUE;
         *pErrorCode = GTSL_ERR_WRONG_RESOURCE_ID;
         formatError(errorMessage,  *pErrorCode, pResourceId, NULL);
      }
   }
   /*---------------------------------------------------------------------/
   /   Dispose memory:
   /     Free the memory block associated with the resource ID.
   /     Note that pBench is no longer valid now because it points
   /     to dynamic memory that has been released!
   /---------------------------------------------------------------------*/
   if (! *pErrorOccurred)
   {
      RESMGR_Free_Memory(sequenceContext, pResourceId, pErrorOccurred,
         pErrorCode, errorMessage);
   }
   pBench = NULL;

   /*---------------------------------------------------------------------/
   /   Free resource:
   /     The resource ID (our "ticket") is given back to the resource
   /     manager and may be reused in a subsequent RESMGR_Alloc_Resource
   /     call.
   /---------------------------------------------------------------------*/
   if (! *pErrorOccurred)
   {
      RESMGR_Free_Resource(sequenceContext, pResourceId, pErrorOccurred,
         pErrorCode, errorMessage);
   }
   if (! *pErrorOccurred)
   {
      if (lTrace)
      {
         sprintf(cTraceBuffer, "Free Resource ID %ld", pResourceId);
         RESMGR_Trace(cTraceBuffer);
      }
   }

   /*---------------------------------------------------------------------/
   /   Cleanup and error handling
   /---------------------------------------------------------------------*/
   if (lTrace)
   {
      if (*pErrorOccurred)
      {
         sprintf(cTraceBuffer, "Error %ld : %s",  *pErrorCode, errorMessage);
         RESMGR_Trace(cTraceBuffer);
      }
      RESMGR_Trace("<<TSADJ_Cleanup end");
   }

}

int StartThreadForAdjustmentPanel(void *data)
{

   ThreadData *threadData = (ThreadData*) data;
   
   CoInitializeEx(NULL, COINIT_MULTITHREADED);

   //  CoInitializeEx() is not declared in objbase.h which ships with CVI 5.0.
   //  You will need to declare this function yourself using the following declaration:
   //  WINOLEAPI  CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);

   //  You must also use objbase.lib which ships with VC++ 5.0.


   if ((giAdjustmentPanelHandle = LoadPanelEx(0, "testadjustmentpanel.uir",
      ADJUSTMENT, __CVIUserHInst)) < 0)
   {
      return  - 1;
   }

 
   DisplayPanel(giAdjustmentPanelHandle);
   
   //SetPanelAttribute(giActualPanelHandle, ATTR_TITLE, cLastNameOfStep);
   //SetCtrlAttribute(panelHandle, PANEL_INFO, ATTR_CTRL_VAL, cTextMessageIntern);
   //GetCtrlAttribute (giActualPanelHandle, OP_SINGLE_TESTCASEPATHWAY, ATTR_VISIBLE_LINES, &giVisibleLines);
   
   /*SetCtrlAttribute (giActualPanelHandle, OP_SINGLE_REPORT, ATTR_OFF_COLOR, VAL_PANEL_GRAY);
   SetCtrlAttribute (giActualPanelHandle, OP_SINGLE_TICKET, ATTR_OFF_COLOR, VAL_PANEL_GRAY);
   SetCtrlAttribute (giActualPanelHandle, OP_SINGLE_YIELD, ATTR_OFF_COLOR, VAL_PANEL_GRAY);
   SetCtrlAttribute (giActualPanelHandle, OP_SINGLE_REPORT, ATTR_ON_COLOR, VAL_DK_GREEN);
   SetCtrlAttribute (giActualPanelHandle, OP_SINGLE_TICKET, ATTR_ON_COLOR, VAL_DK_GREEN);
   SetCtrlAttribute (giActualPanelHandle, OP_SINGLE_YIELD, ATTR_ON_COLOR, VAL_DK_GREEN);   */

   // Use thread data in callbacks
   SetCtrlAttribute(giAdjustmentPanelHandle, ADJUSTMENT_OK, ATTR_CALLBACK_DATA, data);
   
   // Signal main thread that the new thread started
   SetEvent(hThreadReady);

   RunUserInterface();

   DiscardPanel(giAdjustmentPanelHandle);
   if (threadData->execution)
   {
      CA_DiscardObjHandle(threadData->execution);
      threadData->execution = 0;
   }
   if (threadData->ThisContext)
   {
      CA_DiscardObjHandle(threadData->ThisContext);
      threadData->ThisContext = 0;
   }
   Error: return 0;
}



/* FUNCTION *******************************************************************/
/**
formatError:    formats the error string.
formats the error string.
 *
@precondition       --
@postcondition      --
@side_effects       --
 *
@param buffer:      points to a buffer where the error message is stored
(size must be >= GTSL_ERROR_BUFFER_SIZE)
@param code:        Error code
@param resId:       Resource ID
@param benchDevice: Name of the bench device, may be NULL
 *
@return             void
 *******************************************************************************/
static void formatError(char buffer[], int code, long resId, char *benchDevice)
{
   char *pMsg = NULL;
   char resourceName[RESMGR_MAX_NAME_LENGTH + 1] = "";
   char tempMsg[GTSL_ERROR_BUFFER_SIZE] = "";
   short tempOcc = FALSE;
   long tempCode = 0;
   int written = 0;
   GTSL_ERROR_ENTRY *pErr = errorTable; /* pointer into error entry table */


   /* find the error message for a given error code */
   while (pErr->string != NULL)
   {
      if (pErr->value == code)
      {
         pMsg = pErr->string;
         break;
      }
      pErr++;
   }
   if (pMsg == NULL)
   {
      /* should never happen */
      pMsg = "(no message available for this code)";
   }

   /* setup the error message */

   /* 1) Library name */
   strcpy(buffer, GTSL_ERRMSG_PREFIX_LIBRARY);
   strcat(buffer, LIBRARY_NAME);
   strcat(buffer, "\n");

   /* 2) Bench name, only if a valid ID is given */
   if (resId != RESMGR_INVALID_ID)
   {
      /* read the resource name into a local buffer */
      RESMGR_Get_Resource_Name(0, resId, resourceName, sizeof(resourceName),
         &written, &tempOcc, &tempCode, tempMsg);
      if ((!tempOcc) && (written > 0))
      {
         /* append the name */
         strcat(buffer, GTSL_ERRMSG_PREFIX_BENCH);
         strcat(buffer, resourceName);
         strcat(buffer, "\n");
      }
   }

   /* 3) Bench device, if given */
   if (benchDevice != NULL)
   {
      strcat(buffer, GTSL_ERRMSG_PREFIX_BENCH_DEVICE);
      strcat(buffer, benchDevice);
      strcat(buffer, "\n");
   }

   /* 4) Error message */
   strcat(buffer, GTSL_ERRMSG_PREFIX_ERRMSG);
   strcat(buffer, pMsg);
}


void CVICALLBACK QuitThread(void *callbackData)
{
   QuitUserInterface(0);
}

void FormatValues (char cValue[1024], char *cFormat, double dValue)
{
	if(!strcmp(cFormat, "%i"))
	{
		//MessagePopup("bla", "Integer");
		sprintf(cValue, cFormat, (int) dValue);
		return;
	} else if(!strcmp(cFormat, "%u"))
	{
		//MessagePopup("bla", "unsigend Integer");
		sprintf(cValue, cFormat, (unsigned int) dValue);
		return;
	} else if(!strcmp(cFormat, "%#x"))
	{
		//MessagePopup("bla", "klein hex"); 
		sprintf(cValue, cFormat, (int) dValue);
		return;
	}
	 else if(!strcmp(cFormat, "%#X"))
	{
		//MessagePopup("bla", "groﬂ hex"); 
		sprintf(cValue, cFormat, (int) dValue);
		return;
	}  else if(!strcmp(cFormat, "%#o"))
	{
		//MessagePopup("bla", "octal"); 
		sprintf(cValue, cFormat, (int) dValue);
		return;
	} else
	{
		sprintf(cValue, cFormat, dValue);
		return;
	}
	
}
