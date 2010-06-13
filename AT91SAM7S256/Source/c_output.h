//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: c_output.h                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_outp $
//
// Platform        C
//

#ifndef   C_OUTPUT
#define   C_OUTPUT

typedef   struct
{
  UBYTE   TimeCnt;
  UBYTE   Tmp;
}VARSOUTPUT;

void      cOutputInit(void* pHeader);
void      cOutputCtrl(void);
void      cOutputExit(void);
void	  cOutputUpdateIomap(void);

extern    const HEADER cOutput;

#endif
