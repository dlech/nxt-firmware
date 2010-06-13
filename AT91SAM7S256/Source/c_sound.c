//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: c_sound.c                                     $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_soun $
//
// Platform        C
//

#include  <stdlib.h>
#include  <string.h>
#include  "stdconst.h"
#include  "modules.h"
#include  "c_sound.iom"
#include  "c_loader.iom"
#include  "c_sound.h"
#include  "d_sound.h"

static    IOMAPSOUND   IOMapSound;
static    VARSSOUND    VarsSound;
static    HEADER       **pHeaders;

const     HEADER       cSound =
{
  0x00080001L,
  "Sound",
  cSoundInit,
  cSoundCtrl,
  cSoundExit,
  (void *)&IOMapSound,
  (void *)&VarsSound,
  (UWORD)sizeof(IOMapSound),
  (UWORD)sizeof(VarsSound),
  0x0000                      //Code size - not used so far
};


UWORD     cSoundFile(UBYTE Cmd,UBYTE *pFile,UBYTE *pData,ULONG *pLng)
{
  return (pMapLoader->pFunc(Cmd,pFile,pData,pLng));
}


void      cSoundInit(void* pHeader)
{
  pHeaders                    = pHeader;
  IOMapSound.Flags           &= ~SOUND_UPDATE;
  IOMapSound.Flags           &= ~SOUND_RUNNING;
  IOMapSound.State            =  SOUND_IDLE;
  IOMapSound.Mode             =  SOUND_ONCE;
  IOMapSound.Volume           =  SOUNDVOLUMESTEPS;
  IOMapSound.SampleRate       =  0;
  IOMapSound.SoundFilename[0] =  0;
  VarsSound.BufferIn          =  0;
  VarsSound.BufferOut         =  0;
  dSoundInit();
}

