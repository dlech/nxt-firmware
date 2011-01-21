//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 10/21/08 12:08p                                   $
//
// Filename        $Workfile:: c_ui.h                                        $
//
// Version         $Revision:: 10                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_ui.h $
//
// Platform        C
//

#ifndef   C_UI
#define   C_UI

#ifndef STRIPPED
#define   DATALOGENABLED                1           // 1 == Datalog enable
#else
#define   DATALOGENABLED                0           // 0 == Datalog disabled
#endif

#define   NO_OF_FEEDBACK_CHARS          12          // Chars left when bitmap also showed
#define   SIZE_OF_CURSOR                16          // Bitmap size of cursor  (header + 8x8 pixels)
#define   SIZE_OF_PORTBITMAP            11          // Bitmap size of port no (header + 3x8 pixels)
#define   NO_OF_STATUSICONS             4           // Status icons

#define   NO_OF_INTROBITMAPS            16          // Intro bitmaps
#define   INTRO_START_TIME              1000        // Intro startup time                     [mS]
#define   INTRO_SHIFT_TIME              100         // Intro inter bitmap time                [mS] 
#define   INTRO_STOP_TIME               1000        // Intro stop time                        [mS]
#define   INTRO_LOWBATT_TIME            2000        // Low battery show time at power up      [mS]

#define   MAX_VOLUME                    4           // Max volume in UI                       [cnt]

#define   CHECKBYTE                     0x78        // Used to validate NVData

#define   BATTERY_COUNT_TO_MV    (float)13.848      // Battery count to mV factor             [mV/cnt]
#define   LOW_BATT_THRESHOLD            6           // Low batt conunts before warning

#define   BUTTON_DELAY_TIME             800         // Delay before first repeat              [mS]
#define   BUTTON_REPEAT_TIME            200         // Repeat time                            [mS]

#define   RUN_BITMAP_CHANGE_TIME        125         // Running bimap update time              [mS]
#define   RUN_STATUS_CHANGE_TIME        167         // Running status update time             [mS]

#define   DISPLAY_SHOW_ERROR_TIME       2500        // Error string show time                 [mS] 
#define   DISPLAY_SHOW_TIME             1500        // Min. response display time             [mS]
#define   DISPLAY_VIEW_UPDATE           200         // Display update time                    [mS]
#define   MIN_DISPLAY_UPDATE_TIME       50          // OBP min graphics update time           [mS]
#define   MIN_SENSOR_READ_TIME          100         // Time between sensor reads              [mS]

#define   ARM_WAIT_FOR_POWER_OFF        250         // Time for off command to execute        [mS]

#define   DISPLAY_SHOW_FILENAME_TIME    3000        // Datalog show saves as time             [mS]
#define   DATALOG_DEFAULT_SAMPLE_TIME   100L        // Default time between samples           [mS]

// Menu special flags

#define   MENU_SKIP_THIS_MOTHER_ID      0x00000001L // Used to seek next common menu (i0000000)
                                                    // Free
#define   MENU_ENTER_ACT_AS_EXIT        0x00000004L // Enter button acts as exit button
#define   MENU_BACK_TWICE               0x00000008L // Exit twice on exit button
#define   MENU_EXIT_ACT_AS_ENTER        0x00000010L // Exit button acts as enter button
#define   MENU_LEAVE_BACKGROUND         0x00000020L // Don't erase background at next menu
#define   MENU_EXIT_CALLS_WITH_FF       0x00000040L // Exit button calls function with MENU_EXIT
#define   MENU_EXIT_LEAVES_MENUFILE     0x00000080L // Exit leaves menu file
#define   MENU_INIT_CALLS_WITH_0        0x00000100L // Menu init calls with MENU_INIT
#define   MENU_LEFT_RIGHT_AS_CALL       0x00000200L // Left calls with MENU_LEFT and right with MENU_RIGHT
#define   MENU_ENTER_ONLY_CALLS         0x00000400L // Enter calls only it does not change menues
#define   MENU_EXIT_ONLY_CALLS          0x00000800L // Exit calls only it does not change menues
#define   MENU_AUTO_PRESS_ENTER         0x00001000L // Enter button is pressed automaticly
#define   MENU_ENTER_LEAVES_MENUFILE    0x00002000L // Enter leaves menufile
#define   MENU_INIT_CALLS               0x00004000L // Init calls instead of enter
#define   MENU_ACCEPT_INCOMMING_REQUEST 0x00008000L // Accept incomming BT connection request
#define   MENU_BACK_THREE_TIMES         0x00010000L // Exit three times on exit button
#define   MENU_EXIT_DISABLE             0x00020000L // Disable exit button
#define   MENU_EXIT_LOAD_POINTER        0x00040000L // Load item index on exit (0i000000)
#define   MENU_EXIT_CALLS               0x00080000L // Exit calls as enter
#define   MENU_INIT_CALLS_WITH_1        0x00100000L // Menu init calls with MENU_INIT
#define   MENU_EXIT_LOAD_MENU           0x00200000L // Exit loads next menu
#define   MENU_ONLY_BT_ON               0x00400000L // Only valid when bluecore is on
#define   MENU_ONLY_DATALOG_ENABLED     0x00800000L // Only valid when datalog is enabled

