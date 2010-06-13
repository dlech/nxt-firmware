//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_sound.r                                     $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_soun $
//
// Platform        C
//

#include  "d_sound_adpcm.r"

#ifdef    SAM7S256

#define   SAMPLEMIN                     0         // Must be zero (no pwm/interrupt)
#define   SAMPLEMAX                     256       // Must be 256 (8 bit wave format)
#define   SAMPLECENTER                  (((SAMPLEMAX - SAMPLEMIN) / 2) + SAMPLEMIN)

#define   SAMPLEWORD                    ULONG
#define   SAMPLEWORDS                   8
#define   SAMPLEWORDBITS                (sizeof(SAMPLEWORD) * 8)
#define   SAMPLEBITS                    (SAMPLEWORDS * SAMPLEWORDBITS)
#define   SAMPLECONSTANT                3         // >> == (SAMPLEMAX / SAMPLEWORDBITS)

#define   SAMPLETONENO                  16        // No of tone samples

#define   SAMPLEBUFFERS                 2

#define   INIT_PREV_VAL_ADPCM           0x7F
#define   INIT_INDEX_ADPCM              20

SAMPLEWORD SampleBuffer[SAMPLEBUFFERS][SAMPLEWORDS];
SAMPLEWORD ToneBuffer[SAMPLETONENO];

const     SAMPLEWORD TonePattern[SOUNDVOLUMESTEPS + 1][SAMPLETONENO] =
{
  {
    0xF0F0F0F0,0xF0F0F0F0,                        // Step 0 = silence
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0
  },
  {
    0xF0F0F0F0,0xF0F0F0F0,                        // Step 1 = 1/512
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F8,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0
  },
  {
    0xF0F0F0F0,0xF0F0F0F0,                        // Step 2 = 0,+3,+4,+3,0,-3,-4,-3
    0xF0F0F0F0,0xF0F8F8F8,
    0xF0F0F8F8,0xF8F8F0F0,
    0xF8F8F8F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0F0F0F0,
    0xF0F0F0F0,0xF0E0E0E0,
    0xF0F0E0E0,0xE0E0F0F0,
    0xE0E0E0F0,0xF0F0F0F0
  },
  {
    0xF0F0F0F0,0xF0F0F0F0,                        // Step 3 = 0,+10,+14,+10,0,-10,-14,-10
    0xF8F8F8F8,0xF8F8FCFC,
    0xF8F8FCFC,0xFCFCFCFC,
    0xFCFCF8F8,0xF8F8F8F8,
    0xF0F0F0F0,0xF0F0F0F0,
    0xE0E0E0E0,0xE0E0C0C0,
    0xE0E0C0C0,0xC0C0C0C0,
    0xC0C0E0E0,0xE0E0E0E0
  },
  {
    0xF0F0F0F0,0xF0F0F0F0,                        // Step 4 = 0,+22,+32,+22,0,-22,-32,-22
    0xFCFCFCFC,0xFCFCFDFD,
    0xFFFFFFFF,0xFFFFFFFF,
    0xFDFDFCFC,0xFCFCFCFC,
    0xF0F0F0F0,0xF0F0F0F0,
    0xC0C0C0C0,0xC0C08080,
    0x00000000,0x00000000,
    0x8080C0C0,0xC0C0C0C0
  }
};

UBYTE     FractionPattern[SAMPLEWORDS] =
{
  0x00,   // 0 -> 00000000
  0x10,   // 1 -> 00010000
  0x22,   // 2 -> 00100010
  0x4A,   // 3 -> 01001010
  0x55,   // 4 -> 01010101
  0x6D,   // 5 -> 01101101
  0x77,   // 6 -> 01110111
  0x7F,   // 7 -> 01111111
};

typedef   struct
{
    SWORD      Valprev;                           // Previous output value
    SWORD      Index;                             // Index into stepsize table
}ADPCM_State;

ULONG     ToneCycles;                             // No of tone cycles
ULONG     ToneCyclesReady;                        // No of tone cycles for ready
ULONG     ClockNext;                              // Serial clock for next buffer

UBYTE     *pSoundPointer;                         // Pointer to sample in actual sound buffer
UBYTE     *pSoundPointerNext;                     // Pointer to sample in next sound buffer

UWORD     SoundSamplesLeft;                       // Number of samples left on actual sound buffer
UWORD     SoundSamplesLeftNext;                   // Number of samples left on next sound buffer

UBYTE     SampleBufferNo;                         // Sample buffer no in use

UBYTE     SoundReady;                             // Sound channel ready (idle)
UBYTE     SoundDivider;                           // Volume

