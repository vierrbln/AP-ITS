#include "tsapicvi.h"
#include <analysis.h>
#include <ansi_c.h>
#include "definitions.h"
#include <userint.h>
#include "testadjustmentpanel.h"
#include "hrestim.h"

typedef struct threadDataRec
{
   CAObjHandle execution;
   CAObjHandle ThisContext;
} ThreadData;

//-------------------------------------------------------------------------

int CVICALLBACK AdjustmentCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
   
   ThreadData *threadData;
   ERRORINFO pTSErrorInfo;
   threadData = (ThreadData*)callbackData;
   if (event == EVENT_COMMIT)
   {
	   TS_PropertySetValBoolean(threadData->ThisContext, &pTSErrorInfo,
	      "Locals.AdjustmentPanelButtonHit", TS_PropOption_InsertIfMissing, VTRUE);
   }
   return 0;
   
}
