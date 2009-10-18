//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 2-09-05 14:37                                     $
//
// Filename        $Workfile:: d_button.r                                    $
//
// Version         $Revision:: 10                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_button $
//
// Platform        C
//

#ifdef    ATMEGAX8

#pragma   language=extended
#pragma   vector = INT1_vect
__interrupt void OnInterrupt(void)
{
  EIMSK &= ~0x02;
  HARDWAREReset;
}

#define   BUTTONInit                    {\
                                          EIMSK      &= ~0x02;\
                                          PORTD      |=  0x08;\
                                          DDRD       &= ~0x08;\
                                          PORTC      &= ~0x08;\
                                          DDRC       &= ~0x08;\
                                          DIDR0      |=  0x08;\
                                        }


UWORD     ButtonRead(void)
{
  UWORD   Result;
  
  ADMUX    =  0x43;
  ADCSRA  &= ~0x07;
  ADCSRA  |=  0x05;
  ADCSRA  |=  0x40;
  while ((ADCSRA & 0x40));
  ADCSRA  |=  0x40;
  while ((ADCSRA & 0x40));
  Result   = ADC;
  if (!(PIND & 0x08))
  {
    Result += 0x7FF;
  }
  return (Result);
}


#define   BUTTONRead                    ButtonRead()

#define   BUTTONExit                    {\
                                          PORTD     |=  0x08;\
                                          DDRD      &= ~0x08;\
                                          EICRA     &= ~0x0C;\
                                          EIFR      |=  0x02;\
                                          EIMSK     |=  0x02;\
                                        }
#endif