UWORD     MelodyPointer;
UBYTE     CurrentFileFormat;                      // Hold current playing file type

UBYTE     Outdata[2];                             // Output buffer used within the ADPCM algorithm
ADPCM_State State;                                // Struct holding ADPCM state

#define   SOUNDIntEnable                {\
                                          *AT91C_SSC_IER              = AT91C_SSC_ENDTX;\
                                        }

#define   SOUNDIntDisable               {\
                                          *AT91C_SSC_IDR              = AT91C_SSC_ENDTX;\
                                        }

#define   SOUNDEnable                   {\
                                          *AT91C_PIOA_PDR             = AT91C_PA17_TD;              /* Enable TD on PA17  */\
                                        }

#define   SOUNDDisable                  {\
                                          *AT91C_PIOA_PER             = AT91C_PA17_TD;              /* Disable TD on PA17  */\
                                        }

ULONG     SoundSampleRate(UWORD Rate)
{
  ULONG   Result;

  if (Rate > SAMPLERATE_MAX)
  {
    Rate = SAMPLERATE_MAX;
  }
  if (Rate < SAMPLERATE_MIN)
  {
    Rate = SAMPLERATE_MIN;
  }
  Result = ((OSC / (2 * SAMPLEBITS)) / Rate) + 1;

  return (Result);
}

__ramfunc void CalculateBitstream(SAMPLEWORD *pSampleBuffer,UBYTE Sample)
{
  ULONG   IntegerMask;
  ULONG   FractionMask;
  UBYTE   Integer;
  UBYTE   Fraction;
  UBYTE   Mask;
  UBYTE   Tmp;
  SWORD   STmp;

  if (SoundDivider)
  {
    STmp        = Sample;
    STmp       &= 0xFF;
    STmp       -= SAMPLECENTER;
    STmp      >>= (SOUNDVOLUMESTEPS - SoundDivider);
    STmp       += SAMPLECENTER;
    Sample      = (UBYTE)STmp;
    SOUNDEnable;
  }
  else
  {
    SOUNDDisable;
  }

  Tmp = 0;
  IntegerMask   = 0xFFFF0000;
  Integer       = Sample >> SAMPLECONSTANT;
  Fraction      = Sample - (Integer << SAMPLECONSTANT);
  IntegerMask   = 0xFFFFFFFF << (SAMPLEWORDBITS - Integer);
  FractionMask  = (IntegerMask >> 1) | IntegerMask;
  Mask          = FractionPattern[Fraction];
  while (Tmp < SAMPLEWORDS)
  {
    if ((Mask & (0x01 << Tmp)))
    {
      *pSampleBuffer = FractionMask;
    }
    else
    {
      *pSampleBuffer = IntegerMask;
    }
    pSampleBuffer++;
    Tmp++;
  }
}

__ramfunc void SscHandler(void)
{
  static UBYTE ByteCnt = 0;

  if (SoundSamplesLeft)
  {
    if (0 == CurrentFileFormat)
    {
      CalculateBitstream(SampleBuffer[SampleBufferNo],*pSoundPointer);
      *AT91C_SSC_TNPR = (unsigned int)SampleBuffer[SampleBufferNo];
      *AT91C_SSC_TNCR = SAMPLEWORDS;

      pSoundPointer++;
      SoundSamplesLeft--;
      if (!SoundSamplesLeft)
      {
        pSoundPointer         = pSoundPointerNext;
        SoundSamplesLeft      = SoundSamplesLeftNext;
        *AT91C_SSC_CMR        = ClockNext;
        SoundSamplesLeftNext  = 0;
      }

      if (++SampleBufferNo >= SAMPLEBUFFERS)
      {
        SampleBufferNo = 0;
      }
    }
    else
    {
      if (0 == ByteCnt)
      {
        SoundADPCMDecoder(*pSoundPointer, Outdata, &State.Valprev, &State.Index);
        CalculateBitstream(SampleBuffer[SampleBufferNo],Outdata[0]);
        *AT91C_SSC_TNPR  = (unsigned int)SampleBuffer[SampleBufferNo];
        *AT91C_SSC_TNCR  = SAMPLEWORDS;

        if (++SampleBufferNo >= SAMPLEBUFFERS)
        {
          SampleBufferNo = 0;
        }

        ByteCnt++;
      }
      else
      {
        CalculateBitstream(SampleBuffer[SampleBufferNo],Outdata[1]);
        *AT91C_SSC_TNPR  = (unsigned int)SampleBuffer[SampleBufferNo];
        *AT91C_SSC_TNCR  = SAMPLEWORDS;

        pSoundPointer++;
        SoundSamplesLeft--;
        if (!SoundSamplesLeft)
        {
          pSoundPointer         = pSoundPointerNext;
          SoundSamplesLeft      = SoundSamplesLeftNext;
          *AT91C_SSC_CMR        = ClockNext;
          SoundSamplesLeftNext  = 0;
        }

        if (++SampleBufferNo >= SAMPLEBUFFERS)
        {
          SampleBufferNo = 0;
        }
        ByteCnt = 0;
      }
    }
  }
  else
  {
    if (ToneCycles)
    {
      ToneCycles--;
      if (ToneCycles < ToneCyclesReady)
      {
        SoundReady    = TRUE;
      }
      *AT91C_SSC_TNPR = (unsigned int)ToneBuffer;
      *AT91C_SSC_TNCR = SAMPLETONENO;
      if (SoundDivider)
      {
        SOUNDEnable;
      }
      else
      {
        SOUNDDisable;
      }
    }
    else
    {
      SoundReady      = TRUE;
      SOUNDDisable;
      SOUNDIntDisable;
    }
  }
}

