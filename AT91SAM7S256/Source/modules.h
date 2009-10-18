//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkflebun                                        $
//
// Revision date   $Date:: 5-02-07 13:36                                     $
//
// Filename        $Workfile:: modules.h                                     $
//
// Version         $Revision:: 49                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/modules.h  $
//
// Platform        C
//

#ifndef   MODULE_HEADER
#define   MODULE_HEADER

#define   FILENAME_LENGTH         19    // zero termination not included
#define   FILEHEADER_LENGTH       8     // all simple file headers
#define   DISPLAYLINE_LENGTH      16    // zero termination not included
#define   ON_BRICK_PROGRAMSTEPS   5     // no of on brick program steps
#define   STATUSTEXT_SIZE         8     // zero termination not included

#define   TXT_SOUND_EXT           "rso" // Sound filename extension
#define   TXT_LMS_EXT             "rxe" // Mindstorms program filename extension
#define   TXT_NXT_EXT             "rpg" // Program filename extension
#define   TXT_TRYME_EXT           "rtm" // Try me program filename extension
#define   TXT_DATA_EXT            "rdt" // Datalog filename extension
#define   TXT_MENU_EXT            "rms" // Menu system filename extension (hidden)
#define   TXT_SYS_EXT             "sys" // System filename extension      (hidden)
#define   TXT_TMP_EXT             "tmp" // Temporary filename extension   (hidden)


/* Error codes from then Loader */
enum
{
  SUCCESS             = 0x0000,
  INPROGRESS          = 0x0001,
  REQPIN              = 0x0002,
  NOMOREHANDLES       = 0x8100,
  NOSPACE             = 0x8200,
  NOMOREFILES         = 0x8300,
  EOFEXSPECTED        = 0x8400,
  ENDOFFILE           = 0x8500,
  NOTLINEARFILE       = 0x8600,
  FILENOTFOUND        = 0x8700,
  HANDLEALREADYCLOSED = 0x8800,
  NOLINEARSPACE       = 0x8900,
  UNDEFINEDERROR      = 0x8A00,
  FILEISBUSY          = 0x8B00,
  NOWRITEBUFFERS      = 0x8C00,
  APPENDNOTPOSSIBLE   = 0x8D00,
  FILEISFULL          = 0x8E00,
  FILEEXISTS          = 0x8F00,
  MODULENOTFOUND      = 0x9000,
  OUTOFBOUNDERY       = 0x9100,
  ILLEGALFILENAME     = 0x9200,
  ILLEGALHANDLE       = 0x9300,
  BTBUSY              = 0x9400,
  BTCONNECTFAIL       = 0x9500,
  BTTIMEOUT           = 0x9600,
  FILETX_TIMEOUT      = 0x9700,
  FILETX_DSTEXISTS    = 0x9800,
  FILETX_SRCMISSING   = 0x9900,
  FILETX_STREAMERROR  = 0x9A00,
  FILETX_CLOSEERROR   = 0x9B00
};


