//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: c_sound.h                                     $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_soun $
//
// Platform        C
//


#ifndef   C_SOUND
#define   C_SOUND

#define   SOUNDBUFFERSIZE               64    // Flash Sector size ?
#define   SOUNDBUFFERS                  3     // Min 3 - max 255


typedef   struct
{
  UWORD   Length[SOUNDBUFFERS];
  UWORD   File;
  UWORD   SampleRate;
  UBYTE   Buffer[SOUNDBUFFERS][SOUNDBUFFERSIZE];
  UBYTE   BufferIn;
  UBYTE   BufferOut;
  UBYTE   BufferTmp;
}VARSSOUND;

void      cSoundInit(void* pHeaders);
void      cSoundCtrl(void);
void      cSoundExit(void);

extern    const HEADER cSound;

#endif
