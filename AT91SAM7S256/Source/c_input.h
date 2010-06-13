//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-01-09 10:33                                    $
//
// Filename        $Workfile:: c_input.h                                     $
//
// Version         $Revision:: 7                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_inpu $
//
// Platform        C
//

#ifndef   C_INPUT
#define   C_INPUT

#ifdef    INCLUDE_OS
extern    const HEADER cInput;
#endif

#include  "c_input.iom"

#define   ACTUAL_AD_RES                 1023L
#define   SENSOR_RESOLUTION             1023L
#define   DEBOUNCERELOAD                100
#define   THRESHOLD_FALSE               (UWORD)(ACTUAL_AD_RES * 45L / 100L)
#define   THRESHOLD_TRUE                (UWORD)(ACTUAL_AD_RES * 55L / 100L)

#define   ANGLELIMITA                   (UWORD)(ACTUAL_AD_RES * 4400L / 10000L)
#define   ANGLELIMITB                   (UWORD)(ACTUAL_AD_RES * 6600L / 10000L)
#define   ANGLELIMITC                   (UWORD)(ACTUAL_AD_RES * 8900L / 10000L)

#define   FWDDIR                        1
#define   RWDDIR                        2
#define   MAXSAMPLECNT                  5

typedef   struct
{
  UBYTE   ColorInputDebounce [NO_OF_COLORS];
  UBYTE   ColorEdgeCnt       [NO_OF_COLORS];
  UBYTE   ColorLastAngle     [NO_OF_COLORS];
  UBYTE   ColorSampleCnt     [NO_OF_COLORS];
  UBYTE   ColorInitState;
  UBYTE   ReadCnt;
} VARSCOLOR;


typedef   struct
{
  UWORD     InvalidTimer  [NO_OF_INPUTS];
  UBYTE     InputDebounce [NO_OF_INPUTS];
  UBYTE     EdgeCnt       [NO_OF_INPUTS];
  UBYTE     LastAngle     [NO_OF_INPUTS];
  UBYTE     OldSensorType [NO_OF_INPUTS];
  UBYTE     SampleCnt     [NO_OF_INPUTS];
  VARSCOLOR VarsColor     [NO_OF_INPUTS];
  UBYTE     ColorCnt;
  UBYTE     ColorStatus;
}VARSINPUT;

void      cInputInit(void* pHeader);
void      cInputCtrl(void);
void      cInputExit(void);


#endif
