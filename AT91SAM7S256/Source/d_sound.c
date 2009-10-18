//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkflebun                                        $
//
// Revision date   $Date:: 5-02-07 13:36                                     $
//
// Filename        $Workfile:: d_sound.c                                     $
//
// Version         $Revision:: 11                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_sound.c  $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_sound.h"
#include  "d_sound.r"


void      dSoundInit(void)
{
  SOUNDInit;
}


void      dSoundVolume(UBYTE Step)
{
  SOUNDVolume(Step);
}


UBYTE     dSoundReady(void)
{
  return (SOUNDReady);
}


UBYTE     dSoundStart(UBYTE *pSound,UWORD Length,UWORD SampleRate, UBYTE FileType)
{
  return (SOUNDStart(pSound,Length,SampleRate,FileType));
}


UBYTE     dSoundStop(void)
{
  return (SOUNDStop);
}


UBYTE     dSoundTone(UBYTE *pMelody,UWORD Length,UBYTE Volume)
{
  return (SOUNDTone(pMelody,Length,Volume));
}


void      dSoundFreq(UWORD Hz,UWORD mS,UBYTE Volume)
{
  SOUNDFreq(Hz,mS,Volume);
}


void      dSoundExit(void)
{
  SOUNDExit;
}