/* interface between comm and BC4           */
enum
{
  MSG_BEGIN_INQUIRY,
  MSG_CANCEL_INQUIRY,
  MSG_CONNECT,
  MSG_OPEN_PORT,
  MSG_LOOKUP_NAME,
  MSG_ADD_DEVICE,
  MSG_REMOVE_DEVICE,
  MSG_DUMP_LIST,
  MSG_CLOSE_CONNECTION,
  MSG_ACCEPT_CONNECTION,
  MSG_PIN_CODE,
  MSG_OPEN_STREAM,
  MSG_START_HEART,
  MSG_HEARTBEAT,
  MSG_INQUIRY_RUNNING,
  MSG_INQUIRY_RESULT,
  MSG_INQUIRY_STOPPED,
  MSG_LOOKUP_NAME_RESULT,
  MSG_LOOKUP_NAME_FAILURE,
  MSG_CONNECT_RESULT,
  MSG_RESET_INDICATION,
  MSG_REQUEST_PIN_CODE,
  MSG_REQUEST_CONNECTION,
  MSG_LIST_RESULT,
  MSG_LIST_ITEM,
  MSG_LIST_DUMP_STOPPED,
  MSG_CLOSE_CONNECTION_RESULT,
  MSG_PORT_OPEN_RESULT,
  MSG_SET_DISCOVERABLE,
  MSG_CLOSE_PORT,
  MSG_CLOSE_PORT_RESULT,
  MSG_PIN_CODE_ACK,
  MSG_DISCOVERABLE_ACK,
  MSG_SET_FRIENDLY_NAME,
  MSG_SET_FRIENDLY_NAME_ACK,
  MSG_GET_LINK_QUALITY,
  MSG_LINK_QUALITY_RESULT,
  MSG_SET_FACTORY_SETTINGS,
  MSG_SET_FACTORY_SETTINGS_ACK,
  MSG_GET_LOCAL_ADDR,
  MSG_GET_LOCAL_ADDR_RESULT,
  MSG_GET_FRIENDLY_NAME,
  MSG_GET_DISCOVERABLE,
  MSG_GET_PORT_OPEN,
  MSG_GET_FRIENDLY_NAME_RESULT,
  MSG_GET_DISCOVERABLE_RESULT,
  MSG_GET_PORT_OPEN_RESULT,
  MSG_GET_VERSION,
  MSG_GET_VERSION_RESULT,
  MSG_GET_BRICK_STATUSBYTE_RESULT,
  MSG_SET_BRICK_STATUSBYTE_RESULT,
  MSG_GET_BRICK_STATUSBYTE,
  MSG_SET_BRICK_STATUSBYTE
};

#define   SIZE_OF_BT_NAME               16
#define   SIZE_OF_BRICK_NAME            8
#define   SIZE_OF_CLASS_OF_DEVICE       4
#define   SIZE_OF_BT_PINCODE            16
#define   SIZE_OF_BDADDR                7


enum
{
  ENTRY_COMM,
  ENTRY_INPUT,
  ENTRY_BUTTON,
  ENTRY_DISPLAY,
  ENTRY_LOADER,
  ENTRY_LOWSPEED,
  ENTRY_OUTPUT,
  ENTRY_SOUND,
  ENTRY_IOCTRL,
  ENTRY_CMD,
  ENTRY_UI,
  ENTRY_FREE2,
  ENTRY_FREE3,
  ENTRY_FREE4,
  ENTRY_FREE5
};

typedef   struct
{
  ULONG   ModuleID;
  UBYTE   ModuleName[FILENAME_LENGTH + 1];
  void    (*cInit)(void* pHeader);
  void    (*cCtrl)(void);
  void    (*cExit)(void);
  void    *pIOMap;
  void    *pVars;
  UWORD   IOMapSize;
  UWORD   VarsSize;
  UWORD   ModuleSize;
}HEADER;

enum
{
  FILEFORMAT_SOUND            = 0x0100,   // rso
  FILEFORMAT_SOUND_COMPRESSED = 0x0101,   // rso
  FILEFORMAT_BITMAP           = 0x0200,
  FILEFORMAT_FONT             = 0x0300,
  FILEFORMAT_ICON             = 0x0400,
  FILEFORMAT_TEXT             = 0x0500,
  FILEFORMAT_MELODY           = 0x0600,
  FILEFORMAT_MENU             = 0x0700,   // rms
  FILEFORMAT_PROGRAM          = 0x0800,   // rpg
  FILEFORMAT_DATALOG          = 0x0900    // rdt
};

typedef   struct
{
  UBYTE   FormatMsb;
  UBYTE   FormatLsb;
  UBYTE   DateBytesMsb;
  UBYTE   DataBytesLsb;
  UBYTE   SampleRateMsb;
  UBYTE   SampleRateLsb;
  UBYTE   PlayModeMsb;
  UBYTE   PlayModeLsb;
  UBYTE   Data[];
}
SOUND;

typedef   struct
{
  UBYTE   FormatMsb;
  UBYTE   FormatLsb;
  UBYTE   DateBytesMsb;
  UBYTE   DataBytesLsb;
  UBYTE   StartX;
  UBYTE   StartY;
  UBYTE   PixelsX;
  UBYTE   PixelsY;
  UBYTE   Data[];
}
BMPMAP;

typedef   struct
{
  UBYTE   FormatMsb;
  UBYTE   FormatLsb;
  UBYTE   DataBytesMsb;
  UBYTE   DataBytesLsb;
  UBYTE   ItemsX;
  UBYTE   ItemsY;
  UBYTE   ItemPixelsX;
  UBYTE   ItemPixelsY;
  UBYTE   Data[];
}
FONT;

