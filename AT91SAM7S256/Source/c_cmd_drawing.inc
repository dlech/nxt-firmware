//
// Date init       14.12.2004
//
// Revision date   $Date:: 8/26/05 4:17p                                     $
//
// Filename        $Workfile:: c_cmd.c                                       $
//
// Version         $Revision:: 35                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/c_cmd_drawing.c $
//
// Platform        C
//

//absolute value of a
#define ABS(a)    (((a)<0) ? -(a) : (a))

//take binary sign of a, either -1, or 1 if >= 0
#define SGN(a)    (((a)<0) ? -1 : 1)

#define DISP_BUFFER_P ((UBYTE*)&(pMapDisplay->Normal))

//------------------------------------------------------------------
// cCmdClearScreenIfNeeded - Clear entire sceen buffer if explicitly requested or implicitly required.
void cCmdClearScreenIfNeeded(ULONG DrawOptions);

//------------------------------------------------------------------
// cCmdRestorDefaultScreen - Restore screen to default 'Running' screen
void cCmdRestoreDefaultScreen(void);

//------------------------------------------------------------------
// cCmdDrawString - Draw string to display buffer
void cCmdDrawString(UBYTE *pString, ULONG X, ULONG Y);

// OP codes supported by RIC files
enum {
  IMG_DESCRIPTION_ID = 0, // Ignored at this time
  IMG_SPRITE_ID = 1,
  IMG_VARMAP_ID = 2,
  IMG_COPYBITS_ID = 3,
  IMG_PIXEL_ID = 4,
  IMG_LINE_ID = 5,
  IMG_RECTANGLE_ID = 6,
  IMG_CIRCLE_ID = 7,
  IMG_NUMBOX_ID = 8
};

#define IMG_SYMB_USEARGS(_v) (_v & (SWORD)0xF000)
#define IMG_SYMB_MAP(_v)  ((_v & 0x0F00) >> 8)
#define IMG_SYMB_ARG(_v)  (_v & 0x000F)

// DrawingOptions
#define DRAW_OPT_CLEAR_WHOLE_SCREEN     (0x0001)
#define DRAW_OPT_CLEAR_EXCEPT_STATUS_SCREEN     (0x0002)
#define DRAW_OPT_CLEAR_MODE(_v)     ((_v) & 0x0003)


// Clear Before Drawing Modes for Draw functions
enum {
  DO_NOT_CLEAR = 0,
  CLEAR_B4_DRAW = 1
};

// Screen Modes for SetScreenMode function
enum {
  RESTORE_NXT_SCREEN = 0
};

#define IMG_COMMON_FIELDS  UWORD OpSize; UWORD OpCode;

#define TRANSLATE_Y(_y) ((DISPLAY_HEIGHT-1) - (_y))

typedef struct
{
  SWORD X, Y;
} IMG_PT;

typedef struct
{
  IMG_PT Pt;
  SWORD Width, Height;
} IMG_RECT;

typedef struct
{
  IMG_COMMON_FIELDS
} IMG_OP_CORE;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD Options;
  UWORD Width;
  UWORD Height;
} IMG_OP_DESCRIPTION;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD   DataAddr;    //Address sprite handle will be stored in.
  UWORD   Rows;        //Second deminsion of the array below.
  UWORD   RowBytes;    //The actual size of the following array. Must be even.
  UBYTE   Bytes[2];    //Minimum of two for alignment purposes
} IMG_OP_SPRITE;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD   DataAddr;    //Address sprite handle will be stored in.
  UWORD   MapCount;    //The actual size of the following array. Must be even.
  struct
  {    //Minimum of two for alignment purposes
    UWORD   Domain;
    UWORD   Range;
  }       MapElt[1];
} IMG_OP_VARMAP;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD     CopyOptions;  // Copy, CopyNot, Or, BitClear;
  UWORD     DataAddr;   // Address of an already defined sprite
  IMG_RECT  Src;    // Source rectangle
  IMG_PT Dst;    // Destination left top
} IMG_OP_COPYBITS;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD   CopyOptions;
  IMG_PT  Pt;
  UWORD   Value; // typically mapped to an argument
} IMG_OP_PIXEL;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD   CopyOptions;
  IMG_PT  Pt1;
  IMG_PT  Pt2;
} IMG_OP_LINE;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD   CopyOptions;
  IMG_PT  Pt;
  SWORD  Width, Height;
} IMG_OP_RECT;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD   CopyOptions;
  IMG_PT  Pt;
  UWORD   Radius;
} IMG_OP_CIRCLE;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD   CopyOptions;
  IMG_PT  Pt;
  UWORD   Value; // typically mapped to an argument
} IMG_OP_NUMBOX;

