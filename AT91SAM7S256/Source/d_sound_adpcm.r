//Playback of compressed sound files. This additional feature is being brought to you under the following license.
//Please adhere to its terms.
//The original code includes minor changes to function correctly within the LEGO MINDSTORMS NXT embedded system,
//but the main architecture are implemented as within the original code.

//***********************************************************
//Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
//Netherlands.
//
//                        All Rights Reserved
//
//Permission to use, copy, modify, and distribute this software and its
//documentation for any purpose and without fee is hereby granted,
//provided that the above copyright notice appear in all copies and that
//both that copyright notice and this permission notice appear in
//supporting documentation, and that the names of Stichting Mathematisch
//Centrum or CWI not be used in advertising or publicity pertaining to
//distribution of the software without specific, written prior permission.
//
//STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
//THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
//FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
//FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
//OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
//******************************************************************/

//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkflebun                                        $
//
// Revision date   $Date:: 5-02-07 13:36                                     $
//
// Filename        $Workfile:: d_sound_adpcm.r                               $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_sound_adpcm.r  $
//
// Platform        C
//

#ifdef    SAM7S256

static SWORD IndexTable[16] =
{
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static SWORD StepsizeTable[89] =
{
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

__ramfunc void SoundADPCMDecoder(UBYTE Indata, UBYTE *Outdata, SWORD *pStateValprev, SWORD *pStateIndex)
{
    SWORD Step;                // Stepsize
    SWORD Valprev;             // Virtual previous output value
    SWORD Vpdiff;              // Current change to valprev
    SWORD Index;               // Current step change index
    UBYTE *pOut;               // Output buffer pointer
    UBYTE Sign;                // Current adpcm sign bit
    UBYTE Delta;               // Current adpcm output value
    UBYTE Bufferstep;          // Toggle between High og Low nibble
    UBYTE Len;                 // Nibble Counter

    pOut = Outdata;

    Valprev = *pStateValprev;
    Index = *pStateIndex;
    Step = StepsizeTable[Index];

    Bufferstep = 0;
    Len = 2;

    for (; Len > 0 ; Len--)                               //Step 1 - get the delta value and compute next index
    {
       if(Bufferstep)
       {
         Delta = Indata & 0x0F;
       }
       else
       {
         Delta = (Indata >> 4) & 0x0F;
       }
       Bufferstep = !Bufferstep;

       Index += IndexTable[Delta];                        //Step 2 - Find new index value (for later)
       if (Index < 0)
       {
         Index = 0;
       }
       else
       {
         if (Index > 88)
         {
           Index = 88;
         }
       }

       Sign = Delta & 8;                                  //Step 3 - Separate sign and magnitude
       Delta = Delta & 7;

       Vpdiff = Step >> 3;                                //Step 4 - Compute difference and new predicted value

       if (Delta & 4)
       {
         Vpdiff += Step;
       }
       if (Delta & 2)
       {
         Vpdiff += Step>>1;
       }
       if (Delta & 1)
       {
         Vpdiff += Step>>2;
       }

       if (Sign)
         Valprev -= Vpdiff;
       else
         Valprev += Vpdiff;

       if (Valprev > 255)                                 //Step 5 - clamp output value
       {
         Valprev = 255;
       }
       else
       {
         if (Valprev < 0)
         {
           Valprev = 0;
         }
       }
       Step = StepsizeTable[Index];                        //Step 6 - Update step value
       *pOut++ = (UBYTE)Valprev;                           //Step 7 - Output value
    }
    *pStateValprev = Valprev;                                //State.Valprev = Valprev;
    *pStateIndex = Index;                                    //State.Index = Index;
}

#endif
