/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2009. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  ADJUSTMENT                       1
#define  ADJUSTMENT_INDICATOR             2
#define  ADJUSTMENT_OK                    3       /* callback function: AdjustmentCallback */
#define  ADJUSTMENT_UNIT                  4
#define  ADJUSTMENT_LL                    5
#define  ADJUSTMENT_UL                    6
#define  ADJUSTMENT_BACKGROUND            7
#define  ADJUSTMENT_TEXT                  8


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK AdjustmentCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