typedef union
{ IMG_OP_CORE      Core;
  IMG_OP_DESCRIPTION Desc;
  IMG_OP_SPRITE    Sprite;
  IMG_OP_VARMAP    VarMap;
  IMG_OP_COPYBITS  CopyBits;
  IMG_OP_PIXEL     Pixel;
  IMG_OP_LINE      Line;
  IMG_OP_RECT      Rect;
  IMG_OP_CIRCLE    Circle;
  IMG_OP_NUMBOX    NumBox;
} IMG_OP_UNION;

// Variables for DrawImage
#define IMG_MAX_DATA 11
IMG_OP_UNION  * gpImgData[IMG_MAX_DATA];
SLONG         * gpPassedImgVars;
SWORD   gPassedVarsCount;

// Private Prototypes
void cCmdDrawLine(SLONG x1, SLONG y1, SLONG x2, SLONG y2);
void cCmdDrawRect(SLONG left, SLONG bottom, SLONG width, SLONG hieght);
void cCmdCopyBitMapBits(SLONG dst_x, SLONG dst_y,
                        SLONG src_x, SLONG src_y, SLONG src_width, SLONG src_height,
                        IMG_OP_SPRITE * pSprite);
SLONG cCmdResolveValue(SWORD Value);
void cCmdSetPixel(SLONG X, SLONG Y, ULONG Val);

//-----------------------------------------------------------------
//cCmdWrapDrawText
//ArgV[0]: (Function return) Status byte, SBYTE
//ArgV[1]: Location (IMG_PT *)
//ArgV[2]: Text (CStr)
//ArgV[3]: Options (ULONG)
//
NXT_STATUS cCmdWrapDrawText(UBYTE * ArgV[])
{
  IMG_PT * pPt = (IMG_PT*) ArgV[1];

  ArgV[2] = (UBYTE*)cCmdDVPtr(*(DV_INDEX *)(ArgV[2]));   //Resolve array argument

  cCmdClearScreenIfNeeded(*(ULONG*)ArgV[3]);

  // Display the String
  cCmdDrawString(ArgV[2], (UBYTE)pPt->X, (UBYTE)(pPt->Y));
  pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);

  // Set return value
  *((SBYTE*)(ArgV[0])) = NO_ERR;

  return NO_ERR;
}

//-----------------------------------------------------------------
//cCmdWrapDrawPoint
//ArgV[0]: (Function return) Status byte, SBYTE
//ArgV[1]: Location (IMG_PT *)
//ArgV[2]: Options (ULONG)
NXT_STATUS cCmdWrapDrawPoint(UBYTE * ArgV[])
{
  IMG_PT * pPt = (IMG_PT*) ArgV[1];

  cCmdClearScreenIfNeeded(*(ULONG*)ArgV[2]);

  // Display the String
  cCmdSetPixel(pPt->X, pPt->Y, TRUE);

  pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);

  // Set return value
  *((SBYTE*)(ArgV[0])) = NO_ERR;

  return NO_ERR;
}

//-----------------------------------------------------------------
//cCmdWrapDrawLine
//ArgV[0]: (Function return) Status byte, SBYTE
//ArgV[1]: Start Location (IMG_PT *)
//ArgV[2]: End Location (IMG_PT *)
//ArgV[3]: Options (ULONG)
NXT_STATUS cCmdWrapDrawLine(UBYTE * ArgV[])
{
  IMG_PT * pPt1 = (IMG_PT*) ArgV[1];
  IMG_PT * pPt2 = (IMG_PT*) ArgV[2];

  cCmdClearScreenIfNeeded(*(ULONG*)ArgV[3]);

  cCmdDrawLine(pPt1->X, pPt1->Y, pPt2->X, pPt2->Y);

  pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);

  // Set return value
  *((SBYTE*)(ArgV[0])) = NO_ERR;

  return NO_ERR;
}

