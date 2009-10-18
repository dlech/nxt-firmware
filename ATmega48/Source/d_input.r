//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 2-09-05 14:37                                     $
//
// Filename        $Workfile:: d_input.r                                     $
//
// Version         $Revision:: 13                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_input. $
//
// Platform        C
//

#ifdef    ATMEGAX8

//        ADC input used for sensors :

__flash   UBYTE AdcInputNo[NOS_OF_AVR_INPUTS] =
{
  7,0,1,6
};

#define   ONInputPower0                 {\
                                          PORTD  |=  0x02;\
                                          DDRD   |=  0x02;\
                                        }

#define   OFFInputPower0                {\
                                          PORTD  &= ~0x02;\
                                          DDRD   |=  0x02;\
                                        }

#define   ONInputPower1                 {\
                                          PORTD  |=  0x01;\
                                          DDRD   |=  0x01;\
                                        }

#define   OFFInputPower1                {\
                                          PORTD  &= ~0x01;\
                                          DDRD   |=  0x01;\
                                        }

#define   ONInputPower2                 {\
                                          PORTB  |=  0x10;\
                                          DDRB   |=  0x10;\
                                        }

#define   OFFInputPower2                {\
                                          PORTB  &= ~0x10;\
                                          DDRB   |=  0x10;\
                                        }

#define   ONInputPower3                 {\
                                          PORTB  |=  0x20;\
                                          DDRB   |=  0x20;\
                                        }

#define   OFFInputPower3                {\
                                          PORTB  &= ~0x20;\
                                          DDRB   |=  0x20;\
                                        }

void      OnInputPower(UBYTE No)
{
  switch (No)
  {
    case 0 :
    {
      ONInputPower0;
    }
    break;
    
    case 1 :
    {
      ONInputPower1;
    }
    break;
    
    case 2 :
    {
      ONInputPower2;
    }
    break;
    
    case 3 :
    {
      ONInputPower3;
    }
    break;
    
  }
}

void      OffInputPower(UBYTE No)
{
  switch (No)
  {
    case 0 :
    {
      OFFInputPower0;
    }
    break;
    
    case 1 :
    {
      OFFInputPower1;
    }
    break;
    
    case 2 :
    {
      OFFInputPower2;
    }
    break;
    
    case 3 :
    {
      OFFInputPower3;
    }
    break;
    
  }
}


#define   STARTInput                    {\
                                          ADCSRA  &= ~0x07;\
                                          ADCSRA  |=  0x05;\
                                          ADCSRA  |=  0x40;\
                                        }

#define   SELECTInput(No)               {\
                                          UBYTE Mask;\
                                          Mask    = 1 << AdcInputNo[No];\
                                          PORTC   &= ~Mask;\
                                          DDRC    &= ~Mask;\
                                          DIDR0   |=  Mask;\
                                          ADMUX    =  0x40 + (AdcInputNo[No]);\
                                          ADCSRA  &=  ~0x07;\
                                          ADCSRA  |=   0x04;\
                                          ADCSRA  |=   0x40;\
                                        }

#define   BUSYInput                     ((ADCSRA & 0x40))

#define   READInput                     ADC

#define   EXITInput(No)                 {\
                                          UBYTE Mask;\
                                          Mask    = 1 << AdcInputNo[No];\
                                          PORTC   &= ~Mask;\
                                          DDRC    |=  Mask;\
                                        }

#define   INPUTInit                     {\
                                          UBYTE AdcTmp;\
                                          for (AdcTmp = 0;AdcTmp < NOS_OF_AVR_INPUTS;AdcTmp++)\
                                          {\
                                            OffInputPower(AdcTmp);\
                                          }\
                                          ADCSRA     =  0x94;\
                                          ADCSRB     =  0x00;\
                                        }


#define   INPUTSelect(Inp)              {\
                                          SELECTInput(Inp);\
                                          if ((IoToAvr.InputPower & (0x10 << Inp)))\
                                          {\
                                            OnInputPower(Inp);\
                                          }\
                                          else\
                                          {\
                                            OffInputPower(Inp);\
                                          }\
                                        }


#define   INPUTConvert(Inp)             {\
                                          STARTInput;\
                                          while (BUSYInput);\
                                          IoFromAvr.AdValue[Inp] = ADC;\
                                        }


#define   INPUTDeselect(Inp)            {\
                                          if ((IoToAvr.InputPower & (0x01 << Inp)))\
                                          {\
                                            OnInputPower(Inp);\
                                          }\
                                        }


#define   INPUTExit                     {\
                                          UBYTE AdcTmp;\
                                          for (AdcTmp = 0;AdcTmp < NOS_OF_AVR_INPUTS;AdcTmp++)\
                                          {\
                                            OffInputPower(AdcTmp);\
                                            EXITInput(AdcTmp);\
                                          }\
                                          ADCSRA   =  0x10;\
                                        }

#endif