typedef   struct
{
  UBYTE   FormatMsb;
  UBYTE   FormatLsb;
  UBYTE   DataBytesMsb;
  UBYTE   DataBytesLsb;
  UBYTE   ItemsX;
  UBYTE   ItemsY;
  UBYTE   ItemPixelsX;
  UBYTE   ItemPixelsY;
  UBYTE   Data[];
}
ICON;

typedef   struct
{
  UBYTE   FormatMsb;
  UBYTE   FormatLsb;
  UBYTE   DataBytesMsb;
  UBYTE   DataBytesLsb;
  UBYTE   ItemsX;
  UBYTE   ItemsY;
  UBYTE   ItemCharsX;
  UBYTE   ItemCharsY;
  UBYTE   Data[];
}
TXT;

typedef   struct
{
  UBYTE   FormatMsb;
  UBYTE   FormatLsb;
  UBYTE   DateBytesMsb;
  UBYTE   DataBytesLsb;
  UBYTE   TonesMsb;
  UBYTE   TonesLsb;
  UBYTE   PlayModeMsb;
  UBYTE   PlayModeLsb;
  UBYTE   Data[];                       // Data[0] = FreqMsb, Data[1] = FreqLsb, Data[2] = DurationMsb, Data[3] = DurationLsb ....
}
MELODY;

typedef   struct
{
  UBYTE   FormatMsb;
  UBYTE   FormatLsb;
  UBYTE   DataBytesMsb;
  UBYTE   DataBytesLsb;
  UBYTE   Steps;
  UBYTE   NotUsed1;
  UBYTE   NotUsed2;
  UBYTE   NotUsed3;
  UBYTE   Data[];
}
PROGRAM;

typedef   struct
{
  UBYTE   FormatMsb;
  UBYTE   FormatLsb;
  UBYTE   DataBytesMsb;
  UBYTE   DataBytesLsb;
  UBYTE   TotalTime3;
  UBYTE   TotalTime2;
  UBYTE   TotalTime1;
  UBYTE   TotalTime0;
  UBYTE   Data[];
}
DATALOG;

#define   DATALOG_FILE_LENGTH     64000L// Max datalog file size
#define   DATALOG_HEADER_LENGTH     9   // Datalog sensor header length       [Bytes]
#define   DATALOG_DATA_LENGTH       5   // Datalog sensor data length         [Bytes]

#define   ICON_TEXTLNG             15   // 15 characters
#define   ICON_IMAGESIZE           72   // 24 x 24 pixels
#define   MAX_MENUITEMS            256

typedef   struct
{
  UBYTE   ItemId67;                     // Menu item id
  UBYTE   ItemId45;                     // Menu item id
  UBYTE   ItemId23;                     // Menu item id
  UBYTE   ItemId01;                     // Menu item id
  UBYTE   SpecialMask3;                 // Menu item special mask (TBD)
  UBYTE   SpecialMask2;                 // Menu item special mask (TBD)
  UBYTE   SpecialMask1;                 // Menu item special mask (TBD)
  UBYTE   SpecialMask0;                 // Menu item special mask (TBD)
  UBYTE   FunctionIndex;                // Menu item enter function call index
  UBYTE   FunctionParameter;            // Menu item enter function parameter
  UBYTE   FileLoadNo;                   // Menu item enter menu file load no
  UBYTE   NextMenu;                     // Menu item enter next level menu no
  UBYTE   IconText[ICON_TEXTLNG + 1];   // Menu item icon text string
  UBYTE   IconImageNo;                  // Menu item icon image number
}MENUITEM;

typedef   struct
{
  UBYTE   FormatMsb;
  UBYTE   FormatLsb;
  UBYTE   DataBytesMsb;
  UBYTE   DataBytesLsb;
  UBYTE   ItemSize;
  UBYTE   Items;
  UBYTE   ItemPixelsX;
  UBYTE   ItemPixelsY;
  MENUITEM Data[MAX_MENUITEMS];
}
MENU;

typedef   UBYTE   (*FUNCTION)(UBYTE);   // Menu function type

#endif



