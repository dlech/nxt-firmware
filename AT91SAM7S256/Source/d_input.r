//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-01-09 10:33                                    $
//
// Filename        $Workfile:: d_input.r                                     $
//
// Version         $Revision:: 24                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_inpu $
//
// Platform        C
//


#ifdef    SAM7S256

void      rInputWaitUS(UBYTE usec);

void      rInputWait2uS(void);
void      rInputWait20uS(void);
void      rInputWait30uS(void);
void      rInputSingleADC(UBYTE Port, UWORD *Val);

const     ULONG  Digi0Alloc[] = {AT91C_PIO_PA23, AT91C_PIO_PA28, AT91C_PIO_PA29, AT91C_PIO_PA30};
const     ULONG  Digi1Alloc[] = {AT91C_PIO_PA18, AT91C_PIO_PA19, AT91C_PIO_PA20, AT91C_PIO_PA2};
const     ULONG  ADPinDef[NO_OF_INPUTS] = {AT91C_ADC_CH1, AT91C_ADC_CH2, AT91C_ADC_CH3, AT91C_ADC_CH7};
unsigned int volatile* ADValRegs[NO_OF_INPUTS] = {AT91C_ADC_CDR1, AT91C_ADC_CDR2, AT91C_ADC_CDR3, AT91C_ADC_CDR7};

static    UBYTE ColorReset[NO_OF_INPUTS];
static    ULONG ColorClkDef;
static    ULONG ColorTimer[NO_OF_INPUTS];

#define   TIME2US                       ((OSC/16)/500000L)
#define   TIME20US                      ((OSC/16)/50000L)
#define   TIME30US                      ((OSC/16)/33333L)
#define   TIME100MS                     ((OSC/16)/10L)

#define   MAX_AD_VALUE                  0x3FF

#define   INPUTInit                     {\
                                          UBYTE Tmp;                                  \
                                          for (Tmp = 0; Tmp < NOS_OF_AVR_INPUTS; Tmp++)\
                                          {                                           \
                                            IoFromAvr.AdValue[Tmp]  = MAX_AD_VALUE;   \
                                          }                                           \
                                          IoToAvr.InputPower = 0;                     \
                                          for (Tmp = 0; Tmp < NO_OF_INPUTS; Tmp++)    \
                                          {                                           \
                                            *AT91C_PIOA_PPUDR  = Digi0Alloc[Tmp];     \
                                            *AT91C_PIOA_PPUDR  = Digi1Alloc[Tmp];     \
                                            INPUTSetInDigi0(Tmp);                     \
                                            INPUTSetInDigi1(Tmp);                     \
                                            ColorReset[Tmp] = FALSE;                  \
                                          }                                           \
                                          ColorClkDef = 0;                            \
                                        }

#define   INPUTGetVal(pValues, No)      *pValues  = (UWORD)IoFromAvr.AdValue[No];     \
                                        *pValues &= 0x03FF

#define   INPUTSetActive(Input)         IoToAvr.InputPower |=  (0x01 << Input);       \
                                        IoToAvr.InputPower &= ~(0x10 << Input)
#define   INPUTSet9v(Input)             IoToAvr.InputPower |=  (0x10 << Input);       \
                                        IoToAvr.InputPower &= ~(0x01 << Input)
#define   INPUTSetInactive(Input)       IoToAvr.InputPower &= ~(0x11 << Input)

#define   INPUTSetOutDigi0(Input)       *AT91C_PIOA_PER  = Digi0Alloc[Input];         \
                                        *AT91C_PIOA_OER  = Digi0Alloc[Input]

#define   INPUTSetOutDigi1(Input)       *AT91C_PIOA_PER  = Digi1Alloc[Input];         \
                                        *AT91C_PIOA_OER  = Digi1Alloc[Input]

#define   INPUTSetInDigi0(Input)        *AT91C_PIOA_PER  = Digi0Alloc[Input];         \
                                        *AT91C_PIOA_ODR  = Digi0Alloc[Input]

#define   INPUTSetInDigi1(Input)        *AT91C_PIOA_PER  = Digi1Alloc[Input];         \
                                        *AT91C_PIOA_ODR  = Digi1Alloc[Input]

#define   INPUTSetDigi0(Input)          *AT91C_PIOA_SODR = Digi0Alloc[Input]

#define   INPUTSetDigi1(Input)          *AT91C_PIOA_SODR = Digi1Alloc[Input]

#define   INPUTClearDigi0(Input)        *AT91C_PIOA_CODR = Digi0Alloc[Input]

#define   INPUTClearDigi1(Input)        *AT91C_PIOA_CODR = Digi1Alloc[Input]

