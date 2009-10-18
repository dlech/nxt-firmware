//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 2-09-05 14:37                                     $
//
// Filename        $Workfile:: d_output.r                                    $
//
// Version         $Revision:: 17                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_output $
//
// Platform        C
//

#ifdef    ATMEGAX8

//        Schematics      Function      PORT
//        ----------      --------      ----
//        MAIN0                         PB0 (PD6)
//        MAPWM           OC1B          PB2 (PD5)
//        MAIN1                         PB3 (PD7)

#define   OUTPUTAFloat                  PORTB  &= ~0x0B;\
                                        DDRB   |=  0x0B;\
                                        TCCR1A &= ~0x30
                                        
#define   OUTPUTABrake                  PORTB  |=  0x0B;\
                                        DDRB   |=  0x0B;\
                                        TCCR1A &= ~0x30
                                        
#define   OUTPUTAInit                   OUTPUTABrake;\
                                        TCCR1A  =  0x01;\
                                        TCCR1B  =  0x09;\
                                        TCNT1   =  0;\
                                        OCR1B   =  0;\
                                        TIMSK1  =  0x00

#define   OUTPUTAFwdFloat(D)            OUTPUTAFloat;\
                                        DDRB   &= ~0x01;\
                                        TCCR1A |=  0x20;\
                                        OCR1B   =  D 

#define   OUTPUTAFwdBrake(D)            OUTPUTABrake;\
                                        PORTB  &= ~0x08;\
                                        DDRB   &= ~0x08;\
                                        TCCR1A |=  0x30;\
                                        OCR1B   =  D

#define   OUTPUTABwdFloat(D)            OUTPUTAFloat;\
                                        DDRB   &= ~0x08;\
                                        TCCR1A |=  0x20;\
                                        OCR1B   =  D 

#define   OUTPUTABwdBrake(D)            OUTPUTABrake;\
                                        PORTB  &= ~0x01;\
                                        DDRB   &= ~0x01;\
                                        TCCR1A |=  0x30;\
                                        OCR1B   =  D

#define   OUTPUTAExit                   OUTPUTAFloat





//        Schematics      Function      PORT
//        ----------      --------      ----
//        MBIN0                         PD6 (PB0)
//        MBPWM           OC1A          PB1 (PB1)
//        MBIN1                         PD7 (PB3)

#define   OUTPUTBFloat                  PORTD  &= ~0xE0;\
                                        DDRD   |=  0xE0;\
                                        TCCR1A &= ~0xC0
                                        
#define   OUTPUTBBrake                  PORTD  |=  0xE0;\
                                        DDRD   |=  0xE0;\
                                        TCCR1A &= ~0xC0
                                        
#define   OUTPUTBInit                   OUTPUTBBrake;\
                                        TCCR1A  =  0x01;\
                                        TCCR1B  =  0x09;\
                                        TCNT1   =  0;\
                                        OCR1A   =  0;\
                                        TIMSK1  =  0x00

#define   OUTPUTBFwdFloat(D)            OUTPUTBFloat;\
                                        DDRD   &= ~0x40;\
                                        TCCR1A |=  0x80;\
                                        OCR1A   =  D 

#define   OUTPUTBFwdBrake(D)            OUTPUTBBrake;\
                                        PORTD  &= ~0x80;\
                                        DDRD   &= ~0x80;\
                                        TCCR1A |=  0xC0;\
                                        OCR1A   =  D

#define   OUTPUTBBwdFloat(D)            OUTPUTBFloat;\
                                        DDRD   &= ~0x80;\
                                        TCCR1A |=  0x80;\
                                        OCR1A   =  D 

#define   OUTPUTBBwdBrake(D)            OUTPUTBBrake;\
                                        PORTD  &= ~0x40;\
                                        DDRD   &= ~0x40;\
                                        TCCR1A |=  0xC0;\
                                        OCR1A   =  D

#define   OUTPUTBExit                   OUTPUTBFloat



//        Schematics      Function      PORT
//        ----------      --------      ----
//        MCIN0                         PB7 (PB7)
//        MCPWM           OC0B          PD5 (PB2)
//        MCIN1                         PB6 (PB6)

#define   OUTPUTCFloat                  PORTB  &= ~0xC4;\
                                        DDRB   |=  0xC4;\
                                        TCCR0A &= ~0x30
                                        
#define   OUTPUTCBrake                  PORTB  |=  0xC4;\
                                        DDRB   |=  0xC4;\
                                        TCCR0A &= ~0x30
                                        
#define   OUTPUTCInit                   OUTPUTCBrake;\
                                        TCCR0A  =  0x03;\
                                        TCCR0B  =  0x01;\
                                        TCNT0   =  0;\
                                        OCR0B   =  0;\
                                        TIMSK0  =  0x00

#define   OUTPUTCFwdFloat(D)            OUTPUTCFloat;\
                                        DDRB   &= ~0x80;\
                                        TCCR0A |=  0x20;\
                                        OCR0B   =  D 

#define   OUTPUTCFwdBrake(D)            OUTPUTCBrake;\
                                        PORTB  &= ~0x40;\
                                        DDRB   &= ~0x40;\
                                        TCCR0A |=  0x30;\
                                        OCR0B   =  D

#define   OUTPUTCBwdFloat(D)            OUTPUTCFloat;\
                                        DDRB   &= ~0x40;\
                                        TCCR0A |=  0x20;\
                                        OCR0B   =  D 

