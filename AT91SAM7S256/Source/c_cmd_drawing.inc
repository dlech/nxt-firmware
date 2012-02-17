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

#define MAX_CORNERS 128

//absolute value of a
#define ABS(a)    (((a)<0) ? -(a) : (a))

//take binary sign of a, either -1, or 1 if >= 0
#define SGN(a)    (((a)<0) ? -1 : 1)

#define MAX(a,b)    (((a>b) ? (a) : (b)))
#define MIN(a,b)    (((a<b) ? (a) : (b)))

#define DISP_BUFFER_P ((UBYTE*)&(pMapDisplay->Normal))

//------------------------------------------------------------------
// cCmdClearScreenIfNeeded - Clear entire sceen buffer if explicitly requested or implicitly required.
void cCmdClearScreenIfNeeded(ULONG DrawOptions);

//------------------------------------------------------------------
// cCmdRestorDefaultScreen - Restore screen to default 'Running' screen
void cCmdRestoreDefaultScreen(void);

//------------------------------------------------------------------
// cCmdDrawString - Draw string to display buffer
void cCmdDrawString(UBYTE *pString, ULONG X, ULONG Y, UBYTE InvertMode, UBYTE LogicalMode, UBYTE FillMode);

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
  IMG_NUMBOX_ID = 8,
  IMG_ELLIPSE_ID = 9,
  IMG_POLYGON_ID = 10
};

#define IMG_SYMB_USEARGS(_v) (_v & (SWORD)0xF000)
#define IMG_SYMB_MAP(_v)  ((_v & 0x0F00) >> 8)
#define IMG_SYMB_ARG(_v)  (_v & 0x00FF)

//JJR
//#define DRAW_OPT_CLEAR_PIXELS (0x0004)
//#define SET_PIXELS(_v) (((_v) & DRAW_OPT_CLEAR_PIXELS) ? FALSE : TRUE)

// DrawingOptions
// (and CopyOptions)
//   - parameter used within byte code drawing operations
//   - now unified with 'CopyOptions' parameter in RIC files
// Bits 0 & 1 (values 0,1,2,3) control screen clearing behaviour (Not within RIC files).
// Bit 2 (value 4) controls the NOT operation, i.e. draw in white or invert text/graphics.
// Bits 3 & 4 (values 0,8,16,24) control pixel logical combinations (COPY/AND/OR/XOR).
// Bit 5 (value 32) controls shape filling, or overrides text/graphic bitmaps with set pixels.
// These may be ORed together for the full instruction.
// These operations are resolved into the separate, common parameters
// defined in 'c_display.iom' before any drawing function is called.
// Note that when drawing a RIC file, the initial 'DrawingOptions' parameter
// supplied in the drawing instruction controls screen clearing, but nothing else.
// The 'CopyOptions' parameter from each instruction in the RIC file then controls
// graphic operations, but the screen-clearing bits are ignored.

#define DRAW_OPT_NORMAL                     (0x0000)
#define DRAW_OPT_LOGICAL_COPY               (0x0000)

#define DRAW_OPT_CLEAR_WHOLE_SCREEN         (0x0001)
#define DRAW_OPT_CLEAR_EXCEPT_STATUS_SCREEN (0x0002)
#define DRAW_OPT_INVERT                     (0x0004)
#define DRAW_OPT_LOGICAL_AND                (0x0008)
#define DRAW_OPT_LOGICAL_OR                 (0x0010)
#define DRAW_OPT_LOGICAL_XOR                (0x0018)
#define DRAW_OPT_FILL_SHAPE                 (0x0020)

// Combined parameter masks:
#define DRAW_OPT_CLEAR_SCREEN_MODES         (0x0003)
#define DRAW_OPT_LOGICAL_OPERATIONS         (0x0018)
#define DRAW_OPT_FONT_DIRECTIONS            (0x01C0)

#define DRAW_OPT_FONT_WRAP     (0x0200)

#define DRAW_OPT_FONT_DIR_L2RB (0x0000)           // Font left to right bottom align
#define DRAW_OPT_FONT_DIR_L2RT (0x0040)           // Font left to right top align
#define DRAW_OPT_FONT_DIR_R2LB (0x0080)           // Font right to left bottom align
#define DRAW_OPT_FONT_DIR_R2LT (0x00C0)           // Font right to left top align
#define DRAW_OPT_FONT_DIR_B2TL (0x0100)           // Font bottom to top left align
#define DRAW_OPT_FONT_DIR_B2TR (0x0140)           // Font bottom to top right align
#define DRAW_OPT_FONT_DIR_T2BL (0x0180)           // Font top to bottom left align
#define DRAW_OPT_FONT_DIR_T2BR (0x01C0)           // Font top to bottom right align

// new constant for polyline vs polygon
#define DRAW_OPT_POLYGON_POLYLINE           (0x0400)

#define DESC_FONTOUT    0x8001         // ID for new font out function

// CopyOptions, as originally defined for RIC files, now equate to:
//   COPY      =  0 = 0x00 = DRAW_OPT_NORMAL                        = Copy all bits, solid and clear
//   COPY_NOT  =  4 = 0x04 = DRAW_OPT_INVERT                        = Copy all bits, but invert bitmap images first
//   OR        = 16 = 0x10 = DRAW_OPT_LOGICAL_OR                    = Copy solid bits, ignore clear bits
//   BIT_CLEAR = 12 = 0x0C = DRAW_OPT_INVERT + DRAW_OPT_LOGICAL_AND = Erase screen for solid bits, ignore clear bits
//JJR


// Clear Before Drawing Modes for Draw functions
enum {
  DO_NOT_CLEAR = 0,
  CLEAR_B4_DRAW = 1
};

// Screen Modes for SetScreenMode function
enum {
  RESTORE_NXT_SCREEN = 0,
  CLEAR_NXT_SCREEN = 1
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
  UWORD     CopyOptions;  // Copy, CopyNot, Or, BitClear; //JJR ,Xor;
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

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD   CopyOptions;
  IMG_PT  Pt;
  UWORD   Radius1;
  UWORD   Radius2;
} IMG_OP_ELLIPSE;

typedef struct
{
  IMG_COMMON_FIELDS
  UWORD   CopyOptions;
  UWORD   Count;    //The actual size of the following array.
  IMG_PT  Points[3];    //Minimum of 3 for polygon
} IMG_OP_POLYGON;

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
  IMG_OP_ELLIPSE   Ellipse;
  IMG_OP_POLYGON   Polygon;
} IMG_OP_UNION;

// Variables for DrawImage
#define IMG_MAX_DATA 11
IMG_OP_UNION  * gpImgData[IMG_MAX_DATA];
SLONG         * gpPassedImgVars = NULL;
SWORD           gPassedVarsCount = 0;

// Private Prototypes
void cCmdDrawLine(SLONG x1, SLONG y1, SLONG x2, SLONG y2, UBYTE PixelMode);
void cCmdDrawRect(SLONG left, SLONG bottom, SLONG width, SLONG height, UBYTE PixelMode, UBYTE FillMode);
void cCmdDrawCircle(SLONG cx, SLONG cy, SLONG radius, UBYTE PixelMode, UBYTE FillMode);
void cCmdDrawPolygon(IMG_PT* points, UWORD polyCorners, UBYTE PixelMode, UBYTE FillMode, UBYTE bLine);
void cCmdDrawEllipse(SWORD cx, SWORD cy, SWORD a, SWORD b, UBYTE PixelMode, UBYTE FillMode);
void cCmdCopyBitMapBits(SLONG dst_x, SLONG dst_y,
                        SLONG src_x, SLONG src_y, SLONG src_width, SLONG src_height,
                        IMG_OP_SPRITE * pSprite,
                        UBYTE InvertMode, UBYTE LogicalMode, UBYTE FillMode);
SLONG cCmdResolveValue(SWORD Value);
void cCmdSetPixel(SLONG X, SLONG Y, ULONG PixelMode);

UBYTE cCmdResolveDrawingMode(UWORD DrawingOptions, UBYTE* pPixelMode, UBYTE* pFillMode);
void cCmdResolveBitmapMode(UWORD DrawingOptions, UBYTE* pInvertMode, UBYTE* pLogicalMode, UBYTE* pFillMode);
UBYTE cCmdResolveClearScreenMode(UWORD DrawingOptions);