//-----------------------------------------------------------------
//cCmdWrapDrawCircle
//ArgV[0]: (Function return) Status byte, SBYTE
//ArgV[1]: Start Location (IMG_PT *)
//ArgV[2]: Radius (U8)
//ArgV[3]: Options (ULONG)
NXT_STATUS cCmdWrapDrawCircle(UBYTE * ArgV[])
{
  SLONG x, x1, y1, y, dp, delta;
  IMG_PT * pPt = (IMG_PT*) ArgV[1];
  SLONG radius = *(UBYTE*)ArgV[2];

  cCmdClearScreenIfNeeded(*(ULONG*)ArgV[3]);

  x1 = pPt->X;
  y1 = pPt->Y;
  x = 0;
  y = radius;
  dp=2*(1-radius);
  while(y >= 0)
  {
    cCmdSetPixel((x+x1), (y+y1),  TRUE);
    cCmdSetPixel((-x+x1),(-y+y1), TRUE);
    cCmdSetPixel((x+x1), (-y+y1), TRUE);
    cCmdSetPixel((-x+x1),(y+y1),  TRUE);
    if(dp<0)
    {
      delta = 2*dp + 2*y - 1;
      if (delta > 0)
      {
        x++;
        y--;
        dp += 2*x - 2*y + 2;
      }
      else
      {
        x++;
        dp += 2*x + 1;
      }
    }
    else if (dp > 0)
    {
      delta = 2*dp - 2*x - 1;
      if (delta > 0)
      {
        y--;
        dp += 1 - 2*y;
      }
      else
      {
        x++;
        y--;
        dp += 2*x - 2*y + 2;
      }
    }
    else
    {
      x++;
      y--;
      dp += 2*x - 2*y +2;
    }
  }

  pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);

  // Set return value
  *((SBYTE*)(ArgV[0])) = NO_ERR;

  return NO_ERR;
}

//-----------------------------------------------------------------
//cCmdWrapDrawRect
//ArgV[0]: (Function return) Status byte, SBYTE
//ArgV[1]: TopLeft (IMG_PT *)
//ArgV[2]: BottomRight (IMG_PT *)
//ArgV[3]: Options (ULONG)
NXT_STATUS cCmdWrapDrawRect(UBYTE * ArgV[])
{
  IMG_PT * pPt1 = (IMG_PT*) ArgV[1];
  IMG_PT * pPt2 = (IMG_PT*) ArgV[2];  // Second point is actually (width, height)

  cCmdClearScreenIfNeeded(*(ULONG*)ArgV[3]);

  cCmdDrawRect(pPt1->X, pPt1->Y, pPt2->X, pPt2->Y);

  pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);

  // Set return value
  *((SBYTE*)(ArgV[0])) = NO_ERR;

  return NO_ERR;
}

//-----------------------------------------------------------------
IMG_OP_UNION * cCmdGetIMGData(ULONG DataAddr)
{
  if (DataAddr >= IMG_MAX_DATA)
    return NULL;
  else
    return gpImgData[DataAddr];
}

//-----------------------------------------------------------------
void cCmdSetIMGData(ULONG DataAddr, IMG_OP_UNION * pSprite)
{
  if ((DataAddr >= 1) && (DataAddr < IMG_MAX_DATA))
    gpImgData[DataAddr] = pSprite;
}

//-----------------------------------------------------------------
SLONG cCmdResolveValue(SWORD Value)
{
  if (!IMG_SYMB_USEARGS(Value))
  {
    return Value;
  }
  else
  {
    IMG_OP_VARMAP * pVarMap;
    SLONG Arg;

    pVarMap = (IMG_OP_VARMAP *) cCmdGetIMGData((SWORD)IMG_SYMB_MAP(Value));
    Arg = gpPassedImgVars[IMG_SYMB_ARG(Value)];

    if (!pVarMap)
    {
      // No map, this implies a 1:1 mapping.
      return Arg;
    }
    else
    {
      // Scan through the list finding the pair the Arg lies between
      // Then linearly interpolate the mapping.
      SLONG i, DCur, RCur, DSpread, VSpread, RSpread;
      SLONG Count = pVarMap->MapCount;
      SLONG DPrev = pVarMap->MapElt[0].Domain;
      SLONG RPrev = pVarMap->MapElt[0].Range;
      if (Arg <= DPrev)
      {
        // Too small, map it to the first point
        return RPrev;
      }

      for (i = 1; i < Count; i++)
      {
        DCur = pVarMap->MapElt[i].Domain;
        RCur = pVarMap->MapElt[i].Range;
        if (Arg < DCur)
        {
          DSpread = DCur - DPrev;
          VSpread = Arg - DPrev;
          RSpread = RCur - RPrev;
          // Found the point and mapped, it return.
          return  (RPrev+((VSpread*RSpread)/DSpread));
        }
        DPrev = DCur;
        RPrev = RCur;
      }
      // If we get this far then it is too large, map it to the last point.
      return  RCur;
    }
  }
}


