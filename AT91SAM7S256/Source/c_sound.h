//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 26-08-05 8:37                                     $
//
// Filename        $Workfile:: c_sound.h                                     $
//
// Version         $Revision:: 10                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/c_sound.h  $
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
