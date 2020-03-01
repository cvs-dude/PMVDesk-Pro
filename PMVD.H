/**************************************************************************
 *  File name  :	pmvd.h
 *  ¸ Copyright Carrick von Schoultz 1994. All rights reserved.
 *  Description:
 *************************************************************************/

#define LOCAL static

typedef struct {
   HWND hwndTitleBar, hwndSysMenu, hwndMinMax;
   } TITLEBAR, *PTITLEBAR;

typedef struct {
   LONG x, y;
   LONG cx, cy;
   BOOL fMinimized;
   } WINDOWPOS, *PWINDOWPOS;

typedef struct {
   CHAR         chVersion[20];
   BOOL         flTitleBarHidden;
   BOOL         flFloatOnTop;
   BOOL         flRetrieveWindows;
   LONG         lScale;
   LONG         lScreenGrid;
   LONG         lWindowGrid;
   POINTL       ptlMyDesktop;
   WINDOWPOS    windowpos;
   } SETTINGS, *PSETTINGS;

#define DEFAULTSETTINGS {                   \
   "",                                      \
   FALSE,                                   \
   FALSE,                                   \
   FALSE,                                   \
   10,                                      \
   0,                                       \
   0,                                       \
   { 0, 0 },                                \
   { 10, 10, 350, 250, 0 },                 \
   }

#define UM_INITDESKTOP       (WM_USER + 30)
#define UM_INITWINDOW        (WM_USER + 31)
#define UM_CREATE            (WM_USER + 32)
#define UM_DESTROY           (WM_USER + 33)
#define UM_ADJUSTFRAMEPOS    (WM_USER + 34)
#define UM_QUIT              (WM_USER + 35)
#define ID_TIMER             (TID_USERMAX - 1)
#define ID_UPDATETIMER       (TID_USERMAX - 2)
#define ID_MAINWND           100
#define ID_PRODUCTINFO       110
#define ID_ABOUT2            115

/* Ids of messages used in menus */
#define IDM_POPUPMENU                  300
#define IDM_TITLEBAR                   301
#define IDM_FLOAT                      302
#define IDM_RETRIEVE                   303
#define IDM_UTIL                       304
#define IDM_VISIBLE                    305
#define IDM_WINDOW                     306
#define IDM_CANCEL                     307
#define IDM_PRODUCTINFO                308

#define IDM_DESKTOPPOPUPMENU           320
#define IDM_DESKTOPTITLE               321

#define IDM_WINDOWSMENU                330
#define IDM_WINDOWSTITLE               331
#define IDM_MOVEDESKTOP                332
#define IDM_POSITIONSIZE               333
#define IDM_STICKY                     334

#define IDM_SCALE                      340
#define IDM_FIVE                       345
#define IDM_TEN                        350
#define IDM_FIFTEEN                    355
#define IDM_TWENTY                     360
#define IDM_TWENTYFIVE                 365

#define IDM_SCREENGRID                 379
#define IDM_SCREENGRIDNO               380
#define IDM_SCREENGRIDONE              381
#define IDM_SCREENGRIDTWO              382
#define IDM_SCREENGRIDTREE             383
#define IDM_SCREENGRIDFOUR             384

#define IDM_WINDOWGRID                 389
#define IDM_WINDOWGRIDNO               390
#define IDM_WINDOWGRIDONE              391
#define IDM_WINDOWGRIDTWO              392
#define IDM_WINDOWGRIDTREE             393
#define IDM_WINDOWGRIDFOUR             394

#define IDM_HELP                       400
#define IDM_HELPUSINGHELP              410
#define IDM_HELPGENERAL                420
#define IDM_HELPKEYS                   430
#define IDM_HELPINDEX                  440

#define SZ_TITLE                       501
#define SZ_APPNAME                     503
#define SZ_VERSION                     504
#define SZ_KEYNAME                     505
#define SZ_ENVIRON_VAR                 506
#define SZ_INI_FILE                    507
#define SZ_UNREGISTERED                508
#define SZ_MULT_CPY_MUTEX              509
#define SZ_RUNNING                     510
#define SZ_MYDESKTOP                   511
#define SZ_MYWINDOWS                   512
#define SZ_SEMAPHORE_QUIT              513


