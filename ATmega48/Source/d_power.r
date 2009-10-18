//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 2-09-05 14:37                                     $
//
// Filename        $Workfile:: d_power.r                                     $
//
// Version         $Revision:: 8                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_power. $
//
// Platform        C
//

#ifdef    ATMEGAX8

#define   POWERInit                     {\
                                          PORTC      &= ~0x04;\
                                          DDRC       &= ~0x04;\
                                          DIDR0      |=  0x04;\
                                        }

#define   POWEROff                      {\
                                          PORTD &= ~0x10;\
                                          DDRD  |=  0x10;\
                                        }
                                        
#define   POWEROn                       {\
                                          PORTD |=  0x10;\
                                          DDRD  |=  0x10;\
                                        }

#define   POWERSelect                   {\
                                          PORTD |=  0x04;\
                                          DDRD  |=  0x04;\
                                        }

#define   POWERConvert(V)               {\
                                          ADMUX    =  0x42;\
                                          ADCSRA  &= ~0x07;\
                                          ADCSRA  |=  0x05;\
                                          ADCSRA  |=  0x40;\
                                          while ((ADCSRA & 0x40));\
                                          ADCSRA  |=  0x40;\
                                          while ((ADCSRA & 0x40));\
                                          V        = ADC;\
                                          V       &= 0x7FFF;\
                                          IoFromAvr.Battery &= 0x8000;\
                                          IoFromAvr.Battery |= V;\
                                        }

#define   POWERDeselect                 {\
                                          PORTD &= ~0x04;\
                                          DDRD  |=  0x04;\
                                        }

#define   POWERHigh                     {\
                                          PORTC |=  0x04;\
                                          DDRC  |=  0x04;\
                                        }

#define   POWERFloat                    {\
                                          PORTC &= ~0x04;\
                                          DDRC  &= ~0x04;\
                                        }



#define   POWERExit                     {\
                                          POWEROff;\
                                          POWERDeselect;\
                                          POWERConvert(ADC);\
                                        }

#endif