#define   INPUTReadDigi0(Input, Data)   if ((*AT91C_PIOA_PDSR) & Digi0Alloc[Input])   \
                                        {                                             \
                                          *Data |= 0x00000001;                        \
                                        }                                             \
                                        else                                          \
                                        {                                             \
                                          *Data &= ~0x00000001;                       \
                                        }
#define   INPUTReadDigi1(Input, Data)   if ((*AT91C_PIOA_PDSR) & Digi1Alloc[Input])   \
                                        {                                             \
                                          *Data |= 0x00000002;                        \
                                        }                                             \
                                        else                                          \
                                        {                                             \
                                          *Data &= ~0x00000002;                       \
                                        }

#define   INPUTClkHigh(Port)            INPUTSetDigi0(Port);                          \
                                        INPUTSetOutDigi0(Port);                       \
                                        rInputWait2uS()

#define   INPUTClkLow(Port)             INPUTClearDigi0(Port);                        \
                                        INPUTSetOutDigi0(Port);                       \
                                        rInputWait2uS()

#define   COLORClkInput                 *AT91C_PIOA_ODR  = ColorClkDef

#define   UPDATEAllColors(Vals, Status){\
                                          ULONG ADDef;                                \
                                          ADDef = 0;                                  \
                                          ColorClkDef = 0;                            \
                                          if (0x01 & Status)                          \
                                          {                                           \
                                            ADDef |= ADPinDef[0];                     \
                                            ColorClkDef |= Digi0Alloc[0];             \
                                            if ((*AT91C_PIOA_PDSR) & Digi0Alloc[0])   \
                                            {                                         \
                                              ColorReset[0] = TRUE;                   \
                                            }                                         \
                                          }                                           \
                                          if (0x02 & Status)                          \
                                          {                                           \
                                            ADDef |= ADPinDef[1];                     \
                                            ColorClkDef |= Digi0Alloc[1];             \
                                            if ((*AT91C_PIOA_PDSR) & Digi0Alloc[1])   \
                                            {                                         \
                                              ColorReset[1] = TRUE;                   \
                                            }                                         \
                                          }                                           \
                                          if (0x04 & Status)                          \
                                          {                                           \
                                            ADDef |= ADPinDef[2];                     \
                                            ColorClkDef |= Digi0Alloc[2];             \
                                            if ((*AT91C_PIOA_PDSR) & Digi0Alloc[2])   \
                                            {                                         \
                                              ColorReset[2] = TRUE;                   \
                                            }                                         \
                                          }                                           \
                                          if (0x08 & Status)                          \
                                          {                                           \
                                            ADDef |= ADPinDef[3];                     \
                                            ColorClkDef |= Digi0Alloc[3];             \
                                            if ((*AT91C_PIOA_PDSR) & Digi0Alloc[3])   \
                                            {                                         \
                                              ColorReset[3] = TRUE;                   \
                                            }                                         \
                                          }                                           \
                                          *AT91C_PIOA_OER  = ColorClkDef;             \
                                          *AT91C_ADC_CHER = ADDef;                    \
                                          GetAdVals(Vals, BLANK, Status);             \
                                          *AT91C_PIOA_SODR = ColorClkDef;             \
                                          rInputWait20uS();                           \
                                          GetAdVals(Vals, RED, Status);               \
                                          *AT91C_PIOA_CODR = ColorClkDef;             \
                                          rInputWait20uS();                           \
                                          GetAdVals(Vals, GREEN, Status);             \
                                          *AT91C_PIOA_SODR = ColorClkDef;             \
                                          rInputWait20uS();                           \
                                          GetAdVals(Vals, BLUE, Status);              \
                                          *AT91C_PIOA_CODR = ColorClkDef;             \
                                          *AT91C_ADC_CHDR  = ADDef;                   \
                                        }

#define   UPDATELed(Port, Col, Status)  {                                             \
                                          rInputSingleADC(Port, Col);                 \
                                          if ((*AT91C_PIOA_PDSR) & Digi0Alloc[Port])  \
                                          {                                           \
                                            ColorReset[Port] = TRUE;                  \
                                          }                                           \
                                          CHECKColorState(Port, Status);              \
                                        }

#define   COLORTx(Port, Data)           {                                             \
                                          UBYTE BitCnt;                               \
                                          BitCnt = 0;                                 \
                                          while(BitCnt++ < 8)                         \
                                          {                                           \
                                            INPUTClkHigh(Port);                       \
                                            if (Data & 0x01)                          \
                                            {                                         \
                                              INPUTSetDigi1(Port);                    \
                                            }                                         \
                                            else                                      \
                                            {                                         \
                                              INPUTClearDigi1(Port);                  \
                                            }                                         \
                                            rInputWait30uS();                         \
                                            Data >>= 1;                               \
                                            INPUTClkLow(Port);                        \
                                            rInputWait30uS();                         \
                                          }                                           \
                                        }