// Menu function call parameter

#define   MENU_SENSOR_EMPTY             0x01        // Empty
#define   MENU_SENSOR_SOUND_DB          0x02        // Sound sensor dB
#define   MENU_SENSOR_SOUND_DBA         0x03        // Sound sensor dBA
#define   MENU_SENSOR_LIGHT             0x04        // Light sensor with flood light
#define   MENU_SENSOR_LIGHT_AMB         0x05        // Light sensor without flood light
#define   MENU_SENSOR_TOUCH             0x06        // Touch sensor
#define   MENU_SENSOR_MOTOR_DEG         0x07        // Motor sensor degrees
#define   MENU_SENSOR_MOTOR_ROT         0x08        // Motor sensor rotations
#define   MENU_SENSOR_ULTRASONIC_IN     0x09        // Ultrasonic sensor inch
#define   MENU_SENSOR_ULTRASONIC_CM     0x0A        // Ultrasonic sensor cm
#define   MENU_SENSOR_IIC_TEMP_C        0x0B        // IIC temp sensor celcius
#define   MENU_SENSOR_IIC_TEMP_F        0x0C        // IIC temp sensor fahrenheit
#define   MENU_SENSOR_COLOR             0x0D        // Color sensor
#define   MENU_SENSOR_INVALID           0x0E        // Invalid

#define   MENU_PORT_EMPTY               0x11        // Port empty
#define   MENU_PORT_1                   0x12        // Port 1
#define   MENU_PORT_2                   0x13        // Port 2
#define   MENU_PORT_3                   0x14        // Port 3
#define   MENU_PORT_4                   0x15        // Port 4
#define   MENU_PORT_A                   0x16        // Port A
#define   MENU_PORT_B                   0x17        // Port B
#define   MENU_PORT_C                   0x18        // Port C
#define   MENU_PORT_INVALID             0x19        // Invalid

#define   MENU_ACTION_EMPTY             0x21        // Empty
#define   MENU_ACTION_FORWARD_1         0x22        // Forward until
#define   MENU_ACTION_FORWARD_2         0x23        // Forward 5
#define   MENU_ACTION_BACK_LEFT_2       0x24        // Back left 2
#define   MENU_ACTION_TURN_LEFT_1       0x25        // Turn left until
#define   MENU_ACTION_TURN_LEFT_2       0x26        // Turn left 2
#define   MENU_ACTION_BACK_RIGHT_1      0x27        // Back right until
#define   MENU_ACTION_TURN_RIGHT_1      0x28        // Turn right until
#define   MENU_ACTION_TURN_RIGHT_2      0x29        // Turn right 2
#define   MENU_ACTION_BACK_LEFT_1       0x2A        // Back left until
#define   MENU_ACTION_TONE_1            0x2B        // Tone 1
#define   MENU_ACTION_TONE_2            0x2C        // Tone 2
#define   MENU_ACTION_BACKWARD_1        0x2D        // Backward until
#define   MENU_ACTION_BACKWARD_2        0x2E        // Backward 5
#define   MENU_ACTION_BACK_RIGHT_2      0x2F        // Back right 2
#define   MENU_ACTION_INVALID           0x30        // Invalid

#define   MENU_WAIT_EMPTY               0x41        // Empty
#define   MENU_WAIT_LIGHT               0x42        // Light
#define   MENU_WAIT_SEEK_OBJ            0x43        // Seek obj.
#define   MENU_WAIT_SOUND               0x44        // Sound
#define   MENU_WAIT_TOUCH               0x45        // Touch
#define   MENU_WAIT_1                   0x46        // Wait 2
#define   MENU_WAIT_2                   0x47        // Wait 5
#define   MENU_WAIT_3                   0x48        // Wait 10
#define   MENU_WAIT_DARK                0x49        // Dark
#define   MENU_WAIT_INVALID             0x4A        // Invalid