//-----------------------------------------------------------------
//cCmdWrapDrawGraphic
//ArgV[0]: (Function return) Status Byte, SBYTE
//ArgV[1]: Left Top (IMG_PT *)
//ArgV[2]: Filename, CStr
//ArgV[3]: Variables, array of I32
//ArgV[4]: Options (ULONG)
NXT_STATUS cCmdWrapDrawPicture(UBYTE * ArgV[])
{
  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  LOADER_STATUS LStatus;
  NXT_STATUS    DStatus = NO_ERR;
  ULONG         DataSize;
  SLONG         OpSize;
  IMG_PT        Pt;     // Where to draw the picture at (up and to the right)
  UBYTE   ImageHandle;
  IMG_OP_UNION  * pImage;

  //Resolve array argument
  ArgV[2] = (UBYTE*)cCmdDVPtr(*(DV_INDEX *)(ArgV[2]));
  ArgV[3] = (UBYTE*)cCmdDVPtr(*(DV_INDEX *)(ArgV[3]));

  cCmdClearScreenIfNeeded(*(ULONG*)ArgV[4]);

  //Open the file in memory map mode. return if failure.
  LStatus = pMapLoader->pFunc(OPENREADLINEAR, ArgV[2], (UBYTE*)(&pImage), &DataSize);
  ImageHandle = LOADER_HANDLE(LStatus);

  //If error opening file, give up and write loader status back to user.
  if (LOADER_ERR(LStatus) != SUCCESS || pImage == NULL)
  {
    *pReturnVal = (SBYTE)(LOADER_ERR_BYTE(LStatus));
    return (NO_ERR);
  }
  //Else, start interpretting the file
  else
  {
    // Read the ArgV params, Clear the data table.
    Pt = *(IMG_PT*)ArgV[1];
    //!!! Unsafe assumption that array is non-empty.  Should check and avoid using pointer if empty.
    gpPassedImgVars = (SLONG*)ArgV[3];
    memset(gpImgData,0,sizeof(gpImgData));

    // Run through the op codes.
    while(!IS_ERR(DStatus))
    {
      // Setup to look at an opcode, make sure it looke reasonable.
      if (DataSize < sizeof(IMG_OP_CORE))
      {
        DStatus = ERR_FILE;
        break; // Too small to look at, somethings wrong.
      }
      OpSize = pImage->Core.OpSize + sizeof(UWORD);
      if (OpSize & 0x01)
      {
        DStatus = ERR_FILE;
        break; // Odd sizes not allowed.
      }

      switch(pImage->Core.OpCode)
      {
        case IMG_SPRITE_ID:
        {
          if (OpSize >= sizeof(IMG_OP_SPRITE))
            cCmdSetIMGData(pImage->Sprite.DataAddr, pImage);
        }
        break;

        case IMG_VARMAP_ID:
        {
          if (OpSize >= sizeof(IMG_OP_VARMAP))
            cCmdSetIMGData(pImage->VarMap.DataAddr, pImage);
        }
        break;

        case IMG_COPYBITS_ID:
        {
          if (OpSize >= sizeof(IMG_OP_COPYBITS))
          {
            IMG_OP_COPYBITS * pCB = &(pImage->CopyBits);
            cCmdCopyBitMapBits(
              (cCmdResolveValue(pCB->Dst.X) + Pt.X),
              (cCmdResolveValue(pCB->Dst.Y) + Pt.Y),
              cCmdResolveValue((pCB->Src.Pt.X)),
              cCmdResolveValue((pCB->Src.Pt.Y)),
              cCmdResolveValue((pCB->Src.Width)),
              cCmdResolveValue((pCB->Src.Height)),
              (IMG_OP_SPRITE*)cCmdGetIMGData(cCmdResolveValue(pCB->DataAddr)));
          }
        }
        break;

        case IMG_LINE_ID:
        {
          if (OpSize >= sizeof(IMG_OP_LINE))
          {
            IMG_OP_LINE * pL = &(pImage->Line);
            cCmdDrawLine(
              (cCmdResolveValue(pL->Pt1.X)+Pt.X),
              (cCmdResolveValue(pL->Pt1.Y)+Pt.Y),
              (cCmdResolveValue(pL->Pt2.X)+Pt.X),
              (cCmdResolveValue(pL->Pt2.Y)+Pt.Y)
            );
          }
        }
        break;

        case IMG_RECTANGLE_ID:
        {
          if (OpSize >= sizeof(IMG_OP_LINE))
          {
            IMG_OP_RECT * pL = &(pImage->Rect);
            cCmdDrawRect(
              (SWORD)(cCmdResolveValue(pL->Pt.X)+Pt.X),
              (SWORD)(cCmdResolveValue(pL->Pt.Y)+Pt.Y),
              (SWORD)(cCmdResolveValue(pL->Width)),
              (SWORD)(cCmdResolveValue(pL->Height))
            );
          }
        }
        break;

        case IMG_PIXEL_ID:
        {
          if (OpSize >= sizeof(IMG_OP_PIXEL))
          {
            cCmdSetPixel(
              (cCmdResolveValue(pImage->Pixel.Pt.X) + Pt.X),
              (cCmdResolveValue(pImage->Pixel.Pt.Y) + Pt.Y),
              TRUE);
          }
        }
        break;

        case IMG_NUMBOX_ID:
        {
          if (OpSize >= sizeof(IMG_OP_NUMBOX))
          {
            UBYTE NumStr[20];
            IMG_OP_NUMBOX * pNB = &(pImage->NumBox);
            sprintf((PSZ)NumStr, "%d", cCmdResolveValue(pNB->Value));
            cCmdDrawString(
              NumStr,
              (UBYTE) (cCmdResolveValue(pNB->Pt.X) + Pt.X),
              (UBYTE) (cCmdResolveValue(pNB->Pt.Y) + Pt.Y));
          }
        }
        break;

        case IMG_DESCRIPTION_ID:
        {
        //No-op
        }
        break;

        default:
        {
          //Unrecognized opcode, pass an error back to the user.
          DStatus = ERR_FILE;
        }
        break;
      }

      DataSize -= OpSize;
      pImage = (IMG_OP_UNION*) ((UBYTE*)pImage + OpSize);
    }

    pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);
  }

  // Set return value, close file and return
  *pReturnVal = DStatus;
  pMapLoader->pFunc(CLOSE, &ImageHandle, NULL, NULL);
  return (NO_ERR);
}