UBYTE     SoundStart(UBYTE *Sound,UWORD Length,UWORD SampleRate, UBYTE NewFileFormat)
{
  UBYTE   Result = FALSE;

  if (SoundReady == TRUE)
  {
    if (Length > 1)
    {
      CurrentFileFormat = NewFileFormat;
      *AT91C_SSC_CMR   = SoundSampleRate(SampleRate);
      pSoundPointer    = Sound;
      SoundSamplesLeft = Length;

      if (0 == CurrentFileFormat)
      {
        CalculateBitstream(SampleBuffer[0],*pSoundPointer);
        *AT91C_SSC_TPR   = (unsigned int)SampleBuffer[0];
        *AT91C_SSC_TCR   = SAMPLEWORDS;
        pSoundPointer++;
        SoundSamplesLeft--;
        CalculateBitstream(SampleBuffer[1],*pSoundPointer);
        *AT91C_SSC_TNPR  = (unsigned int)SampleBuffer[1];
        *AT91C_SSC_TNCR  = SAMPLEWORDS;
        pSoundPointer++;
        SoundSamplesLeft--;
      }
      else
      {
        State.Valprev = INIT_PREV_VAL_ADPCM;
        State.Index = INIT_INDEX_ADPCM;
        SoundADPCMDecoder(*pSoundPointer, Outdata, &State.Valprev, &State.Index);
        CalculateBitstream(SampleBuffer[0],Outdata[0]);
        *AT91C_SSC_TPR   = (unsigned int)SampleBuffer[0];
        *AT91C_SSC_TCR   = SAMPLEWORDS;
        pSoundPointer++;
        SoundSamplesLeft--;
        CalculateBitstream(SampleBuffer[1],Outdata[1]);
        *AT91C_SSC_TNPR  = (unsigned int)SampleBuffer[1];
        *AT91C_SSC_TNCR  = SAMPLEWORDS;
      }
      SampleBufferNo   = 0;
      SoundReady       = FALSE;
      SOUNDIntEnable;
      *AT91C_SSC_PTCR  = AT91C_PDC_TXTEN;
    }
    Result           = TRUE;
  }
  else
  {
    if (!ToneCycles)
    {
      if (!SoundSamplesLeftNext)
      {
        CurrentFileFormat     = NewFileFormat;
        ClockNext             = SoundSampleRate(SampleRate);
        pSoundPointerNext     = Sound;
        SoundSamplesLeftNext  = Length;
        Result                = TRUE;
      }
    }
  }

  return (Result);
}

UBYTE     SoundStop(void)
{
  ToneCycles            = 0;
  SOUNDIntDisable;
  SOUNDDisable;
  SoundReady            = TRUE;
  SoundSamplesLeft      = 0;
  SoundSamplesLeftNext  = 0;
  MelodyPointer         = 0;

  return (TRUE);
}

void      SoundVolume(UBYTE Step)
{
  if (Step > SOUNDVOLUMESTEPS)
  {
    Step = SOUNDVOLUMESTEPS;
  }
  SoundDivider = Step;
}