#define   MENU_INIT                     0x00        // Init
#define   MENU_INIT_ALTERNATIVE         0x01        // Init alternative
#define   MENU_DRAW                     0xE9        // Draw
#define   MENU_OFF                      0xEA        // Off
#define   MENU_ON                       0xEB        // On
#define   MENU_OPEN_STREAM              0xEC        // Open stream
#define   MENU_OVERWRITE                0xED        // Overwrite file
#define   MENU_CALCULATE                0xEE        // Calculate
#define   MENU_ENTER                    0xEF        // Enter
#define   MENU_DISCONNECT               0xF0        // Disconnect BT  
#define   MENU_DELETE                   0xF1        // Delete  
#define   MENU_SELECT                   0xF2        // Select
#define   MENU_RUN_SILENT               0xF3        // Run without graphics
#define   MENU_TOGGLE                   0xF4        // Toggle
#define   MENU_CONNECT                  0xF5        // Connect BT
#define   MENU_UPDATE                   0xF6        // Update
#define   MENU_TEXT                     0xF7        // Text
#define   MENU_RUN                      0xF8        // Run
#define   MENU_SEND                     0xF9        // Send
#define   MENU_SAVE                     0xFA        // Save
#define   MENU_STOP                     0xFB        // Stop
#define   MENU_LOOP                     0xFC        // Loop
#define   MENU_LEFT                     0xFD        // Left
#define   MENU_RIGHT                    0xFE        // Right
#define   MENU_EXIT                     0xFF        // Exit

#define   DATALOGPORTS                  (MENU_PORT_INVALID - MENU_PORT_EMPTY - 1)
#define   MAX_DATALOGS                  9999        // Highest datalog file number
#define   DATALOGBUFFERSIZE             25          // Largest number of characters buffered before flash write

#define   MENULEVELS                    10          // Max no of levels in one file (8 + 2 virtual)
#define   MENUFILELEVELS                3           // Max deept in menu file pool

typedef   struct                                    // VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevels[VarsUi.MenuLevel].
{                                                   
  ULONG   Id;                                       // Menu item id
  UBYTE   *IconText;                                // Menu item icon text  pointer
  ULONG   SpecialFlags;                             // Menu item special behaivor
  UBYTE   IconImageNo;                              // Menu item icon image no
  UBYTE   FunctionNo;                               // Menu item function call no   (0 = none)
  UBYTE   Parameter;                                // Menu item function call parameter
  UBYTE   NextFileNo;                               // Menu item next menu file no  (0 = none)
  UBYTE   NextMenuNo;                               // Menu item next menu no       (0 = none)

  UBYTE   ItemIndex;                                // Menu item index on level
  UBYTE   Items;                                    // Menu items on level
}
MENULEVEL;

typedef   struct
{
  MENULEVEL MenuLevels[MENULEVELS];                 // See above
  UBYTE   FileId;                                   // VarsUi.MenuFiles[VarsUi.MenuFileLevel].FileId
  UBYTE   MenuLevel;                                // VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel
}
MENUFILE;

typedef   struct
{
  UBYTE   CheckByte;                                // Check byte (CHECKBYTE)
  UBYTE   DatalogEnabled;                           // Datalog enabled flag (0 = no)
  UBYTE   VolumeStep;                               // Volume step (0 - MAX_VOLUME)
  UBYTE   PowerdownCode;                            // Power down code
  UWORD   DatalogNumber;                            // Datalog file number (0 - MAX_DATALOGS)
}
NVDATA;