void cCmdResolveFontOptions(UWORD DrawingOptions, UBYTE* pFontDir, UBYTE* pFontWrap);
void cCmdDrawFont(IMG_OP_SPRITE *pSprite, SLONG sW, SLONG sH, SLONG startx, SLONG starty,
  UBYTE fontDir, UBYTE fontWrap, UBYTE invertMode, UBYTE logicalMode, UBYTE fillMode);


void cCmdDrawTextHelper(ULONG DrawOptions, UBYTE *pString, IMG_PT * pPt)
{
  UBYTE invertMode, logicalMode, fillMode;
  cCmdClearScreenIfNeeded(DrawOptions);
  // Display the String
  cCmdResolveBitmapMode(DrawOptions, &invertMode, &logicalMode, &fillMode);
  cCmdDrawString(pString, (UBYTE)pPt->X, (UBYTE)pPt->Y, invertMode, logicalMode, fillMode);
  pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);
}

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
  cCmdDrawTextHelper(*(ULONG*)ArgV[3], ArgV[2], pPt);
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
  ULONG DrawOptions = *(ULONG*)ArgV[2];
  UBYTE pixelMode, fillMode;

  cCmdClearScreenIfNeeded(DrawOptions);

  // Display the Pixel
  if (cCmdResolveDrawingMode(DrawOptions, &pixelMode, &fillMode))
    cCmdSetPixel(pPt->X, pPt->Y, pixelMode);

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
  ULONG DrawOptions = *(ULONG*)ArgV[3];
  UBYTE pixelMode, fillMode;

  cCmdClearScreenIfNeeded(DrawOptions);

  if (cCmdResolveDrawingMode(DrawOptions, &pixelMode, &fillMode))
    cCmdDrawLine(pPt1->X, pPt1->Y, pPt2->X, pPt2->Y, pixelMode);

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
  IMG_PT * pPt = (IMG_PT*) ArgV[1];
  SLONG radius = *(UBYTE*)ArgV[2];
  ULONG DrawOptions = *(ULONG*)ArgV[3];
  UBYTE pixelMode, fillMode;
  
  cCmdClearScreenIfNeeded(DrawOptions);
  if (cCmdResolveDrawingMode(DrawOptions, &pixelMode, &fillMode))
    cCmdDrawCircle(pPt->X, pPt->Y, radius, pixelMode, fillMode);
    
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
  ULONG DrawOptions = *(ULONG*)ArgV[3];
  UBYTE pixelMode, fillMode;

  cCmdClearScreenIfNeeded(DrawOptions);

  if (cCmdResolveDrawingMode(DrawOptions, &pixelMode, &fillMode))
    cCmdDrawRect(pPt1->X, pPt1->Y, pPt2->X, pPt2->Y, pixelMode, fillMode);

  pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);

  // Set return value
  *((SBYTE*)(ArgV[0])) = NO_ERR;

  return NO_ERR;
}

//-----------------------------------------------------------------
//cCmdWrapDrawPolygon
//ArgV[0]: (Function return) Status byte, SBYTE
//ArgV[1]: Points IMG_PT[]
//ArgV[2]: Options (ULONG)
NXT_STATUS cCmdWrapDrawPolygon(UBYTE * ArgV[])
{
  ULONG DrawOptions = *(ULONG*)ArgV[2];
  UBYTE pixelMode, fillMode, bLine;

  //Resolve array argument
  IMG_PT * Points = (IMG_PT*)cCmdDVPtr(*(DV_INDEX *)(ArgV[1]));
  UWORD polyCorners = DV_ARRAY[*(DV_INDEX *)(ArgV[1])].Count;

  cCmdClearScreenIfNeeded(DrawOptions);
  bLine = (DrawOptions & DRAW_OPT_POLYGON_POLYLINE) ? 1 : 0;
  if (cCmdResolveDrawingMode(DrawOptions, &pixelMode, &fillMode)) {
    if (fillMode == DRAW_SHAPE_FILLED && polyCorners > MAX_CORNERS) {
      // Set return value
      *((SBYTE*)(ArgV[0])) = ERR_INSTR;
      return ERR_INSTR;
    }
    else
      cCmdDrawPolygon(Points, polyCorners, pixelMode, fillMode, bLine);
  }
  
  pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);
  
  // Set return value
  *((SBYTE*)(ArgV[0])) = NO_ERR;
 
  return NO_ERR;
}