#define   CALDataRead(Port, pData)      {\
                                          UBYTE BitCnt;                               \
                                          UBYTE Data;                                 \
                                          BitCnt = 0;                                 \
                                          INPUTClkHigh(Port);                         \
                                          rInputWait2uS();                            \
                                          while(BitCnt++ < 8)                         \
                                          {                                           \
                                            INPUTClkHigh(Port);                       \
                                            rInputWait2uS();                          \
                                            rInputWait2uS();                          \
                                            INPUTClkLow(Port);                        \
                                            Data >>= 1;                               \
                                            if ((*AT91C_PIOA_PDSR) & Digi1Alloc[Port])\
                                            {                                         \
                                              Data |= 0x80;                           \
                                            }                                         \
                                            rInputWait2uS();                          \
                                          }                                           \
                                          *pData = Data;                              \
                                        }

#define   CHECKColorState(Port, Status) {\
                                          Status = TRUE;                              \
                                          if ((IoFromAvr.AdValue[Port] > 50) || (TRUE == ColorReset[Port])) \
                                          {                                           \
                                            Status = FALSE;                           \
                                            ColorReset[Port] = FALSE;                 \
                                          }                                           \
                                        }


#define   INPUTExit                     {                                             \
                                          UBYTE Tmp;                                  \
                                          *AT91C_ADC_CHDR = (AT91C_ADC_CH1 | AT91C_ADC_CH2 | AT91C_ADC_CH3 | AT91C_ADC_CH7);\
                                          for (Tmp = 0; Tmp < NO_OF_INPUTS; Tmp++)    \
                                          {                                           \
                                            INPUTSetInDigi0(Tmp);                     \
                                            INPUTSetInDigi1(Tmp);                     \
                                          }                                           \
                                        }
                                        
#define   CLEARColor100msTimer(No)      ColorTimer[No] = (*AT91C_PITC_PIIR);\

#define   COLOR100msStatus(No,V)        V = FALSE;\
                                        if (((*AT91C_PITC_PIIR) - ColorTimer[No]) > TIME100MS)\
                                        {\
                                          V = TRUE;\
                                        }
                                        


void      rInputSingleADC(UBYTE Port, UWORD *Val)
{
  *Val = *AT91C_ADC_LCDR;
  *AT91C_ADC_CHER = ADPinDef[Port];
  ADStart;
  while(!((*AT91C_ADC_SR) & AT91C_ADC_DRDY));
  *Val = *AT91C_ADC_LCDR;
  *AT91C_ADC_CHDR = ADPinDef[Port];
}

void    GetAdVals(COLORSTRUCT *pColStruct, UBYTE Color, UBYTE Status)
{
  UBYTE ChCnt;
  ADStart;
  for(ChCnt = 0; ChCnt < NO_OF_INPUTS; ChCnt++)
  {
    if (Status & (0x01 << ChCnt))
    {
      while(!((*AT91C_ADC_SR) & ADPinDef[ChCnt]));
      pColStruct[ChCnt].ADRaw[Color] = *ADValRegs[ChCnt];
    }
  }
  ADStart;
  for(ChCnt = 0; ChCnt < NO_OF_INPUTS; ChCnt++)
  {
    if (Status & (0x01 << ChCnt))
    {
      while(!((*AT91C_ADC_SR) & ADPinDef[ChCnt]));
      pColStruct[ChCnt].ADRaw[Color] += *ADValRegs[ChCnt];
      pColStruct[ChCnt].ADRaw[Color]  = (pColStruct[ChCnt].ADRaw[Color])>>1;
    }
  }
}

void      rInputWaitUS(UBYTE usec)
{
  // OSC = 48054850L
  ULONG Count = (OSC/16)/(1000000L/usec);
  ULONG PitTmr = (*AT91C_PITC_PIIR);
  while (((*AT91C_PITC_PIIR) - PitTmr) < Count);
}

void      rInputWait2uS(void)
{
  ULONG PitTmr;
  PitTmr = (*AT91C_PITC_PIIR);
  while (((*AT91C_PITC_PIIR) - PitTmr) < TIME2US);
}

void      rInputWait20uS(void)
{
  ULONG PitTmr;
  PitTmr = (*AT91C_PITC_PIIR);
  while (((*AT91C_PITC_PIIR) - PitTmr) < TIME20US);
}

void      rInputWait30uS(void)
{
  ULONG PitTmr;
  PitTmr = (*AT91C_PITC_PIIR);
  while (((*AT91C_PITC_PIIR) - PitTmr) < TIME30US);
}

#endif

#ifdef    PCWIN

#endif
