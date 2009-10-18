//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkflebun                                        $
//
// Revision date   $Date:: 5-02-07 13:36                                     $
//
// Filename        $Workfile:: d_sound.h                                     $
//
// Version         $Revision:: 10                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_sound.h  $
//
// Platform        C
//


#ifndef   D_SOUND
#define   D_SOUND

void      dSoundInit(void);
void      dSoundVolume(UBYTE Step);
UBYTE     dSoundReady(void);
UBYTE     dSoundStart(UBYTE *pSound,UWORD Length,UWORD SampleRate, UBYTE FileFormat);
UBYTE     dSoundStop(void);
UBYTE     dSoundTone(UBYTE *pMelody,UWORD Length,UBYTE Volume);
void      dSoundFreq(UWORD Hz,UWORD mS,UBYTE Volume);
void      dSoundExit(void);

#define   SOUNDVOLUMESTEPS              4

#define   DURATION_MIN                  10        // [mS]
#define   FREQUENCY_MIN                 220       // [Hz]
#define   FREQUENCY_MAX                 14080     // [Hz]

#define   SAMPLERATE_MIN                2000      // Min sample rate [sps]
#define   SAMPLERATE_DEFAULT            8000      // Default sample rate [sps]
#define   SAMPLERATE_MAX                16000     // Max sample rate [sps]

#endif