#define   OUTPUTCBwdBrake(D)            OUTPUTCBrake;\
                                        PORTB  &= ~0x80;\
                                        DDRB   &= ~0x80;\
                                        TCCR0A |=  0x30;\
                                        OCR0B   =  D

#define   OUTPUTCExit                   OUTPUTCFloat



UBYTE     TopValue = 255;
UBYTE     BrakeMask;

void      WriteFreq(UBYTE Freq)
{
  if (Freq  >= 4)
  {
    TopValue = (UBYTE)(((ULONG)OSC / 8000L) / (ULONG)Freq);
    TCCR0B  &= ~0x0F;
    TCCR0B  |=  0x0A;

    TCCR1A  &= ~0x03;
    TCCR1A  |=  0x02;
    TCCR1B  &= ~0x1F;
    TCCR1B  |=  0x1A;
  }
  else
  {
    TopValue = (UBYTE)(((ULONG)OSC / 64000L) / (ULONG)Freq);    
    TCCR0B  &= ~0x0F;
    TCCR0B  |=  0x0B;

    TCCR1A  &= ~0x03;
    TCCR1A  |=  0x02;
    TCCR1B  &= ~0x1F;
    TCCR1B  |=  0x1B;
  }
  OCR0B    =  0;
  OCR0A    =  TopValue;
  OCR1A    =  0;
  OCR1B    =  0;
  ICR1L    =  TopValue;
  ICR1H    =  0;
}


void      OutputBrake(UBYTE No)
{
  switch (No)
  {
    case 0 :
    {
      OUTPUTABrake;
    }
    break;
    
    case 1 :
    {
      OUTPUTBBrake;
    }
    break;
    
    case 2 :
    {
      OUTPUTCBrake;
    }
    break;
    
  }
}

void      OutputFloat(UBYTE No)
{
  switch (No)
  {
    case 0 :
    {
      OUTPUTAFloat;
    }
    break;
    
    case 1 :
    {
      OUTPUTBFloat;
    }
    break;
    
    case 2 :
    {
      OUTPUTCFloat;
    }
    break;
    
  }
}

void      OutputFwdBrake(UBYTE No,UBYTE Pwm)
{
  switch (No)
  {
    case 0 :
    {
      OUTPUTAFwdBrake(Pwm);
    }
    break;
    
    case 1 :
    {
      OUTPUTBFwdBrake(Pwm);
    }
    break;
    
    case 2 :
    {
      OUTPUTCFwdBrake(Pwm);
    }
    break;
    
  }
}

void      OutputBwdBrake(UBYTE No,UBYTE Pwm)
{
  switch (No)
  {
    case 0 :
    {
      OUTPUTABwdBrake(Pwm);
    }
    break;
    
    case 1 :
    {
      OUTPUTBBwdBrake(Pwm);
    }
    break;
    
    case 2 :
    {
      OUTPUTCBwdBrake(Pwm);
    }
    break;
    
  }
}

void      OutputFwdFloat(UBYTE No,UBYTE Pwm)
{
  switch (No)
  {
    case 0 :
    {
      OUTPUTAFwdFloat(Pwm);
    }
    break;
    
    case 1 :
    {
      OUTPUTBFwdFloat(Pwm);
    }
    break;
    
    case 2 :
    {
      OUTPUTCFwdFloat(Pwm);
    }
    break;
    
  }
}

void      OutputBwdFloat(UBYTE No,UBYTE Pwm)
{
  switch (No)
  {
    case 0 :
    {
      OUTPUTABwdFloat(Pwm);
    }
    break;
    
    case 1 :
    {
      OUTPUTBBwdFloat(Pwm);
    }
    break;
    
    case 2 :
    {
      OUTPUTCBwdFloat(Pwm);
    }
    break;
    
  }
}

void      OutputWrite(UBYTE No,SBYTE Duty)
{
  UBYTE   Pwm;
  
  if (No < NOS_OF_AVR_OUTPUTS)
  {  
    if (Duty < 0)
    {
      Pwm = (UBYTE)(0 - Duty);
    }
    else
    {
      Pwm = (UBYTE)Duty;
    }
    Pwm = (UBYTE)(((UWORD)Pwm * (UWORD)TopValue) / 100);
    

    if ((BrakeMask & (0x01 << No)))
    {
      if (Duty)
      {
        if (Duty > 0)
        {
          OutputFwdBrake(No,Pwm);
        }
        else
        {
          OutputBwdBrake(No,Pwm);
        }
      }
      else
      {
        OutputBrake(No);
      }
    }
    else
    {
      if (Duty)
      {
        if (Duty > 0)
        {
          OutputFwdFloat(No,Pwm);
        }
        else
        {
          OutputBwdFloat(No,Pwm);
        }
      }
      else
      {
        OutputFloat(No);
      }
    }
  }
}



#define   OUTPUTInit                    OUTPUTAInit;\
                                        OUTPUTBInit;\
                                        OUTPUTCInit;\
                                        BrakeMask = 0xFF

#define   OUTPUTWriteBrakeMask(M)       BrakeMask = M

#define   OUTPUTWrite(No,Duty)          OutputWrite(No,Duty)

#define   OUTPUTFreq(Freq)              WriteFreq(Freq)

#define   OUTPUTUpdate

#define   OUTPUTExit                    OUTPUTAExit;\
                                        OUTPUTBExit;\
                                        OUTPUTCExit

#endif