void      SoundFreq(UWORD Freq,UWORD mS,UBYTE Step)
{
  UBYTE   Tmp;

  if (mS < DURATION_MIN)
  {
    mS = DURATION_MIN;
  }
  if (Freq)
  {
    if (Freq < FREQUENCY_MIN)
    {
      Freq = FREQUENCY_MIN;
    }
    if (Freq > FREQUENCY_MAX)
    {
      Freq = FREQUENCY_MAX;
    }
    if (Step > SOUNDVOLUMESTEPS)
    {
      Step = SOUNDVOLUMESTEPS;
    }
  }
  else
  {
    Step = 0;
    Freq = 1000;
  }
  SoundDivider          = Step;
  SoundSamplesLeft      = 0;
  SoundSamplesLeftNext  = 0;
  for (Tmp = 0;Tmp < SAMPLETONENO;Tmp++)
  {
    ToneBuffer[Tmp] = TonePattern[Step][Tmp];
  }

  *AT91C_SSC_CMR   = (((ULONG)OSC / (2L * 512L)) / ((ULONG)Freq)) + 1L;
  ToneCycles       = ((ULONG)Freq * (ULONG)mS) / 1000L - 1L;
  ToneCyclesReady  = ((ULONG)Freq * (ULONG)2L) / 1000L + 1L;

  *AT91C_SSC_TNPR  = (unsigned int)ToneBuffer;
  *AT91C_SSC_TNCR  = SAMPLETONENO;
  *AT91C_SSC_PTCR  = AT91C_PDC_TXTEN;
  SoundReady       = FALSE;
  SOUNDIntEnable;
}

UBYTE     SoundTone(UBYTE *pMel,UWORD Length,UBYTE Step)
{
  UBYTE   Result = FALSE;
  UWORD   Freq;
  UWORD   mS;

  if ((SoundReady == TRUE))
  {
    if (MelodyPointer <= (Length - 4))
    {
      Freq    = (UWORD)pMel[MelodyPointer++] << 8;
      Freq   += (UWORD)pMel[MelodyPointer++];
      mS      = (UWORD)pMel[MelodyPointer++] << 8;
      mS     += (UWORD)pMel[MelodyPointer++];
      SoundFreq(Freq,mS,Step);
    }
    else
    {
      MelodyPointer = 0;
      Result        = TRUE;
    }
  }

  return (Result);
}

#define   SOUNDInit                     {\
                                          SOUNDIntDisable;\
                                          SoundReady                  = TRUE;\
                                          MelodyPointer               = 0;\
                                          *AT91C_PMC_PCER             = (1L << AT91C_ID_SSC);       /* Enable MCK clock   */\
                                          *AT91C_PIOA_PER             = AT91C_PA17_TD;              /* Disable TD on PA17  */\
                                          *AT91C_PIOA_ODR             = AT91C_PA17_TD;\
                                          *AT91C_PIOA_OWDR            = AT91C_PA17_TD;\
                                          *AT91C_PIOA_MDDR            = AT91C_PA17_TD;\
                                          *AT91C_PIOA_PPUDR           = AT91C_PA17_TD;\
                                          *AT91C_PIOA_IFDR            = AT91C_PA17_TD;\
                                          *AT91C_PIOA_CODR            = AT91C_PA17_TD;\
                                          *AT91C_PIOA_IDR             = AT91C_PA17_TD;\
                                          *AT91C_SSC_CR               = AT91C_SSC_SWRST;\
                                          AT91C_AIC_SVR[AT91C_ID_SSC] = (unsigned int)SscHandler;\
                                          AT91C_AIC_SMR[AT91C_ID_SSC] = AT91C_AIC_PRIOR_LOWEST | AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED;     /* Set priority       */\
                                          *AT91C_SSC_TCMR             = AT91C_SSC_CKS_DIV + AT91C_SSC_CKO_CONTINOUS + AT91C_SSC_START_CONTINOUS;\
                                          *AT91C_SSC_TFMR             = (SAMPLEWORDBITS - 1) + ((SAMPLEWORDS & 0xF) << 8) + AT91C_SSC_MSBF;\
                                          *AT91C_SSC_CR               = AT91C_SSC_TXEN;             /* TX enable          */\
                                          *AT91C_AIC_ICCR             = (1L << AT91C_ID_SSC);       /* Clear interrupt    */\
                                          *AT91C_AIC_IECR             = (1L << AT91C_ID_SSC);       /* Enable int. controller */\
                                        }

#define   SOUNDVolume(V)                SoundVolume((UBYTE)V)

#define   SOUNDReady                    SoundReady

#define   SOUNDStart(pSnd,Lng,SR,FT)    SoundStart(pSnd,Lng,SR,FT)

#define   SOUNDStop                     SoundStop()

#define   SOUNDTone(pMel,Lng,Vol)       SoundTone(pMel,Lng,Vol)

#define   SOUNDFreq(Freq,Duration,Vol)  SoundFreq(Freq,Duration,Vol)

#define   SOUNDExit                     {\
                                          SOUNDIntDisable;\
                                          SOUNDDisable;\
                                          *AT91C_AIC_IDCR             = (1L << AT91C_ID_SSC);\
                                        }


#endif
