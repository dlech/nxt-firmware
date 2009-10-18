//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 12:13                                    $
//
// Filename        $Workfile:: c_output.h                                    $
//
// Version         $Revision:: 6                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/c_output.h $
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
