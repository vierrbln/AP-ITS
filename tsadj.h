
/*******************************************************************************/
/**
@file tspan.h
*
@brief Exported function declaration
*
@version 1.0.0.0
*
@author Robert Renner <A HREF="mailto:robert.renner@spectral.de">robert.renner@spectral.de</A>\n
&copy; Spectral Electronic Forschungs- und Produktions GmbH - Berlin \n \n
language: ANSI-C ISO/IEC9899:1990
*
<b>History:</b>
- <b>23.11.2007 R. Renner</b>
- Initial revision

******************************************************************************
*****************************************************************************/
#ifndef TSADJ_H
#define TSADJ_H

/* INCLUDE FILES ***************************************************************/
#include <cvidef.h>
#include "gtslerr.h"   /* GTSL error handling */
#include "testadjustmentpanel.h"
#include <cvirte.h>

#include <windows.h>
#include <objbase.h>

#include <utility.h>
#include <userint.h>


#include <ansi_c.h>
#include <tsapicvi.h>
//#include "lc.h"
#include "toolbox.h"
#include <formatio.h>
#include <analysis.h>
#include <resmgr.h>

#include "definitions.h"
#include "testadjustmentpanel.h"
#include "hrestim.h" 

/* EXPORT */



/* GLOBAL TYPE DECLARATIONS ***************************************************/

/* GLOBAL CONSTANT DECLARATIONS ***********************************************/

/* Error codes */
/* User-specific error code region */
#define TSPAN_ERR_BASE                          -1004000
#define TSPAN_ERR_NOT_A_BENCH                   (TSPAN_ERR_BASE - 1)    /* -1004001 */
#define TSPAN_ERR_THREADWASNOTSTARTET           (TSPAN_ERR_BASE - 2)    /* -1004002 */
#define TSPAN_ERR_WRONGFORMATYTE                (TSPAN_ERR_BASE - 3)    /* -1004003 */

/* GLOBAL VARIABLE DECLARATIONS ***********************************************/

/* GLOBAL MACRO DEFINITIONS ***************************************************/

/* EXPORTED FUNCTIONS *********************************************************/

void __stdcall tsadj_Setup (CAObjHandle sequenceContext, char *benchName,
                           long *resourceID, short *errorOccurred,
                           long *errorCode, char errorMessage[]);


void __stdcall tsadj_DisplayAdjustmentPanel (CAObjHandle sequenceContext,
                                            long resourceID, char nameOfStep[], char buttonText[],
                                            char unit[], char compType[],
                                            char format[], double lowerLimit,
                                            double upperLimit, short AlwaysOnTop,
                                            short *errorOccurred,
                                            long *errorCode, char errorMessage[]);

void __stdcall tsadj_SetValueAdjustmentPanel (CAObjHandle sequenceContext,
                                             long resourceID, double value,
                                             short *errorOccurred,
                                             long *errorCode,
                                             char errorMessage[]);

void __stdcall tsadj_HideAdjustmentPanel (CAObjHandle sequenceContext,
                                         long resourceID, short *errorOccurred,
                                         long *errorCode, char errorMessage[]);

void __stdcall  tsadj_Cleanup (CAObjHandle sequenceContext, long resourceID,
                             short *errorOccurred, long *errorCode,
                             char errorMessage[]);

#endif   /* do not add code after this line */
/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