typedef   struct
{
  UBYTE   StatusText[STATUSTEXT_SIZE + 1];          // RCX name
  UBYTE   Initialized;                              // Ui init done
  UWORD   SleepTimer;                               // Sleep timer

  // Menu system
  MENUFILE  MenuFiles[MENUFILELEVELS];              // Menu file array
  MENUFILE  *pMenuFile;                             // Actual menu file pointer
  MENULEVEL *pMenuLevel;                            // Actual menu item on level, pointer
  MENUITEM  *pMenuItem;                             // Actual menu item in menu flash file
  UBYTE     MenuFileLevel;                          // Actual menu file level
  UBYTE   Function;                                 // Running function (0 = none)
  UBYTE   Parameter;                                // Parameter for running function
  UBYTE   SecondTime;                               // Second time flag
  UBYTE   EnterOnlyCalls;                           // Enter button only calls
  UBYTE   ExitOnlyCalls;                            // Exit button only calls
  UWORD   ButtonTimer;                              // Button repeat timer
  UWORD   ButtonTime;                               // Button repeat time
  UBYTE   ButtonOld;                                // Button old state

  // Update status
  UWORD   UpdateCounter;                            // Update counter
  UBYTE   Running;                                  // Running pointer
  UBYTE   BatteryToggle;                            // Battery flash toggle flag
  UBYTE   NewStatusIcons[NO_OF_STATUSICONS];        // New status icons (used to detect changes)

  // Low battery voltage
  UBYTE   *LowBattSavedBitmap;                      // Low battery overwritten bitmap placeholder
  UBYTE   LowBatt;                                  // Low battery volatge flag
  UBYTE   LowBattHasOccured;                        // Low battery voltage has occured
  UBYTE   LowBattSavedState;                        // Low battery current state placeholder

  // General used variables
  UBYTE   *MenuIconTextSave;                        // Menu icon text save

  UBYTE   *pTmp;                                    // General UBYTE pointer
  ULONG   TmpLength;                                // General filelength  (used in filelist)
  SWORD   TmpHandle;                                // General filehandle  (used in filelist)

  SWORD   Timer;                                    // General tmp purpose timer
  SWORD   ReadoutTimer;                             // General read out timer
  UBYTE   Tmp;                                      // General UBYTE
  UBYTE   FileType;                                 // General file type
  UBYTE   State;                                    // General tmp purpose state
  UBYTE   Pointer;                                  // General tmp purpose pointer
  UBYTE   Counter;                                  // General tmp purpose counter
  UBYTE   Cursor;                                   // General cursor
  UBYTE   SelectedSensor;                           // General used for selected sensor
  UBYTE   SelectedPort;                             // General used for selected port
  UBYTE   SensorReset;
  UBYTE   SensorState;                              // Sensor state (reset, ask, read)
  SWORD   SensorTimer;                              // Timer used to time sensor states
  UBYTE   NextState;

  UBYTE   SelectedFilename[FILENAME_LENGTH + 1];    // Selected file name
  UBYTE   FilenameBuffer[FILENAME_LENGTH + 1];      // General filename buffer
  UBYTE   SearchFilenameBuffer[FILENAME_LENGTH + 1];// General filename buffer
  UBYTE   DisplayBuffer[DISPLAYLINE_LENGTH + 1];    // General purpose display buffer

  UBYTE   PortBitmapLeft[SIZE_OF_PORTBITMAP];       // Port no bitmap for left icon
  UBYTE   PortBitmapCenter[SIZE_OF_PORTBITMAP];     // Port no bitmap for center icon
  UBYTE   PortBitmapRight[SIZE_OF_PORTBITMAP];      // Port no bitmap for right icon

  // Find no of files and find name for file no
  ULONG   FNOFLength;                               // Length
  SWORD   FNOFHandle;                               // Handle
  UBYTE   FNOFState;                                // State
  UBYTE   FNOFSearchBuffer[FILENAME_LENGTH + 1];    // Search buffer
  UBYTE   FNOFNameBuffer[FILENAME_LENGTH + 1];      // File name buffer
  UBYTE   FNOFFileNo;                               // File no

  // File list
  UBYTE   FileCenter;                               // File center
  UBYTE   FileLeft;                                 // File left
  UBYTE   FileRight;                                // File right
  UBYTE   NoOfFiles;                                // No of files

#ifndef STRIPPED
  // On brick programming menu
  UBYTE   ProgramSteps[ON_BRICK_PROGRAMSTEPS];      // On brick programming steps
  UBYTE   ProgramStepPointer;                       // On brick programming step pointer
  UBYTE   CursorTmp[SIZE_OF_CURSOR];                // On brick programming cursor
  UBYTE   FileHeader[FILEHEADER_LENGTH];            // File header for programs
  UBYTE   *FeedBackText;                            // Program end text
  UWORD   OBPTimer;                                 // Graphic update timer
#endif
  
  // BT search menu
  UBYTE   NoOfDevices;                              // BT search no of devices found
  UBYTE   NoOfNames;                                // BT search no of names found
  UBYTE   SelectedDevice;                           // BT selected device
  UBYTE   SelectedSlot;                             // BT selected slot

  // BT device list menu
  UBYTE   DevicesKnown;                             // BT device known flag
  UBYTE   Devices;                                  // BT devices
  UBYTE   DeviceLeft;                               // BT device left
  UBYTE   DeviceCenter;                             // BT device center
  UBYTE   DeviceRight;                              // BT device right
  UBYTE   DeviceType;                               // BT device type

  // BT connect Menu
  UBYTE   Slots;                                    // BT connect no of slots
  UBYTE   SlotLeft;                                 // BT connect
  UBYTE   SlotCenter;                               // BT connect
  UBYTE   SlotRight;                                // BT connect

  // Get user string
  UBYTE   GUSTmp;                                   // Seperat tmp for "Get user string"
  UBYTE   GUSState;                                 // Seperat state for "Get user string"
  UBYTE   GUSNoname;                                // No user entry
  UBYTE   UserString[DISPLAYLINE_LENGTH + 1];       // User string
  UBYTE   DisplayText[DISPLAYLINE_LENGTH + 1];      // Display buffer
  SBYTE   FigurePointer;                            // Figure cursor
  UBYTE   GUSCursor;                                // User string cursor

  // Connect request
  ULONG   CRPasskey;                                // Passkey to fake wrong pin code
  UBYTE   CRState;                                  // Seperate state for "Connect request"
  UBYTE   CRTmp;                                    // Seperate tmp for "Connect request"
  
  // Run files
  UBYTE   *RunIconSave;                             // Menu center icon save
  UWORD   RunTimer;                                 // Bitmap change timer
  UBYTE   RunBitmapPointer;                         // Bitmap pointer
  
  // Delete files
  UBYTE   SelectedType;                             // Type of selected files for delete

  // View
  SLONG   ViewSampleValue;                          // Latch for sensor values
  UBYTE   ViewSampleValid;                          // Latch for sensor valid

#ifndef STRIPPED
  // Datalog
  ULONG   DatalogOldTick;
  ULONG   DatalogRTC;                               // Real time in mS
  ULONG   DatalogTimer;                             // Logging main timer
  ULONG   DatalogSampleTime;                        // Logging sample time
  ULONG   DatalogSampleTimer;                       // Logging sample timer
  SLONG   DatalogSampleValue[DATALOGPORTS];         // Latch for sensor values
  UBYTE   DatalogSampleValid[DATALOGPORTS];         // Latch for sensor valid
  UWORD   DatalogError;                             // Error code
  UBYTE   DatalogPort[DATALOGPORTS];                // Logging sensor
  UBYTE   Update;                                   // Update icons flag
#endif
  
  // NV storage
  ULONG   NVTmpLength;                              // Non volatile filelength
  SWORD   NVTmpHandle;                              // Non volatile filehandle
  UBYTE   NVFilename[FILENAME_LENGTH + 1];          // Non volatile file name
  NVDATA  NVData;                                   // Non volatile data

  // Feedback
  UBYTE   *FBText;                                  // Seperate text pointer for feedback
  UWORD   FBTimer;                                  // Seperate timer for feedback
  UBYTE   FBState;                                  // Seperate state for feedback
  UBYTE   FBPointer;                                // Seperate pointer for feedback

  // BT command
  UBYTE   BTIndex;                                  // List index
  UBYTE   BTTmpIndex;                               // Tmp list index
  UBYTE   BTCommand;                                // Last lached BT command
  UBYTE   BTPar1;                                   // Last lached BT command parameter 1
  UBYTE   BTPar2;                                   // Last lached BT command parameter 2
  UWORD   BTResult;                                 // Last lached BT command result

  // Error display
  UBYTE   ErrorTimer;                               // Error show timer  
  UBYTE   ErrorFunction;                            // Error latched function
  UBYTE   ErrorParameter;                           // Error latched parameter
  UBYTE   ErrorState;                               // Error latched state
  UBYTE   ErrorString[DISPLAYLINE_LENGTH + 1];      // Error string
}VARSUI;


void      cUiInit(void* pHeader);                   // Init controller
void      cUiCtrl(void);                            // Run  controller
void      cUiExit(void);                            // Exit controller

extern    const HEADER cUi;

#endif
