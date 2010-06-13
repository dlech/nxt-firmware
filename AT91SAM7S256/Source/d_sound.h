//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_sound.h                                     $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_soun $
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
