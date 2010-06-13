//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_button.r                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_butt $
//
// Platform        C
//

#ifdef    SAM7S256

static    UBYTE PrellCnt[NOS_OF_AVR_BTNS];
static    UWORD OldVal;
static    UBYTE OldState;
static    UBYTE RisingTime;

#define   PRELL_TIME                    (60/TimeTick)
#define   RISING_THRESHOLD              (10/TimeTick)

#define   BUTTONInit                    {\
                                          UBYTE Tmp;\
                                          for (Tmp = 0; Tmp < NOS_OF_AVR_BTNS; Tmp++)\
                                          {\
                                            PrellCnt[Tmp] = 0;\
                                          }\
                                          IoFromAvr.Buttons = 0;\
                                          OldVal            = 0;\
                                          OldState          = 0;\
                                          RisingTime        = 0;\
                                        }

#if       defined (PROTOTYPE_PCB_3) || (PROTOTYPE_PCB_4)

/* Buttons read here are free of prell or jitter          */
/* And because it's an AD value returned from the AVR     */
/* then a peak detector is needed                         */
#define   BUTTONRead(pB)                {\
                                          UBYTE Tmp, BtnPtr;\
                                          UWORD TmpBtn;\
                                          *pB    = OldState;\
                                          BtnPtr = 0x01;\
                                          if (OldVal < IoFromAvr.Buttons)\
                                          {\
                                            OldVal = IoFromAvr.Buttons;\
                                            RisingTime  = 0;\
                                          }\
                                          else\
                                          {\
                                            if (OldVal > (IoFromAvr.Buttons + 20))\
                                            {\
                                              OldVal = IoFromAvr.Buttons;\
                                              RisingTime  = 0;\
                                            }\
                                            else\
                                            {\
                                              if (RisingTime > RISING_THRESHOLD)\
                                              {\
                                                TmpBtn = IoFromAvr.Buttons;\
                                                if (0x40 > TmpBtn)\
                                                {\
                                                  TmpBtn = 0x00;\
                                                }\
                                                else if (0x100 > TmpBtn)\
                                                {\
                                                  TmpBtn = 0x04;\
                                                }\
                                                else if (0x1FF > TmpBtn)\
                                                {\
                                                  TmpBtn = 0x02;\
                                                }\
                                                else if (0x5FF > TmpBtn)\
                                                {\
                                                  TmpBtn = 0x01;\
                                                }\
                                                else\
                                                {\
                                                  TmpBtn = 0x08;\
                                                }\
                                                for (Tmp = 0; Tmp < NOS_OF_AVR_BTNS; Tmp++)\
                                                {\
                                                  if ((TmpBtn) & BtnPtr)\
                                                  {\
                                                    *pB |= BtnPtr;\
                                                    PrellCnt[Tmp]  = PRELL_TIME;\
                                                  }\
                                                  else\
                                                  {\
                                                    /* btn not pressed */\
                                                    if (0 != PrellCnt[Tmp])\
                                                    {\
                                                      PrellCnt[Tmp]--;\
                                                    }\
                                                    else\
                                                    {\
                                                      *pB &= ~BtnPtr;\
                                                    }\
                                                  }\
                                                  BtnPtr <<= 1;\
                                                }\
                                                OldState = *pB;\
                                              }\
                                              else\
                                              {\
                                                RisingTime++;\
                                              }\
                                            }\
                                          }\
                                        }

#else

// Buttons read here are free of prell or jitter
#define   BUTTONRead(pB)                {\
                                          UBYTE Tmp, BtnPtr;\
                                          UWORD TmpBtn;\
                                          *pB    = OldState;\
                                          BtnPtr = 0x01;\
                                          if ((OldVal) < IoFromAvr.Buttons)\
                                          {\
                                            OldVal = IoFromAvr.Buttons;\
                                          }\
                                          else\
                                          {\
                                            if ((OldVal) > IoFromAvr.Buttons)\
                                            {\
                                              OldVal = IoFromAvr.Buttons;\
                                            }\
                                            else\
                                            {\
                                              TmpBtn = IoFromAvr.Buttons;\
                                              if (100 > TmpBtn)\
                                              {\
                                                TmpBtn = 0x00;\
                                              }\
                                              else if (170 > TmpBtn)\
                                              {\
                                                TmpBtn = 0x01;\
                                              }\
                                              else if (255 > TmpBtn)\
                                              {\
                                                TmpBtn = 0x02;\
                                              }\
                                              else if (1000 > TmpBtn)\
                                              {\
                                                TmpBtn = 0x04;\
                                              }\
                                              else if (1024 > TmpBtn)\
                                              {\
                                                TmpBtn = 0x08;\
                                              }\
                                              for (Tmp = 0; Tmp < NOS_OF_AVR_BTNS; Tmp++)\
                                              {\
                                                if ((TmpBtn) & BtnPtr)\
                                                {\
                                                  *pB |= BtnPtr;\
                                                  PrellCnt[Tmp]  = PRELL_TIME;\
                                                }\
                                                else\
                                                {\
                                                  /* btn not pressed */\
                                                  if (0 != PrellCnt[Tmp])\
                                                  {\
                                                    PrellCnt[Tmp]--;\
                                                  }\
                                                  else\
                                                  {\
                                                    *pB &= ~BtnPtr;\
                                                  }\
                                                }\
                                                BtnPtr <<= 1;\
                                              }\
                                              OldState = *pB;\
                                            }\
                                          }\
                                        }
#endif

#define   BUTTONExit

#endif

#ifdef    PCWIN

#endif