//-----------------------------------------------------------------
//cCmdWrapDrawEllipse
//ArgV[0]: (Function return) Status byte, SBYTE
//ArgV[1]: Start Location (IMG_PT *)
//ArgV[2]: Radius1 (U8)
//ArgV[3]: Radius2 (U8)
//ArgV[4]: Options (ULONG)
NXT_STATUS cCmdWrapDrawEllipse(UBYTE * ArgV[])
{
  IMG_PT * pPt = (IMG_PT*) ArgV[1];
  SWORD radius1 = *(UBYTE*)ArgV[2];
  SWORD radius2 = *(UBYTE*)ArgV[3];
  ULONG DrawOptions = *(ULONG*)ArgV[4];
  UBYTE pixelMode, fillMode;
  
  cCmdClearScreenIfNeeded(DrawOptions);
  if (cCmdResolveDrawingMode(DrawOptions, &pixelMode, &fillMode))
    cCmdDrawEllipse(pPt->X, pPt->Y, radius1, radius2, pixelMode, fillMode);
    
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
  if (!IMG_SYMB_USEARGS(Value) || !gpPassedImgVars || (gPassedVarsCount == 0))
  {
    return Value;
  }
  else
  {
    // if we try to access a variable beyond the end of the array we provided
    // then just return the original value
    if (IMG_SYMB_ARG(Value) >= gPassedVarsCount)
      return Value;
      
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


NXT_STATUS cCmdDrawPictureHelper(IMG_OP_UNION** ppImage, IMG_PT* pPoint, SLONG* pVars, ULONG DataSize, ULONG Options)
{
  SLONG OpSize;
  UBYTE gFillMode, gInvertMode, gLogicalMode, gFontDir, gFontWrap;
  UBYTE pixelMode, fillMode;
  UBYTE invertMode, logicalMode;
  IMG_PT Pt;     // Where to draw the picture at (up and to the right)
  NXT_STATUS DStatus = NO_ERR;
  IMG_OP_UNION* pImage = *ppImage;
  // Read the params, Clear the data table.
  Pt = *pPoint;
  gpPassedImgVars = pVars;
  memset(gpImgData,0,sizeof(gpImgData));

  cCmdClearScreenIfNeeded(Options);

  cCmdResolveBitmapMode(Options, &gInvertMode, &gLogicalMode, &gFillMode);
  cCmdResolveFontOptions(Options, &gFontDir, &gFontWrap);

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
          cCmdResolveBitmapMode(pCB->CopyOptions, &invertMode, &logicalMode, &fillMode);
          cCmdCopyBitMapBits(
            (cCmdResolveValue(pCB->Dst.X) + Pt.X),
            (cCmdResolveValue(pCB->Dst.Y) + Pt.Y),
            cCmdResolveValue((pCB->Src.Pt.X)),
            cCmdResolveValue((pCB->Src.Pt.Y)),
            cCmdResolveValue((pCB->Src.Width)),
            cCmdResolveValue((pCB->Src.Height)),
            (IMG_OP_SPRITE*)cCmdGetIMGData(cCmdResolveValue(pCB->DataAddr)),
            invertMode, logicalMode, fillMode);
        }
      }
      break;

      case IMG_LINE_ID:
      {
        if (OpSize >= sizeof(IMG_OP_LINE))
        {
          IMG_OP_LINE * pL = &(pImage->Line);
          cCmdResolveDrawingMode(pL->CopyOptions, &pixelMode, &fillMode);   //JJR
          cCmdDrawLine(
            (cCmdResolveValue(pL->Pt1.X)+Pt.X),
            (cCmdResolveValue(pL->Pt1.Y)+Pt.Y),
            (cCmdResolveValue(pL->Pt2.X)+Pt.X),
            (cCmdResolveValue(pL->Pt2.Y)+Pt.Y),
            pixelMode                             //JJR
          );
        }
      }
      break;

      case IMG_RECTANGLE_ID:
      {
        if (OpSize >= sizeof(IMG_OP_RECT))
        {
          IMG_OP_RECT * pR = &(pImage->Rect);
          cCmdResolveDrawingMode(pR->CopyOptions, &pixelMode, &fillMode);   //JJR
          cCmdDrawRect(
            (SWORD)(cCmdResolveValue(pR->Pt.X)+Pt.X),
            (SWORD)(cCmdResolveValue(pR->Pt.Y)+Pt.Y),
            (SWORD)(cCmdResolveValue(pR->Width)),
            (SWORD)(cCmdResolveValue(pR->Height)),
            pixelMode, fillMode                             //JJR
          );
        }
      }
      break;

      case IMG_CIRCLE_ID:
      {
        if (OpSize >= sizeof(IMG_OP_CIRCLE))
        {
          IMG_OP_CIRCLE * pC = &(pImage->Circle);
          cCmdResolveDrawingMode(pC->CopyOptions, &pixelMode, &fillMode);   //JJR
          cCmdDrawCircle(
            (SWORD)(cCmdResolveValue(pC->Pt.X)+Pt.X),
            (SWORD)(cCmdResolveValue(pC->Pt.Y)+Pt.Y),
            (SWORD)(cCmdResolveValue(pC->Radius)),
            pixelMode, fillMode                             //JJR
          );
        }
      }
      break;

      case IMG_PIXEL_ID:
      {
        if (OpSize >= sizeof(IMG_OP_PIXEL))
        {
          cCmdResolveDrawingMode(pImage->Pixel.CopyOptions, &pixelMode, &fillMode);   //JJR
          cCmdSetPixel(
            (cCmdResolveValue(pImage->Pixel.Pt.X) + Pt.X),
            (cCmdResolveValue(pImage->Pixel.Pt.Y) + Pt.Y),
            pixelMode                                         //JJR
            );
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
          cCmdResolveBitmapMode(pNB->CopyOptions, &invertMode, &logicalMode, &fillMode);
          cCmdDrawString(
            NumStr,
            (UBYTE) (cCmdResolveValue(pNB->Pt.X) + Pt.X),
            (UBYTE) (cCmdResolveValue(pNB->Pt.Y) + Pt.Y),
            invertMode, logicalMode, fillMode                     //JJR
            );
        }
      }
      break;

      case IMG_ELLIPSE_ID:
      {
        if (OpSize >= sizeof(IMG_OP_ELLIPSE))
        {
          IMG_OP_ELLIPSE * pE = &(pImage->Ellipse);
          cCmdResolveDrawingMode(pE->CopyOptions, &pixelMode, &fillMode);   //JJR
          cCmdDrawEllipse(
            (SWORD)(cCmdResolveValue(pE->Pt.X)+Pt.X),
            (SWORD)(cCmdResolveValue(pE->Pt.Y)+Pt.Y),
            (SWORD)(cCmdResolveValue(pE->Radius1)),
            (SWORD)(cCmdResolveValue(pE->Radius2)),
            pixelMode, fillMode                             //JJR
          );
        }
      }
      break;

      case IMG_POLYGON_ID:
      {
        if (OpSize >= sizeof(IMG_OP_POLYGON))
        {
          IMG_OP_POLYGON * pP = &(pImage->Polygon);
          UBYTE bLine = (pP->CopyOptions & DRAW_OPT_POLYGON_POLYLINE) ? 1 : 0;
          cCmdResolveDrawingMode(pP->CopyOptions, &pixelMode, &fillMode);   //JJR
          // resolve all the values in the struct
          if (fillMode != DRAW_SHAPE_FILLED || pP->Count <= MAX_CORNERS)
          {
            for(int i=0;i<pP->Count; i++) {
              pP->Points[i].X = (SWORD)(cCmdResolveValue(pP->Points[i].X)+Pt.X);
              pP->Points[i].Y = (SWORD)(cCmdResolveValue(pP->Points[i].Y)+Pt.Y);
            }
            cCmdDrawPolygon(pP->Points, pP->Count, pixelMode, fillMode, bLine);
          }
        }
      }
      break;

      case IMG_DESCRIPTION_ID:
      {
        switch( pImage->Desc.Options )
        {
          case DESC_FONTOUT : 
            cCmdDrawFont((IMG_OP_SPRITE*)cCmdGetIMGData(1), pImage->Desc.Width,
              pImage->Desc.Height, Pt.X, Pt.Y, gFontDir, gFontWrap, gInvertMode,
              gLogicalMode, gFillMode);
            break;
        }
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
  gpPassedImgVars = NULL;  
  memset(gpImgData,0,sizeof(gpImgData));
  pMapDisplay->UpdateMask |= SCREEN_BIT(SCREEN_BACKGROUND);
  return DStatus;
}

void cCmdDrawPictureFromFile(UBYTE *pFilename, IMG_PT* pPoint, SLONG* pVars, ULONG Options, SBYTE * pReturnVal)
{
  LOADER_STATUS LStatus;
  ULONG         DataSize;
  UBYTE   ImageHandle;
  IMG_OP_UNION  * pImage;

  //Open the file in memory map mode. return if failure.
  LStatus = pMapLoader->pFunc(OPENREADLINEAR, pFilename, (UBYTE*)(&pImage), &DataSize);
  ImageHandle = LOADER_HANDLE(LStatus);

  //If error opening file, give up and write loader status back to user.
  if (LOADER_ERR(LStatus) != SUCCESS || pImage == NULL)
  {
    *pReturnVal = (SBYTE)(LOADER_ERR_BYTE(LStatus));
    return;
  }
  //Else, start interpretting the file
  else
  {
    *pReturnVal = cCmdDrawPictureHelper(&pImage, pPoint, pVars, DataSize, Options);
  }
  pMapLoader->pFunc(CLOSE, &ImageHandle, NULL, NULL);
}

//-----------------------------------------------------------------
//cCmdWrapDrawPicture
//ArgV[0]: (Function return) Status Byte, SBYTE
//ArgV[1]: Left Top (IMG_PT *)
//ArgV[2]: Filename, CStr
//ArgV[3]: Variables, array of I32
//ArgV[4]: Options (ULONG)
NXT_STATUS cCmdWrapDrawPicture(UBYTE * ArgV[])
{
  SBYTE * pReturnVal =  (SBYTE *)ArgV[0];
  IMG_PT* pPt        =  (IMG_PT*)ArgV[1];
  ULONG   Options    = *(ULONG *)ArgV[4];

  // count the variables
  gPassedVarsCount = DV_ARRAY[*(DV_INDEX *)(ArgV[3])].Count;

  //Resolve array argument
  ArgV[2] = (UBYTE*)cCmdDVPtr(*(DV_INDEX *)(ArgV[2]));
  ArgV[3] = (UBYTE*)cCmdDVPtr(*(DV_INDEX *)(ArgV[3]));

  cCmdDrawPictureFromFile((UBYTE *)ArgV[2], pPt, (SLONG *)ArgV[3], Options, pReturnVal);

  return (NO_ERR);
}

//-----------------------------------------------------------------
//cCmdWrapDrawPictureArray
//ArgV[0]: (Function return) Status Byte, SBYTE
//ArgV[1]: Left Top (IMG_PT *)
//ArgV[2]: Data, array of U8
//ArgV[3]: Variables, array of I32
//ArgV[4]: Options (ULONG)
NXT_STATUS cCmdWrapDrawPictureArray(UBYTE * ArgV[])
{
  SBYTE *  pReturnVal =  (SBYTE   *)ArgV[0];
  IMG_PT*  pPt        =  (IMG_PT  *)ArgV[1];
  DV_INDEX DVIndex    = *(DV_INDEX*)ArgV[2];
  ULONG    Options    = *(ULONG   *)ArgV[4];
  ULONG DataSize = DV_ARRAY[DVIndex].Count;
  IMG_OP_UNION* pImage;

  // count the variables
  gPassedVarsCount = DV_ARRAY[*(DV_INDEX *)(ArgV[3])].Count;

  //Resolve array argument
  pImage = (IMG_OP_UNION*)cCmdDVPtr(DVIndex);
  ArgV[3] = (UBYTE*)cCmdDVPtr(*(DV_INDEX *)(ArgV[3])); // pVar

  *pReturnVal = cCmdDrawPictureHelper((IMG_OP_UNION**)&pImage, pPt, (SLONG*)ArgV[3], DataSize, Options);
  return (NO_ERR);
}

//-----------------------------------------------------------------
//cCmdWrapDrawFont
//ArgV[0]: (Function return) Status Byte, SBYTE
//ArgV[1]: Left Top (IMG_PT *)
//ArgV[2]: FontFilename, CStr
//ArgV[3]: Text, CStr
//ArgV[4]: Options (ULONG)
NXT_STATUS cCmdWrapDrawFont(UBYTE * ArgV[])
{
  SBYTE  * pReturnVal =  (SBYTE *)ArgV[0];
  IMG_PT * pPt        =  (IMG_PT*)ArgV[1];
  ULONG    Options    = *(ULONG *)ArgV[4];
  
  // count the number of characters
  gPassedVarsCount = DV_ARRAY[*(DV_INDEX *)(ArgV[3])].Count;

  //Resolve array arguments
  ArgV[2] = (UBYTE*)cCmdDVPtr(*(DV_INDEX *)(ArgV[2]));
  ArgV[3] = (UBYTE*)cCmdDVPtr(*(DV_INDEX *)(ArgV[3]));
  
  if (strlen((PSZ)ArgV[2]) == 0) {
    // if font filename is empty then use draw text instead
    cCmdDrawTextHelper(Options, ArgV[3], pPt);
    // Set return value
    *pReturnVal = NO_ERR;
    return NO_ERR;
  }
  else
  {
    // count the variables
    gPassedVarsCount = MIN(strlen((PSZ)ArgV[3]), 256);
    // copy data from input string into SLONG* array
    SLONG Vars[256];
    for(int i=0; i<256;i++) {
      if (i < gPassedVarsCount)
        Vars[i] = ArgV[3][i];
      else
        Vars[i] = 0;
    }
    cCmdDrawPictureFromFile((UBYTE *)ArgV[2], pPt, (SLONG*)Vars, Options, pReturnVal);
    return (NO_ERR);
  }
}

//-----------------------------------------------------------------
// cCmdDrawLine - draw a line.  Line is clipped to screen boundaries.
void cCmdDrawLine(
  SLONG x1,
  SLONG y1,
  SLONG x2,
  SLONG y2,
  UBYTE PixelMode)    //JJR
{
  SLONG tx, ty;       //JJR
  SLONG dx, dy;       //JJR

  dx = x2-x1;         //JJR
  dy = y2-y1;         //JJR

  //Clip line ends vertically - easier if y1<y2:
  if (y1 > y2) {tx=x1; x1=x2; x2=tx;
                ty=y1; y1=y2; y2=ty;}
  
  //Is line completely off screen?
  if (y2<0 || y1>=DISPLAY_HEIGHT) return;
  
  //Trim y1 end:
  if (y1 < 0)
  {
    if (dx && dy)
      x1 = x1 + (((0-y1)*dx)/dy);
    y1 = 0;
  }
  //Trim y2 end:
  if (y2 > DISPLAY_HEIGHT-1)
  {
    if (dx && dy)
      x2 = x2 - (((y2-(DISPLAY_HEIGHT-1))*dx)/dy);
    y2 = DISPLAY_HEIGHT-1;
  }
    
  //Clip horizontally - easier if x1<x2
  if (x1 > x2) {tx=x1; x1=x2; x2=tx;
                ty=y1; y1=y2; y2=ty;}
                
  //Is line completely off screen?
  if (x2<0 || x1>=DISPLAY_WIDTH) return;
  
  //Trim x1 end:
  if (x1 < 0)
    {
      if (dx && dy)
        y1 = y1 + (((0-x1)*dy)/dx);
      x1 = 0;
    }
  //Trim x2 end:
  if (x2 > DISPLAY_WIDTH-1)
    {
      if (dx && dy)
        y2 = y2 - (((x2-(DISPLAY_WIDTH-1))*dy)/dx);
      x2 = DISPLAY_WIDTH-1;
    }
  
  if (x1 == x2) {
    // vertical line or a single point
    if (y1 == y2)
      //cCmdSetPixel(x1, y1, Set);        //JJR
      pMapDisplay->pFunc(DISPLAY_PIXEL, (UBYTE)PixelMode, (UBYTE)x1, (UBYTE)TRANSLATE_Y(y1), 0, 0);   //JJR
    else
    {
      //pMapDisplay->pFunc(DISPLAY_VERTICAL_LINE, Set, x1, TRANSLATE_Y(y1), 0, TRANSLATE_Y(y2));        //JJR
      pMapDisplay->pFunc(DISPLAY_VERTICAL_LINE, PixelMode, x1, TRANSLATE_Y(y1), 0, TRANSLATE_Y(y2));    //JJR
    }
  }
  else if (y1 == y2) {
  
    // horizontal line (single point already dealt with)
      pMapDisplay->pFunc(DISPLAY_HORIZONTAL_LINE, PixelMode, x1, TRANSLATE_Y(y1), x2, 0);   //JJR
  }
  else {
    SLONG d,x,y,ax,ay,sx,sy,dx,dy;
    // Initialize variables
    dx = x2-x1;  ax = ABS(dx)<<1;  sx = SGN(dx);
    dy = y2-y1;  ay = ABS(dy)<<1;  sy = SGN(dy);
    x = x1;
    y = y1;
    if (ax>ay)
    {    // x dominant
      d = ay-(ax>>1);
      for (;;)
      {
        //cCmdSetPixel(x, y, Set);        //JJR
        pMapDisplay->pFunc(DISPLAY_PIXEL, (UBYTE)PixelMode, (UBYTE)x, (UBYTE)TRANSLATE_Y(y), 0, 0);   //JJR
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
    {      // y dominant
      d = ax-(ay>>1);
      for (;;)
      {
        //cCmdSetPixel(x, y, Set);        //JJR
        pMapDisplay->pFunc(DISPLAY_PIXEL, (UBYTE)PixelMode, (UBYTE)x, (UBYTE)TRANSLATE_Y(y), 0, 0);   //JJR
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
}


//-----------------------------------------------------------------
// cCmdDrawRect - draw a rectangle. 
void cCmdDrawRect(
  SLONG left,
  SLONG bottom,
  SLONG width,
  SLONG height,
  UBYTE PixelMode,    //JJR
  UBYTE FillMode)     //JJR
{
  SLONG x1, y1;
  SLONG x2, y2;
  SLONG t;
  
  x1 = left;
  x2 = left + width;
/*
  if (width > 0)
    x2 = left + width - 1;
  else if (width < 0)
    x2 = left + width + 1;
  else
    x2 = x1;
*/
  y1 = bottom;
  y2 = bottom + height;
/*
  if (height > 0)
    y2 = bottom + height - 1;
  else if (height < 0)
    y2 = bottom + height + 1;
  else
    y2 = y1; 
*/

  if (x1>x2) {t = x1; x1 = x2; x2 = t;}
  if (y1>y2) {t = y1; y1 = y2; y2 = t;}  

  if (y2 == y1 || x2 == x1) {
    // height == 0 so draw a single pixel horizontal line OR
    // width == 0 so draw a single pixel vertical line
    cCmdDrawLine(x1, y1, x2, y2, PixelMode);
    return;
  }
  // rectangle has abs(width) or abs(height) >= 1
  if (FillMode == DRAW_SHAPE_FILLED)
  {
    if (x2<0 || y2<0 || x1>DISPLAY_WIDTH-1 || y1>DISPLAY_HEIGHT-1) return;
    if (x1<0) x1=0;
    if (y1<0) y1=0;
    if (x2>DISPLAY_WIDTH-1) x2=DISPLAY_WIDTH-1;
    if (y2>DISPLAY_HEIGHT-1) y2=DISPLAY_HEIGHT-1;
    pMapDisplay->pFunc(DISPLAY_FILL_REGION, PixelMode, x1, TRANSLATE_Y(y2), x2-x1+1, y2-y1+1);
  }
  else
  {
    //Use the full line drawing functions rather than horizontal/vertical
    //functions so these get clipped properly.  These will fall straight
    //through to the faster functions anyway.
    //Also don't re-draw parts of slim rectangles since XOR might be on.
  
    cCmdDrawLine(x1, y1, x2, y1, PixelMode);
    if (y2>y1)
    {
      cCmdDrawLine(x1, y2, x2, y2, PixelMode);
      if (y2 > y1+1)
      {
        cCmdDrawLine(x2, y1+1, x2, y2-1, PixelMode);
        if (x2>x1)
          cCmdDrawLine(x1, y1+1, x1, y2-1, PixelMode);
      }
    }
  }
  
}

void cCmdDrawPolygon(IMG_PT* points, UWORD polyCorners, UBYTE PixelMode, UBYTE FillMode, UBYTE bLine)
{
  // the fill mode overrides the polyline vs polygon setting
  if (FillMode == DRAW_SHAPE_FILLED) 
  {
    int nodes;
    int nodeX[MAX_CORNERS];
    int pixelY;
    int i, j;
    int swap;
    int IMAGE_TOP    = -1;
    int IMAGE_BOTTOM = 1024;
    int IMAGE_RIGHT  = -1;
    int IMAGE_LEFT   = 1024;
    // calculate maximum and minimum X and Y coordinate values
    for(i=0;i<polyCorners;i++) {
      int tmpY = points[i].Y;
      int tmpX = points[i].X;
      if (tmpY>IMAGE_TOP)    IMAGE_TOP    = tmpY;
      if (tmpY<IMAGE_BOTTOM) IMAGE_BOTTOM = tmpY;
      if (tmpX>IMAGE_RIGHT)  IMAGE_RIGHT  = tmpX;
      if (tmpX<IMAGE_LEFT)   IMAGE_LEFT   = tmpX;
    }

    //  Loop through the rows of the image.
    for (pixelY=IMAGE_BOTTOM; pixelY<IMAGE_TOP; pixelY++) {
      //  Build a list of nodes.
      nodes=0; j=polyCorners-1;
      for (i=0; i<polyCorners; i++) {
        IMG_PT pI, pJ;
        pI = points[i];
        pJ = points[j];
        if ((pI.Y<pixelY && pJ.Y>=pixelY) ||
            (pJ.Y<pixelY && pI.Y>=pixelY)) {
          nodeX[nodes++]=(pI.X+(pixelY-pI.Y)*(pJ.X-pI.X)/(pJ.Y-pI.Y));
        }
        j=i;
      }

      //  Sort the nodes, via a simple “Bubble” sort.
      i=0;
      while (i<nodes-1) {
        if (nodeX[i]>nodeX[i+1]) {
          swap=nodeX[i];
          nodeX[i]=nodeX[i+1];
          nodeX[i+1]=swap;
          if (i) i--;
        }
        else
          i++;
      }

      //  Fill the pixels between node pairs.
      for (i=0; i<nodes; i+=2) {
        if (nodeX[i  ]>=IMAGE_RIGHT) break;
        if (nodeX[i+1]> IMAGE_LEFT) {
          if (nodeX[i  ]< IMAGE_LEFT)  nodeX[i  ]=IMAGE_LEFT;
          if (nodeX[i+1]> IMAGE_RIGHT) nodeX[i+1]=IMAGE_RIGHT;
          cCmdDrawLine(nodeX[i], pixelY, nodeX[i+1], pixelY, PixelMode);
        }
      }
    }
  }
  else
  {
//    for(int i = 0, j = polyCorners-1; i<polyCorners; j=i++) {
    IMG_PT pI, pJ;
    int i = 1, j = 0;
    while (i < polyCorners) {
      pI = points[i];
      pJ = points[j];
      cCmdDrawLine(pI.X, pI.Y, pJ.X, pJ.Y, PixelMode);
      j=i++;
    }
    if (!bLine) {
      i = 0;
      pI = points[i];
      pJ = points[j];
      cCmdDrawLine(pI.X, pI.Y, pJ.X, pJ.Y, PixelMode);
    }
  }
}

//-----------------------------------------------------------------
// cCmdDrawEllipse - draw an ellipse. All clipping is done by the set pixel function.
void cCmdDrawEllipse(SWORD xc, SWORD yc, SWORD a, SWORD b, UBYTE PixelMode, UBYTE FillMode)  //JCH
{			/* e(x,y) = b^2*x^2 + a^2*y^2 - a^2*b^2 */
  SWORD x = 0, y = b;
  SWORD rx = x, ry = y;
  UWORD width = 1;
  UWORD height = 1;
  SLONG a2 = (SLONG)a*a; 
  SLONG b2 = (SLONG)b*b;
  SLONG crit1 = -(a2/4 + a%2 + b2);
  SLONG crit2 = -(b2/4 + b%2 + a2);
  SLONG crit3 = -(b2/4 + b%2);
  SLONG t = -a2*y; /* e(x+1/2,y-1/2) - (a^2+b^2)/4 */
  SLONG dxt = 2*b2*x, dyt = -2*a2*y;
  SLONG d2xt = 2*b2, d2yt = 2*a2;

  if (b == 0) {
    cCmdDrawRect(xc-a, yc, 2*a, 0, PixelMode, FillMode);
    return;
  }
  if (a == 0) {
    cCmdDrawRect(xc, yc-b, 0, 2*b, PixelMode, FillMode);
    return;
  }

  while (y>=0 && x<=a) 
  {
    if (FillMode != DRAW_SHAPE_FILLED)
    {
      cCmdSetPixel(xc+x, yc+y,  PixelMode);
      if (x!=0 || y!=0)
        cCmdSetPixel(xc-x, yc-y,  PixelMode);
      if (x!=0 && y!=0) 
      {
        cCmdSetPixel(xc+x, yc-y,  PixelMode);
        cCmdSetPixel(xc-x, yc+y,  PixelMode);
      }
    }
    if (t + b2*x <= crit1 ||     /* e(x+1,y-1/2) <= 0 */
        t + a2*y <= crit3)       /* e(x+1/2,y) <= 0 */
    {
      if (FillMode == DRAW_SHAPE_FILLED)
      {
        if (height == 1)
          ; /* draw nothing */
        else if (ry*2+1 > (height-1)*2) 
        {
          cCmdDrawRect(xc-rx, yc-ry, width-1, height-1, PixelMode, FillMode);
          cCmdDrawRect(xc-rx, yc+ry, width-1, -(height-1), PixelMode, FillMode);
          ry -= height-1;
          height = 1;
        }
        else 
        {
          cCmdDrawRect(xc-rx, yc-ry, width-1, ry*2, PixelMode, FillMode);
          ry -= ry;
          height = 1;
        }
        rx++;
        width += 2;
      }
      x++; 
      dxt += d2xt; 
      t += dxt;
    }
    else if (t - a2*y > crit2)   /* e(x+1/2,y-1) > 0 */
    { 
      y--; 
      dyt += d2yt; 
      t += dyt;
      if (FillMode == DRAW_SHAPE_FILLED)
        height++;
    }
    else 
    {
      if (FillMode == DRAW_SHAPE_FILLED)
      {
        if (ry*2+1 > height*2)
        {
          cCmdDrawRect(xc-rx, yc-ry, width-1, height-1, PixelMode, FillMode);
          cCmdDrawRect(xc-rx, yc+ry, width-1, -(height-1), PixelMode, FillMode);
        }
        else 
        {
          cCmdDrawRect(xc-rx, yc-ry, width-1, ry*2, PixelMode, FillMode);
        }
        width += 2;
        ry -= height;
        height = 1;
        rx++;
      }
      x++; 
      dxt += d2xt; 
      t += dxt;
      y--; 
      dyt += d2yt; 
      t += dyt;
    }
  }
  if (FillMode == DRAW_SHAPE_FILLED)
  {
    if (ry > height) {
      cCmdDrawRect(xc-rx, yc-ry, width-1, height-1, PixelMode, FillMode);
      cCmdDrawRect(xc-rx, yc+ry, width-1, -(height-1), PixelMode, FillMode);
    }
    else {
      cCmdDrawRect(xc-rx, yc-ry, width-1, ry*2, PixelMode, FillMode);
    }
  }
}

//-----------------------------------------------------------------
// cCmdDrawCircle - draw a circle. All clipping is done by the set pixel function.
//void cCmdDrawCircle(SLONG cx, SLONG cy, SLONG radius, UBYTE Set)                      //JJR
void cCmdDrawCircle(SLONG cx, SLONG cy, SLONG radius, UBYTE PixelMode, UBYTE FillMode)  //JJR
{
  cCmdDrawEllipse(cx, cy, radius, radius, PixelMode, FillMode);
}
/*
//-----------------------------------------------------------------
// cCmdDrawCircle - draw a circle. All clipping is done by the set pixel function.
//void cCmdDrawCircle(SLONG cx, SLONG cy, SLONG radius, UBYTE Set)                      //JJR
void cCmdDrawCircle(SLONG cx, SLONG cy, SLONG radius, UBYTE PixelMode, UBYTE FillMode)  //JJR
{
  SLONG f, ddF_x, ddF_y, x, y;
  SLONG ox, oy;                 //JJR
  
  f = 1 - radius;
  ddF_x = 0;
  ddF_y = -2 * radius;
  x = 0;
  y = abs(radius);    //JJR
  
//JJR
  if (y==0)
  {
    cCmdSetPixel(cx, cy + radius,  PixelMode);
    return;
  }
  
  if (FillMode == DRAW_SHAPE_FILLED)
  {
    cCmdDrawLine(cx-radius, cy, cx+radius, cy, PixelMode);
  }
  else
  {
    cCmdSetPixel(cx + radius, cy,  PixelMode);
    cCmdSetPixel(cx - radius, cy,  PixelMode);
  }
  
  if (FillMode != DRAW_SHAPE_FILLED || y==1)
  {
    cCmdSetPixel(cx, cy + radius,  PixelMode);
    cCmdSetPixel(cx, cy - radius,  PixelMode);
  }
  
  if (y==1) return;
  
  
  while (x < y)
  {
     ox = x;         //JJR
     oy = y;         //JJR
     if(f >= 0) 
     {
       y--;
       ddF_y += 2;
       f += ddF_y;
     }
     x++;
     ddF_x += 2;
     f += ddF_x + 1;    
     
//JJR
    //Need to avoid duplicating pixels if drawing with XOR:
    
  if (FillMode == DRAW_SHAPE_FILLED)
  {
    //Fill by drawing successive horizontal lines above and below
    //a horizontal centre line.
    if (x <= y)
    {
      cCmdDrawLine(cx-y, cy+x, cx+y, cy+x, PixelMode);
      cCmdDrawLine(cx-y, cy-x, cx+y, cy-x, PixelMode);
    }
    //Only draw horizontal lines from the top and bottom if:
    //i. The line doesn't overlap with the one just drawn, and the top
    //   curve has just stepped down a pixel or:
    //ii. A line wasn't just drawn and the loop is about to exit.
    if ((x<oy && y!=oy) || (x>y && ox<oy))
    {
      cCmdDrawLine(cx-ox, cy+oy, cx+ox, cy+oy, PixelMode);
      cCmdDrawLine(cx-ox, cy-oy, cx+ox, cy-oy, PixelMode);
    }
  }
  else
  {
    if (x <= y)
    {
      cCmdSetPixel(cx + x, cy + y,  PixelMode);
      cCmdSetPixel(cx - x, cy + y,  PixelMode);
      cCmdSetPixel(cx + x, cy - y,  PixelMode);
      cCmdSetPixel(cx - x, cy - y,  PixelMode);
    }
    if (x < y)
    {
      cCmdSetPixel(cx + y, cy + x,  PixelMode);
      cCmdSetPixel(cx - y, cy + x,  PixelMode);
      cCmdSetPixel(cx + y, cy - x,  PixelMode);
      cCmdSetPixel(cx - y, cy - x,  PixelMode);
    }
  }
  }
}
*/

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
  IMG_OP_SPRITE * pSprite, UBYTE InvertMode, UBYTE LogicalMode, UBYTE FillMode)
{
  SLONG dy;  // Location in the destination pixmap , the screen that is
  SLONG sx;
  SLONG sy;  // Location in the source pixmap.
  SLONG trim, last_x, last_y, rowbytes;
  UBYTE *pSrcByte;
  UBYTE srcByte;      //JJR
  UBYTE *pDstBytes;
  UBYTE *pDstByte, *pFirstDstByte;
  UBYTE *pLastDstByte;
  UBYTE bit_y;                //JJR
  UBYTE masks[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

  // Data in the image file is row major 8 pixels per byte. top row first.
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
    // not_bit_y = ~ bit_y;          //JJR
    pDstByte = pFirstDstByte;
    pLastDstByte = pDstByte + (last_x - dst_x);
    for (; pDstByte < pLastDstByte; pDstByte++)
    {
      //Read source byte:
      //If fill mode is on, pretend the source bitmap is solid:
      if (FillMode==DRAW_SHAPE_FILLED)
        srcByte = 0xff;
      else
        srcByte =  *(pSrcByte + (sx >> 3));                     //JJR
      
      //If invert mode is on, invert the source byte:
      if (InvertMode==DRAW_BITMAP_INVERT) srcByte = ~srcByte;   //JJR

      //Test the pixel in the source byte:
      if ( srcByte & masks[sx & 0x07] )                         //JJR
      {
        //If pixel is set in source image:
        switch (LogicalMode)
        {
          case DRAW_LOGICAL_AND:
            break;
          case DRAW_LOGICAL_XOR:
            *pDstByte ^= bit_y;
            break;
          case DRAW_LOGICAL_OR:
          case DRAW_LOGICAL_COPY:
          default:
            *pDstByte |= bit_y;
            break;
        }
      }
      else
      {
        //If pixel is clear in source image:
        switch (LogicalMode)
        {
          case DRAW_LOGICAL_OR:
          case DRAW_LOGICAL_XOR:
            break;
          case DRAW_LOGICAL_AND:
          case DRAW_LOGICAL_COPY:
          default:
            *pDstByte &= ~bit_y;
            break;
        }
      }
//JJR
      sx ++;
    }
    pSrcByte -= rowbytes;
    sy ++;
    if ((dy & 0x07) == 0) // bump back the scan line start point at rollover
      pFirstDstByte -= DISPLAY_REALWIDTH;
  }

}

//-----------------------------------------------------------------
// cCmdSetPixel - Set, clear or invert a pixel based on PixelMode
void cCmdSetPixel(SLONG X, SLONG Y, ULONG PixelMode)
{
  Y = TRANSLATE_Y(Y);

  if (X>=0 && X<DISPLAY_WIDTH && Y>=0 && Y<DISPLAY_HEIGHT)
    pMapDisplay->pFunc(DISPLAY_PIXEL, (UBYTE)PixelMode, (UBYTE)X, (UBYTE)Y, 0, 0);
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
  else if (ScreenMode == CLEAR_NXT_SCREEN) {
    pMapDisplay->pFunc(DISPLAY_ERASE_ALL, 0, 0, 0, 0, 0);
  }

  // Set return value
  *(SBYTE*)(ArgV[0]) = NO_ERR;
  return NO_ERR;
}

void cDirtyDisplay(ULONG* DrawOptions)
{
  //If we are the first drawing command mark the display as dirty
  if (VarsCmd.DirtyDisplay == FALSE)
  {
    VarsCmd.DirtyDisplay = TRUE;
    pMapUi->Flags &= ~UI_ENABLE_STATUS_UPDATE;
    
    if (DrawOptions) {
      //Override DrawOptions because we have to clear anyway
      *DrawOptions = DRAW_OPT_CLEAR_WHOLE_SCREEN;
    }
  }
}

//
//cCmdWrapDisplayExecuteFunction
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: UBYTE Cmd
//ArgV[2]: UBYTE On
//ArgV[3]: UBYTE X1
//ArgV[4]: UBYTE Y1
//ArgV[5]: UBYTE X2
//ArgV[6]: UBYTE Y2
//
NXT_STATUS cCmdWrapDisplayExecuteFunction(UBYTE * ArgV[])
{
  cDirtyDisplay(NULL);
  UBYTE Cmd = *(UBYTE*)(ArgV[1]);
  UBYTE Y1 = *(UBYTE*)(ArgV[4]);
  UBYTE Y2 = *(UBYTE*)(ArgV[6]);
  if (Cmd != DISPLAY_CHAR)
    Y1 = TRANSLATE_Y(Y1);
  if (Cmd == DISPLAY_VERTICAL_LINE)
    Y2 = TRANSLATE_Y(Y2);
  pMapDisplay->pFunc(Cmd, 
                     *(UBYTE*)(ArgV[2]),
                     *(UBYTE*)(ArgV[3]),
                     Y1,
                     *(UBYTE*)(ArgV[5]),
                     Y2);
  *(SBYTE*)(ArgV[0]) = NO_ERR;
  return (NO_ERR);
}


//------------------------------------------------------------------
// cCmdClearScreenIfNeeded - Clear entire sceen buffer if explicitly requested or implicitly required.
void cCmdClearScreenIfNeeded(ULONG DrawOptions)
{
  cDirtyDisplay(&DrawOptions);

  if (cCmdResolveClearScreenMode(DrawOptions))
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
//void cCmdDrawString(UBYTE *pString, ULONG X, ULONG Y)                                                       //JJR
void cCmdDrawString(UBYTE *pString, ULONG X, ULONG Y, UBYTE InvertMode, UBYTE LogicalMode, UBYTE FillMode)    //JJR
{
  UBYTE   *pSource;
  UBYTE   *pDestination1;
  UBYTE   *pDestination2;
  UBYTE   a;
  FONT    *pFont;
  ULONG   FontWidth;
  ULONG   Items;
  ULONG   Item;
  SLONG   Line;
  UBYTE   offset;
  UWORD   a2;
  UWORD   mask;
  UWORD   font;

  //Get current font information
  pFont = pMapDisplay->pFont;
  Items = pFont->ItemsX * pFont->ItemsY;

  //Invert Y coordinate to match display buffer
  Y = TRANSLATE_Y(Y);
  Line = (Y & 0xF8) / 8; // truncates down to nearest multiple of 8
  offset = (Y-7) % 8; // how many bits is the content shifted?
  if (offset)
    Line--;
    
  mask = ~(0x00ff << offset);

  //If text line is out of bounds, do nothing.
  if (Line >= TEXTLINES)
    return;

  //Calculate pointer to first byte of drawing destination
  if (Line >= TEXTLINE_1)
    pDestination1 = &(DISP_BUFFER_P[Line * DISPLAY_WIDTH + X]);
  else
    pDestination1 = NULL;
  if (offset && (Line < TEXTLINE_8))
    pDestination2 = &(DISP_BUFFER_P[(Line+1) * DISPLAY_WIDTH + X]);
  else
    pDestination2 = NULL;

  // bail out early if neither destination is valid
  if (pDestination1 == NULL && pDestination2 == NULL)
    return;

  while (*pString)
  {
    FontWidth = pFont->ItemPixelsX;
    //Calculate X coordinate of the right edge of this character.
    //If it will extend past the right edge, clip the string.
    X += FontWidth;
    if (X > DISPLAY_WIDTH) // JCH: was >= This clipped text that ended at pixel 99
      break;

    //If Item is defined by the font, display it.  Else, ignore it.
    Item = *pString - ' ';
    if (Item < Items)
    {
      pSource      = (UBYTE*)&(pFont->Data[Item * FontWidth]);
      while (FontWidth--)
      {
//JJR

          //Fetch a byte from the source bitmap:
          //If fill mode is on, pretend the source bitmap is solid:
          if (FillMode==DRAW_SHAPE_FILLED)
            a = 0xff;
          else
            a = *pSource;
            
          //Implement bitmap invert mode:
          if (InvertMode==DRAW_BITMAP_INVERT) a = ~a;

          // grab data from 1 or 2 lines 
          a2 = (pDestination2 ? ((UWORD)(*pDestination2) << 8) : 0x0000) |
               (pDestination1 ? (UWORD)(*pDestination1) : 0x0000);

          font = (UWORD) a << offset;
          
          //Implement bitmap logical mode when writing on screen:
          switch (LogicalMode)
          {
            case DRAW_LOGICAL_OR:
              a2 |= font;
              break;
            case DRAW_LOGICAL_AND:
              a2 &= font | mask;
              break;
            case DRAW_LOGICAL_XOR:
              a2 ^= font;
              break;
            case DRAW_LOGICAL_COPY:
            default:
              a2 = (a2 & mask) | font;
              break;
          }
//JJR
        if (pDestination1) {
          *pDestination1 = (UBYTE)(a2 & 0xFF);
          pDestination1++;
        }
        if (pDestination2) {
          *pDestination2 = ((UBYTE)(a2>>8) & 0xFF);
          pDestination2++;
        }
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
  // restore default display state
  pMapDisplay->Display = (UBYTE*)pMapDisplay->Normal;
  pMapDisplay->Flags = DISPLAY_REFRESH | DISPLAY_ON;
}

//------------------------------------------------------------------
// cCmdResolveDrawingMode - extract the parameters that affect pixel, line and shape
//                          drawing from the 'DrawingOptions' or 'CopyOptions' parameter.
//                          Return FALSE for combinations that do nothing.

UBYTE cCmdResolveDrawingMode(UWORD DrawingOptions, UBYTE* pPixelMode, UBYTE* pFillMode)
{
  DrawingOptions = cCmdResolveValue(DrawingOptions);
  // Extract shape fill option:
  if (DrawingOptions & DRAW_OPT_FILL_SHAPE)
    *pFillMode = DRAW_SHAPE_FILLED;
  else
    *pFillMode = DRAW_SHAPE_HOLLOW;
  
  // Extract pixel drawing options:
  if (DrawingOptions & DRAW_OPT_INVERT)
  {  
    //Drawing with white pixels:
    switch (DrawingOptions & DRAW_OPT_LOGICAL_OPERATIONS)
    {
      //Only these cases do anything:
      case DRAW_OPT_LOGICAL_COPY:
      case DRAW_OPT_LOGICAL_AND:
        *pPixelMode = DRAW_PIXELS_CLEAR;
        return TRUE;
      default:
        break;
    }
  }  
  else
  {  
    //Drawing with black pixels:
    switch (DrawingOptions & DRAW_OPT_LOGICAL_OPERATIONS)
    {
      //Only these cases do anything:
      case DRAW_OPT_LOGICAL_COPY:
      case DRAW_OPT_LOGICAL_OR:
        *pPixelMode = DRAW_PIXELS_SET;
        return TRUE;
      case DRAW_OPT_LOGICAL_XOR:
        *pPixelMode = DRAW_PIXELS_INVERT;
        return TRUE;
      default:
        break;
    }
  }

  // If no operation is required, set defaults and return FALSE.
  // e.g. 'AND' on its own is meaningless for line drawing,
  // 'INVERT + OR' and 'INVERT + XOR'  do nothing either.
  
  *pPixelMode = DRAW_PIXELS_SET;
  *pFillMode = DRAW_SHAPE_HOLLOW;
  return FALSE;
}

//------------------------------------------------------------------
// cCmdResolveBitmapMode - extract the parameters that affect bitmap and text
//                         drawing from the 'DrawingOptions' or 'CopyOptions' parameter.
void cCmdResolveBitmapMode(UWORD DrawingOptions, UBYTE* pInvertMode, UBYTE* pLogicalMode, UBYTE* pFillMode)
{
  DrawingOptions = cCmdResolveValue(DrawingOptions);
  //Extract bitmap inversion mode:
  if (DrawingOptions & DRAW_OPT_INVERT)
    *pInvertMode = DRAW_BITMAP_INVERT;
  else
    *pInvertMode = DRAW_BITMAP_PLAIN;

  //Extract bitmap logical operation:
  switch (DrawingOptions & DRAW_OPT_LOGICAL_OPERATIONS)
  {
    case DRAW_OPT_LOGICAL_OR:
      *pLogicalMode = DRAW_LOGICAL_OR;
      break;
    case DRAW_OPT_LOGICAL_AND:
      *pLogicalMode = DRAW_LOGICAL_AND;
      break;
    case DRAW_OPT_LOGICAL_XOR:
      *pLogicalMode = DRAW_LOGICAL_XOR;
      break;
    case DRAW_OPT_LOGICAL_COPY:
    default:
      *pLogicalMode = DRAW_LOGICAL_COPY;
      break;
  }
  
  // Extract shape fill option (overrides source bitmap with all pixels set):
  if (DrawingOptions & DRAW_OPT_FILL_SHAPE)
    *pFillMode = DRAW_SHAPE_FILLED;
  else
    *pFillMode = DRAW_SHAPE_HOLLOW;
  
}

//------------------------------------------------------------------
// cCmdResolveFontOptions - extract the parameters that affect custom RIC font drawing
//                         drawing from the 'DrawingOptions' parameter.
void cCmdResolveFontOptions(UWORD DrawingOptions, UBYTE* pFontDir, UBYTE* pFontWrap)
{
  //Extract bitmap inversion mode:
  if (DrawingOptions & DRAW_OPT_FONT_WRAP)
    *pFontWrap = DRAW_FONT_WRAP_ON;
  else
    *pFontWrap = DRAW_FONT_WRAP_OFF;

  //Extract bitmap logical operation:
  switch (DrawingOptions & DRAW_OPT_FONT_DIRECTIONS)
  {
    case DRAW_OPT_FONT_DIR_L2RB:
      *pFontDir = DRAW_FONT_DIR_L2RB;
      break;
    case DRAW_OPT_FONT_DIR_L2RT:
      *pFontDir = DRAW_FONT_DIR_L2RT;
      break;
    case DRAW_OPT_FONT_DIR_R2LB:
      *pFontDir = DRAW_FONT_DIR_R2LB;
      break;
    case DRAW_OPT_FONT_DIR_R2LT:
      *pFontDir = DRAW_FONT_DIR_R2LT;
      break;
    case DRAW_OPT_FONT_DIR_B2TL:
      *pFontDir = DRAW_FONT_DIR_B2TL;
      break;
    case DRAW_OPT_FONT_DIR_B2TR:
      *pFontDir = DRAW_FONT_DIR_B2TR;
      break;
    case DRAW_OPT_FONT_DIR_T2BL:
      *pFontDir = DRAW_FONT_DIR_T2BL;
      break;
    case DRAW_OPT_FONT_DIR_T2BR:
      *pFontDir = DRAW_FONT_DIR_T2BR;
      break;
    default:
      *pFontDir = DRAW_FONT_DIR_L2RB;
      break;
  }
}

//------------------------------------------------------------------
// cCmdResolveClearScreenMode - extract the parameters that affect screen clearing
//                              from the 'DrawingOptions' parameter.
UBYTE cCmdResolveClearScreenMode(UWORD DrawingOptions)
{
  if (DrawingOptions & DRAW_OPT_CLEAR_SCREEN_MODES)
    return TRUE;
  else
    return FALSE;
}
//JJR

void cCmdDrawFont(IMG_OP_SPRITE *pSprite, SLONG sW, SLONG sH, SLONG startx, SLONG starty,
  UBYTE fontDir, UBYTE fontWrap, UBYTE invertMode, UBYTE logicalMode, UBYTE fillMode)
{
  SWORD  index;                           // index on id for output
  SWORD  xpos;                            // x-position for next output
  SWORD  ypos;                            // y-position for next output
  SLONG  sourcex;                         // X-Pos of pattern inside bitmap
  SLONG  sourcey;                         // Y-Pos of pattern inside bitmap
  SLONG  sourcew;                         // Width of pattern inside bitmap
  SLONG  sourceh;                         // Height of pattern inside bitmap
  int    destx;                           // X-Destination position on screen
  int    desty;                           // Y-Destination position on screen
  SWORD  stdw;                            // Standard font width
  SWORD  stdh;                            // Standard font height
  
  if (!gpPassedImgVars) // exit early if we do not have parameters
    return;
    
  stdw = MIN( sW, DISPLAY_WIDTH );        // calculate the standard width
  stdh = MIN( sH, DISPLAY_HEIGHT );       // calculate the standard height
  
  index=0;                                // Reset the start position
  xpos=startx;                            // Initial start position X
  ypos=starty;                            // Initial start position Y
  
  while( index < gPassedVarsCount )
  {
    // terminate the loop at our first null
    if (gpPassedImgVars[index] == 0) 
      break;
    sourcex=cCmdResolveValue(0xF200|index);   // Get dynamic xpos of pattern
    sourcey=cCmdResolveValue(0xF300|index);   // Get dynamic ypos of pattern
    sourcew=cCmdResolveValue(0xF400|index);   // Get dynamic width of pattern
    sourceh=cCmdResolveValue(0xF500|index);   // Get dynamic height of pattern
  
    //--Calculate the rectangle for source (inside the sprite) and destination 
    //--on the screen (related to the fontdirection) for performing the 
    //--BitCopy function
    
    switch( fontDir )
    {
       case DRAW_FONT_DIR_L2RB : // Left to right with bottom align----------------
       
                            if( fontWrap )
                            {
                               if( ( xpos + sourcew ) > DISPLAY_WIDTH )
                               {
                                   xpos = startx;
                                   ypos -= stdh;
                               }    
                            }
                            
                            destx = xpos;               
                            desty = ypos;
                            xpos  = xpos + sourcew;
                                                          
                            break;
                            
       case DRAW_FONT_DIR_L2RT : // Left to right with align to top----------------
  
                            if( fontWrap )
                            {
                                if( ( xpos + sourcew ) > DISPLAY_WIDTH )
                                {
                                    xpos = startx;
                                    ypos += stdh;
                                }
                            }     
  
                            destx = xpos;                
                            desty = ypos - sourceh + 1;
                            xpos  = xpos + sourcew;
                            break;
  
       case DRAW_FONT_DIR_R2LB : // Right to left with align to bottom-------------
       
                            if( fontWrap )
                            {
                                if( ( xpos - sourcew ) < 0 )
                                {
                                    xpos = startx;
                                    ypos -= stdh;
                                }
                            }
                                    
                            destx = xpos - sourcew + 1;   
                            desty = ypos;
                            xpos  = xpos - sourcew;
                            break;
                            
       case DRAW_FONT_DIR_R2LT : // Right to left with align to top----------------
       
                            if( fontWrap )
                            {
                                if( ( xpos - sourcew ) < 0 )
                                {
                                    xpos = startx;
                                    ypos += stdh;
                                }
                            }
                            
                            destx = xpos - sourcew + 1;   
                            desty = ypos - sourceh + 1;     
                            xpos  = xpos - sourcew;
                            break;
                            
       case DRAW_FONT_DIR_B2TL : // Bottom to top with allign to left-------------- 
       
                            if( fontWrap )
                            {
                                if( ( ypos + sourceh ) > DISPLAY_HEIGHT )
                                {
                                    xpos += stdw;
                                    ypos = starty;
                                }
                            }
                            
                            destx = xpos;
                            desty = ypos;
                            ypos  = ypos + sourceh;
                            break;
                            
       case DRAW_FONT_DIR_B2TR : // Bottom to top with allign to right-------------
       
                            if( fontWrap )
                            {
                                if( ( ypos + sourceh ) > DISPLAY_HEIGHT )
                                {
                                    xpos -= stdw;
                                    ypos = starty;
                                }
                            }
                            
                            destx = xpos - sourcew + 1;     
                            desty = ypos;
                            ypos  = ypos + sourceh;
                            break;
                            
       case DRAW_FONT_DIR_T2BL : // Top to bottom with allign to right------------- 
       
                            if( fontWrap )
                            {
                                if( ( ypos - sourceh ) < 0 )
                                {
                                    xpos += stdw;
                                    ypos = starty;
                                }
                            }
                            
                            destx = xpos;   
                            desty = ypos - sourceh + 1;
                            ypos  = ypos - sourceh;
                            break;
                            
       case DRAW_FONT_DIR_T2BR : // Top to bottom with allign to right------------- 
       
                            if( fontWrap )
                            {
                                if( ( ypos - sourceh ) < 0 )
                                {
                                    xpos -= stdw;
                                    ypos = starty;
                                }
                            }
                            
                            destx = xpos - sourcew + 1;
                            desty = ypos - sourceh + 1;  
                            ypos  = ypos - sourceh;
                            break;
    }
  
    //--Output of the calculated pattern to the screen------------------------
    cCmdCopyBitMapBits(destx, desty, sourcex, sourcey, sourcew, sourceh,
                       pSprite, invertMode, logicalMode, fillMode);
  
    // move on to next character
    index++;
  }   
}