//-----------------------------------------------------------------
// cCmdDrawLine - draw a line. All clipping is done by the set pixel function.
void cCmdDrawLine(
  SLONG x1,
  SLONG y1,
  SLONG x2,
  SLONG y2)
{
  SLONG d,x,y,ax,ay,sx,sy,dx,dy;

  // Initialize variables
  dx = x2-x1;  ax = ABS(dx)<<1;  sx = SGN(dx);
  dy = y2-y1;  ay = ABS(dy)<<1;  sy = SGN(dy);
  x = x1;
  y = y1;
  if (ax>ay)
  {    /* x dominant */
    d = ay-(ax>>1);
    for (;;)
    {
      cCmdSetPixel(x, y, TRUE);
      if (x==x2)
        return;
      if (d>=0)
      {
        y += sy;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  }
  else
  {      /* y dominant */
    d = ax-(ay>>1);
    for (;;)
    {
      cCmdSetPixel(x, y, TRUE);
      if (y==y2)
        return;
      if (d>=0)
      {
        x += sx;
        d -= ay;
      }
      y += sy;
      d += ax;
    }
  }
}


//-----------------------------------------------------------------
// cCmdDrawRect - draw a rectangle. All clipping is done by the set pixel function.
void cCmdDrawRect(
  SLONG left,
  SLONG bottom,
  SLONG width,
  SLONG height)
{
  SLONG right = left + width;
  SLONG top = bottom + height;

  // Draw the four line segments
  cCmdDrawLine(left, top, right, top);
  cCmdDrawLine(right, top, right, bottom);
  cCmdDrawLine(right, bottom, left, bottom);
  cCmdDrawLine(left, bottom, left, top);
}


#ifndef DISPLAY_REALWIDTH
  #define DISPLAY_REALWIDTH   DISPLAY_WIDTH
#endif
//-----------------------------------------------------------------
//cCmdCopyBitMapBits
void cCmdCopyBitMapBits(
  SLONG dst_x,  // left pixel on LCD
  SLONG dst_y,  // bottom pixel on LCD
  SLONG src_x,  // starting pixel x coordinate from source map
  SLONG src_y,  // starting pixel y coordinate from source map
  SLONG src_width, // width in pixels to the right (negative implies to the left)
  SLONG src_height, // height in pixels down (negative implies down)
  IMG_OP_SPRITE * pSprite)
{
  SLONG dy;  // Location in the destination pixmap , the screen that is
  SLONG sx;
  SLONG sy;  // Location in the source pixmap.
  SLONG trim, last_x, last_y, rowbytes;
  UBYTE *pSrcByte;
  UBYTE *pDstBytes;
  UBYTE *pDstByte, *pFirstDstByte;
  UBYTE *pLastDstByte;
  UBYTE bit_y, not_bit_y;
  UBYTE masks[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

  // Data in the image file is row major 8 pixels per byte.top row first.
  // src and dst coordinates treat the bottom left most pixel as (0,0)

  if (!pSprite || pSprite->OpCode!=IMG_SPRITE_ID)
    return;

  pDstBytes = DISP_BUFFER_P;

  // Clip the edges. Modify the source and width as well.
  if (dst_x < 0) {        // bounds check start of x
    trim = (0 - dst_x);
    dst_x = 0;
    src_x += trim;
    src_width -= trim;
    }

  last_x = dst_x + src_width;
  if (last_x > DISPLAY_WIDTH)   // bound check end of x
    last_x = DISPLAY_WIDTH;

  if (dst_y < 0) {        // bound check start of y
    trim  = (0 - dst_y);
    dst_y = 0;
    src_y += trim;  // fix up source as well since we are clipping the start of the loop
    src_height -= trim;
    }

  last_y = dst_y + src_height;
  if (last_y > DISPLAY_HEIGHT)  // bound check end of y
    last_y = DISPLAY_HEIGHT;

  // Convert the 0,0 bottom left origin to the top left 0,0 used by the actual
  // buffer
  last_y = TRANSLATE_Y(last_y);
  dst_y = TRANSLATE_Y(dst_y);

  // The last row is the top most scan line in the LCD Buffer
  // so limit if the copy would copy into memory before the buffer.
  // The first row copied will be the one closest to the bottom of the LCD
  // If that is off screen then limit as well and adjust the start point on the start

  // Copy bits top to top moving down.
  sy = src_y;
  rowbytes = pSprite->RowBytes;

  pSrcByte = pSprite->Bytes + ((pSprite->Rows - 1 - sy) * rowbytes);
  pFirstDstByte =  pDstBytes + ((dst_y >> 3) * DISPLAY_REALWIDTH) + dst_x;
  for (dy = dst_y;  dy > last_y; dy--)
  {
    sx = src_x;
  bit_y = masks[7 - (dy & 0x07)];
  not_bit_y = ~ bit_y;
    pDstByte = pFirstDstByte;
    pLastDstByte = pDstByte + (last_x - dst_x);
    for (; pDstByte < pLastDstByte; pDstByte++)
    {
    if ( *(pSrcByte + (sx >> 3)) & masks[sx & 0x07] ){
      *pDstByte |= bit_y;
    } else {
      *pDstByte &= not_bit_y;
      }
      sx ++;
    }
    pSrcByte -= rowbytes;
    sy ++;
    if ((dy & 0x07) == 0) // bump back the scan line start point at rollover
      pFirstDstByte -= DISPLAY_REALWIDTH;
  }

}

//-----------------------------------------------------------------
// cCmdSetPixel - Set or clear a pixel based on Val
void cCmdSetPixel(SLONG X, SLONG Y, ULONG Val)
{
  Y = TRANSLATE_Y(Y);

  pMapDisplay->pFunc(DISPLAY_PIXEL, (UBYTE)Val, (UBYTE)X, (UBYTE)Y, 0, 0);
}


//-----------------------------------------------------------------
//cCmdWrapSetScreenMode
//ArgV[0]: (Function return) Status code, SBYTE
//ArgV[1]: ScreenMode  ULONG
NXT_STATUS cCmdWrapSetScreenMode(UBYTE * ArgV[])
{
  ULONG ScreenMode = (ULONG)(*ArgV[1]);
  if (ScreenMode == RESTORE_NXT_SCREEN) {
    cCmdRestoreDefaultScreen();
  }

  // Set return value
  *(SBYTE*)(ArgV[0]) = NO_ERR;
  return NO_ERR;
}

//------------------------------------------------------------------
// cCmdClearScreenIfNeeded - Clear entire sceen buffer if explicitly requested or implicitly required.
void cCmdClearScreenIfNeeded(ULONG DrawOptions)
{
  //If we are the first drawing command, clear the screen and record that we've done so
  if (VarsCmd.DirtyDisplay == FALSE)
  {
    VarsCmd.DirtyDisplay = TRUE;
    pMapUi->Flags &= ~UI_ENABLE_STATUS_UPDATE;

    //Override DrawOptions because we have to clear anyway
    DrawOptions = DRAW_OPT_CLEAR_WHOLE_SCREEN;
  }

  if (DRAW_OPT_CLEAR_MODE(DrawOptions))
  {
    pMapDisplay->pFunc(DISPLAY_ERASE_ALL, 0, 0, 0, 0, 0);

    //Clear UpdateMask to kill any pending updates
    pMapDisplay->UpdateMask = 0;
  }

  return;
}

//------------------------------------------------------------------
// cCmdDrawString - Draw string to display buffer
// Properly uses 'Normal' display buffer to avoid conflicts with popup buffer
// Clips text at bottom and right hand edges of the screen buffer
//!!! Function copied and modified from cDisplayString
void cCmdDrawString(UBYTE *pString, ULONG X, ULONG Y)
{
  UBYTE   *pSource;
  UBYTE   *pDestination;
  FONT    *pFont;
  ULONG   FontWidth;
  ULONG   Items;
  ULONG   Item;
  ULONG   Line;

  //Get current font information
  pFont = pMapDisplay->pFont;
  Items = pFont->ItemsX * pFont->ItemsY;

  //Invert Y coordinate to match display buffer
  Y = TRANSLATE_Y(Y);
  Line = (Y & 0xF8) / 8;

  //If text line is out of bounds, do nothing.
  if (Line >= TEXTLINES)
    return;

  //Calculate pointer to first byte of drawing destination
  pDestination = &(DISP_BUFFER_P[Line * DISPLAY_WIDTH + X]);

  while (*pString)
  {
    FontWidth = pFont->ItemPixelsX;
    //Calculate X coordinate of the right edge of this character.
    //If it will extend past the right edge, clip the string.
    X += FontWidth;
    if (X >= DISPLAY_WIDTH)
      break;

    //If Item is defined by the font, display it.  Else, ignore it.
    Item = *pString - ' ';
    if (Item < Items)
    {
      pSource      = (UBYTE*)&(pFont->Data[Item * FontWidth]);
      while (FontWidth--)
      {
        *pDestination = *pSource;
        pDestination++;
        pSource++;
      }
    }
    pString++;
  }
}

//------------------------------------------------------------------
// cCmdRestoreDefaultScreen - Restore to Default 'Running' screen
void cCmdRestoreDefaultScreen(void)
{
  //If this program has taken over the display, reset it for the UI
  if (VarsCmd.DirtyDisplay == TRUE)
  {
    VarsCmd.DirtyDisplay = FALSE;

    pMapDisplay->pFunc(DISPLAY_ERASE_ALL, 0, 0, 0, 0, 0);
    pMapDisplay->UpdateMask = SCREEN_BIT(SCREEN_BACKGROUND);

    pMapUi->Flags |= UI_ENABLE_STATUS_UPDATE | UI_REDRAW_STATUS;
  }
}