void      cSoundCtrl(void)
{
  static  UWORD FileFormat;
  static  UBYTE SoundFilename[FILENAME_LENGTH + 1];
  UWORD   Handle;
  ULONG   Length;
  UBYTE   Header[FILEHEADER_LENGTH];
  UBYTE   In,Out,Tmp;

  In  = VarsSound.BufferIn;
  Out = VarsSound.BufferOut;

  if ((IOMapSound.Flags & SOUND_UPDATE))
  {
// Check if valid update
    if (!(SOUND_TONE & IOMapSound.Mode))
    {
      Handle = pMapLoader->pFunc(FINDFIRST,IOMapSound.SoundFilename,SoundFilename,&Length);
      if (!(Handle & 0x8000))
      {
        pMapLoader->pFunc(CLOSE,(UBYTE*)&Handle,NULL,NULL);
      }
      else
      {
        IOMapSound.Flags   &= ~SOUND_UPDATE;
      }
    }
    if ((IOMapSound.Flags & SOUND_UPDATE))
    {
// Check for open file
      if (!(VarsSound.File & 0x8000))
      {
        cSoundFile(CLOSE,(UBYTE*)&VarsSound.File,NULL,NULL);
        VarsSound.File = 0x8000;
      }

      IOMapSound.Flags   &= ~SOUND_UPDATE;

      if ((SOUND_TONE & IOMapSound.Mode))
      {
        dSoundFreq(IOMapSound.Freq,IOMapSound.Duration,IOMapSound.Volume);
        IOMapSound.State  = SOUND_FREQ;
      }
      else
      {
        if (IOMapSound.Flags & SOUND_RUNNING)
        {
          dSoundStop();
          IOMapSound.Flags  &= ~SOUND_RUNNING;
        }
        VarsSound.File = pMapLoader->pFunc(OPENREAD,SoundFilename,NULL,&Length);
        if (!(VarsSound.File & 0x8000))
        {
          Length = FILEHEADER_LENGTH;
          pMapLoader->pFunc(READ,(UBYTE*)&VarsSound.File,Header,&Length);
          if (Length == FILEHEADER_LENGTH)
          {
            FileFormat = ((UWORD)Header[0] << 8) + (UWORD)Header[1];

            if (FILEFORMAT_SOUND == (FileFormat & 0xFF00))
            {
              if (IOMapSound.SampleRate)
              {
                VarsSound.SampleRate  = IOMapSound.SampleRate;
                IOMapSound.SampleRate = 0;
              }
              else
              {
                VarsSound.SampleRate  = ((UWORD)Header[4] << 8) + (UWORD)Header[5];
              }
              dSoundVolume(IOMapSound.Volume);
              Length = SOUNDBUFFERSIZE;
              pMapLoader->pFunc(READ,(UBYTE*)&VarsSound.File,VarsSound.Buffer[In],&Length);
              VarsSound.Length[In] = (UWORD)Length;
              In++;
              if (In >= SOUNDBUFFERS)
              {
                In = 0;
              }
              IOMapSound.State  = SOUND_BUSY;
            }
            else
            {
              if (FILEFORMAT_MELODY == FileFormat)
              {
                Length = SOUNDBUFFERSIZE;
                pMapLoader->pFunc(READ,(UBYTE*)&VarsSound.File,VarsSound.Buffer[In],&Length);
                VarsSound.Length[In] = (UWORD)Length;
                In++;
                if (In >= SOUNDBUFFERS)
                {
                  In = 0;
                }
                IOMapSound.State  = SOUND_BUSY;
              }
              else
              {
                pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsSound.File,NULL,NULL);
              }
            }
          }
        }
      }
    }
  }

  switch (IOMapSound.State)
  {
    case SOUND_BUSY :
    {
      IOMapSound.Flags |= SOUND_RUNNING;
      if (In != Out)
      {
        if ((FILEFORMAT_SOUND == FileFormat) || (FILEFORMAT_SOUND_COMPRESSED == FileFormat))
        {
          if (dSoundStart(VarsSound.Buffer[Out],VarsSound.Length[Out],VarsSound.SampleRate,(UBYTE)(FileFormat & 0x00FF)) == TRUE)
          {
            Out++;
            if (Out >= SOUNDBUFFERS)
            {
              Out = 0;
            }
          }
        }
        else
        {
          if (dSoundTone(VarsSound.Buffer[Out],VarsSound.Length[Out],IOMapSound.Volume) == TRUE)
          {
            Out++;
            if (Out >= SOUNDBUFFERS)
            {
              Out = 0;
            }
          }
        }
      }

      Tmp = In;
      Tmp++;
      if (Tmp >= SOUNDBUFFERS)
      {
        Tmp = 0;
      }

      if (Tmp != Out)
      {
        Tmp++;
        if (Tmp >= SOUNDBUFFERS)
        {
          Tmp = 0;
        }
        if (Tmp != Out)
        {
          Length = SOUNDBUFFERSIZE;
          Handle = cSoundFile(READ,(UBYTE*)&VarsSound.File,VarsSound.Buffer[In],&Length);
          if ((Handle & 0x8000))
          {
            Length = 0L;
          }
          VarsSound.Length[In] = (UWORD)Length;
          if (VarsSound.Length[In] == 0)
          {
            if (SOUND_LOOP == IOMapSound.Mode)
            {
              if (!(IOMapSound.Flags & SOUND_UPDATE))
              {
                cSoundFile(CLOSE,(UBYTE*)&VarsSound.File,NULL,NULL);
                VarsSound.File = cSoundFile(OPENREAD,SoundFilename,NULL,&Length);
                Length = FILEHEADER_LENGTH;
                cSoundFile(READ,(UBYTE*)&VarsSound.File,Header,&Length);
                Length = SOUNDBUFFERSIZE;
                cSoundFile(READ,(UBYTE*)&VarsSound.File,VarsSound.Buffer[In],&Length);
                VarsSound.Length[In] = (UWORD)Length;
              }
            }
          }
          if (VarsSound.Length[In] != 0)
          {
            In++;
            if (In >= SOUNDBUFFERS)
            {
              In = 0;
            }
          }
          if (VarsSound.Length[Out] == 0)
          {
            if (!(VarsSound.File & 0x8000))
            {
              pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsSound.File,NULL,NULL);
              VarsSound.File = 0x8000;
            }
            IOMapSound.Flags &= ~SOUND_RUNNING;
            IOMapSound.State  = SOUND_IDLE;
          }
        }
      }
    }
    break;

    case SOUND_FREQ :
    {
      IOMapSound.Flags |= SOUND_RUNNING;
      if (dSoundReady() == TRUE)
      {
        if (SOUND_LOOP & IOMapSound.Mode)
        {
          dSoundFreq(IOMapSound.Freq,IOMapSound.Duration,IOMapSound.Volume);
        }
        else
        {
          IOMapSound.Flags &= ~SOUND_RUNNING;
          IOMapSound.State  = SOUND_IDLE;
        }
      }
    }
    break;

    case SOUND_STOP :
    {
      dSoundStop();
      if (!(VarsSound.File & 0x8000))
      {
        pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsSound.File,NULL,NULL);
        VarsSound.File = 0x8000;
      }
      IOMapSound.Flags &= ~SOUND_RUNNING;
      IOMapSound.State  = SOUND_IDLE;
      Out = In;
    }
    break;

  }

  VarsSound.BufferIn  = In;
  VarsSound.BufferOut = Out;
}


void      cSoundExit(void)
{
  dSoundExit();
}
