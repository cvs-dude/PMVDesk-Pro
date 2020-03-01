/**************************************************************************
 *  File name  :  pmvd.c
 *  ¸ Copyright Carrick von Schoultz 1994. All rights reserved.
 *  Description:  
 *************************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSERRORS

#include <os2.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "pmvd.h"
#include "xtrn.h"

MRESULT EXPENTRY MainWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY SubFrameWndProc(HWND, ULONG, MPARAM, MPARAM);

int main (INT argc, CHAR *argv[]) {
   CHAR  szTitle[64], szMultCpyMutex[256];
   CHAR szSemaphoreQuit[256];
   HAB   habMain = WinInitialize (0);
   HEV   hevQuit;
   HMQ   hmqMain = WinCreateMsgQueue (habMain, 0); // default is 10
   HMTX  hmtx;
   HWND  hwndFrame, hwndClient, hwndHelp, hwndMyDesktopWnd;
   QMSG  qmsg;
// FCF_HIDEBUTTON | FCF_MINMAX | 
   ULONG flFrameFlags = FCF_TITLEBAR | FCF_SYSMENU | FCF_ICON | FCF_SIZEBORDER | FCF_HIDEMAX | 
                        FCF_ACCELTABLE | FCF_AUTOICON | FCF_NOBYTEALIGN | FCF_TASKLIST;
   ULONG ulClr = SYSCLR_FIELDBACKGROUND;

   WinLoadString (habMain, NULLHANDLE, SZ_MULT_CPY_MUTEX, sizeof (szMultCpyMutex), szMultCpyMutex);
   // check if program allready running
   if (DosCreateMutexSem (szMultCpyMutex, &hmtx, 0, TRUE) == ERROR_DUPLICATE_NAME){
      CHAR szRunning[256];
      DosOpenMutexSem(szMultCpyMutex, &hmtx);
      WinLoadString (habMain, NULLHANDLE, SZ_RUNNING, sizeof (szRunning), szRunning);
      WinAlarm(HWND_DESKTOP, WA_ERROR);
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, szRunning, (PSZ)NULL, 0, MB_OK | MB_ERROR);
      WinDestroyMsgQueue(hmqMain);
      WinTerminate(habMain);
      return(1);
      }

   WinLoadString (habMain, NULLHANDLE, SZ_UNREGISTERED, sizeof (szTitle), szTitle);
   WinRegisterClass (habMain, szTitle, MainWndProc, CS_SIZEREDRAW | CS_SYNCPAINT | CS_CLIPCHILDREN, sizeof (PVOID) * 2);

   hwndFrame = WinCreateStdWindow (HWND_DESKTOP, 0L, &flFrameFlags, (PSZ)szTitle, (PSZ)szTitle, 0L, NULLHANDLE, ID_MAINWND, &hwndClient);
   WinSetPresParam (hwndFrame, PP_BACKGROUNDCOLOR, sizeof (ulClr), &ulClr);
//   hwndHelp = InstallHelp (habMain, hwndFrame, hmqMain);

   while (WinGetMsg (habMain, &qmsg, (HWND)NULL, 0, 0)) {
      WinDispatchMsg (habMain, &qmsg);
      if (qmsg.msg == UM_INITDESKTOP)
         hwndMyDesktopWnd = HWNDFROMMP(qmsg.mp1);
      }
   WinLoadString (habMain, NULLHANDLE, SZ_SEMAPHORE_QUIT, sizeof (szSemaphoreQuit), szSemaphoreQuit);
   DosOpenEventSem (szSemaphoreQuit, &hevQuit);
   WinPostMsg (hwndMyDesktopWnd, UM_QUIT, MPVOID, MPVOID);
   DosWaitEventSem (hevQuit, SEM_INDEFINITE_WAIT);
   DosCloseEventSem (hevQuit);
//   if ((BOOL)hwndHelp) {
//      WinDestroyHelpInstance (hwndHelp);
//      }
   WinDestroyWindow (hwndFrame);
   DosCloseMutexSem (hmtx);
   WinDestroyMsgQueue (hmqMain);
   WinTerminate (habMain);
   return(0);
   }

LOCAL VOID LoadSettings (HWND hwnd, PSETTINGS pSettings) {
   HAB   hab = WinQueryAnchorBlock (hwnd);
   HINI  hini;
   PSZ   pszIniFileName;
   CHAR  szAppName[80], szKeyName[80], szEnviron[80], szVersion[80], szIni[80];
   ULONG ulBufferSize = sizeof (*pSettings);
   BOOL  flAction;

   WinLoadString (hab, NULLHANDLE, SZ_APPNAME, sizeof (szAppName), szAppName);
   WinLoadString (hab, NULLHANDLE, SZ_KEYNAME, sizeof (szKeyName), szKeyName);
   WinLoadString (hab, NULLHANDLE, SZ_ENVIRON_VAR, sizeof (szEnviron), szEnviron);
   WinLoadString (hab, NULLHANDLE, SZ_INI_FILE, sizeof (szIni), szIni);
   WinLoadString (hab, NULLHANDLE, SZ_VERSION, sizeof (szVersion), szVersion);
   if (DosScanEnv ((PSZ)szEnviron, &pszIniFileName))
      pszIniFileName = szIni;
   hini = PrfOpenProfile (hab, pszIniFileName);
   if (!PrfQueryProfileData (hini, szAppName, szKeyName, pSettings, &ulBufferSize)) {
      SETTINGS settingsDefault = DEFAULTSETTINGS;
      *pSettings = settingsDefault;
      strcpy (pSettings->chVersion, szVersion);
      flAction = SWP_SIZE | SWP_SHOW | SWP_MOVE | SWP_ACTIVATE;   
      }
   else {
      flAction = SWP_SIZE | SWP_SHOW | SWP_MOVE | SWP_ACTIVATE; // | pSettings->windowpos.fMinimized;
      }
   WinSetWindowPos (hwnd, 0, pSettings->windowpos.x, pSettings->windowpos.y, pSettings->windowpos.cx, pSettings->windowpos.cy, flAction);
   PrfCloseProfile (hini);
   }

LOCAL VOID SaveSettings (HWND hwnd, PSETTINGS pSettings) {
   HAB   hab = WinQueryAnchorBlock (hwnd);
   HINI  hini;
   PSZ   pszIniFileName;
   CHAR  szAppName[80], szKeyName[80], szEnviron[80], szIni[80];
   SWP   swp;

   WinLoadString (hab, NULLHANDLE, SZ_APPNAME, sizeof (szAppName), szAppName);
   WinLoadString (hab, NULLHANDLE, SZ_KEYNAME, sizeof (szKeyName), szKeyName);
   WinLoadString (hab, NULLHANDLE, SZ_ENVIRON_VAR, sizeof (szEnviron), szEnviron);
   WinLoadString (hab, NULLHANDLE, SZ_INI_FILE, sizeof (szIni), szIni);
   WinQueryWindowPos (hwnd, &swp);
   if ((swp.fl & SWP_MINIMIZE) || (swp.fl & SWP_MAXIMIZE)) {
      pSettings->windowpos.x  = WinQueryWindowUShort(hwnd, QWS_XRESTORE);
      pSettings->windowpos.y  = WinQueryWindowUShort(hwnd, QWS_YRESTORE);
      pSettings->windowpos.cx = WinQueryWindowUShort(hwnd, QWS_CXRESTORE);
      pSettings->windowpos.cy = WinQueryWindowUShort(hwnd, QWS_CYRESTORE);
      }
   else {
      pSettings->windowpos.x  = swp.x;
      pSettings->windowpos.y  = swp.y;
      pSettings->windowpos.cx = swp.cx;
      pSettings->windowpos.cy = swp.cy;
      }
   pSettings->windowpos.fMinimized   = swp.fl & SWP_MINIMIZE;
   if (DosScanEnv ((PSZ)szEnviron, &pszIniFileName))
      pszIniFileName = szIni;
   hini = PrfOpenProfile (hab, pszIniFileName);
   PrfWriteProfileData (hini, szAppName, szKeyName, pSettings, sizeof (*pSettings));
   PrfCloseProfile (hini);
   }

VOID SetParent(HWND hwnd, PTITLEBAR pTitleBar) {
   WinSetParent (pTitleBar->hwndTitleBar, hwnd, FALSE);
   WinSetParent (pTitleBar->hwndSysMenu, hwnd, FALSE);
   WinSetParent (pTitleBar->hwndMinMax, hwnd, FALSE);
}

VOID TitleBar (HWND hwndFrame, PTITLEBAR pTitleBar, PSETTINGS pSettings){
   SWP     swp;
   LONG    cxWindowSize, cyWindowSize;

   WinQueryWindowPos(hwndFrame, (PSWP)&swp);
   if (!(swp.fl & SWP_MINIMIZE)){
      if (pSettings->flTitleBarHidden){
         SetParent (hwndFrame, pTitleBar);
         cxWindowSize = WinQuerySysValue(HWND_DESKTOP, SV_CXMINMAXBUTTON)*2 + 2*WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);
         cyWindowSize = WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR) + 2*WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);
         if (swp.cx < cxWindowSize)
            swp.cx = cxWindowSize;
         if (swp.cy < cyWindowSize)
            swp.cy = cyWindowSize;
         WinSetWindowPos (hwndFrame, swp.hwndInsertBehind, swp.x, swp.y, swp.cx, swp.cy, SWP_SIZE);
         }
      else {
         SetParent (HWND_OBJECT, pTitleBar);
         }
      WinSendMsg(hwndFrame, WM_UPDATEFRAME, MPFROMLONG(FCF_TITLEBAR | FCF_SYSMENU | FCF_MINMAX), 0);
      pSettings->flTitleBarHidden = !pSettings->flTitleBarHidden;
      }
   } // End of TitleBar

BOOL SetCascadeDefault(HWND hwnd,USHORT usSubmenu,USHORT usDefault) {
   MENUITEM miItem;
   ULONG ulStyle;
 
   WinSendMsg(hwnd, MM_QUERYITEM, MPFROM2SHORT(usSubmenu,TRUE), MPFROMP(&miItem));
   ulStyle=WinQueryWindowULong(miItem.hwndSubMenu,QWL_STYLE);
   ulStyle|=MS_CONDITIONALCASCADE;
   WinSetWindowULong(miItem.hwndSubMenu,QWL_STYLE,ulStyle);
   WinSendMsg(miItem.hwndSubMenu, MM_SETDEFAULTITEMID, MPFROM2SHORT(usDefault,FALSE), 0L);
   WinSendMsg(miItem.hwndSubMenu, MM_SETITEMATTR, MPFROM2SHORT(usDefault,FALSE),
               MPFROM2SHORT(MIA_CHECKED,MIA_CHECKED));
   return TRUE;
   }  // End of SetCascadeDefault

VOID EnableMenuItem(HWND hwndMenu, USHORT idItem, BOOL flEnable){
   SHORT fsFlag;

   if(flEnable)
      fsFlag = 0;
   else
      fsFlag = MIA_DISABLED;
   WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(idItem, TRUE), MPFROM2SHORT(MIA_DISABLED, fsFlag));
   } // EnableMenuItem

VOID CheckMenuItem(HWND hwndMenu, SHORT idItem, BOOL flEnable){
   SHORT fsFlag;

   if(flEnable)
      fsFlag = MIA_CHECKED;
   else
      fsFlag = 0;
   WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(idItem, TRUE), MPFROM2SHORT(MIA_CHECKED, fsFlag));
   } // CheckMenuItem

typedef struct {
   CHAR              szTitle[64], szMyWindowClass[20];
   HAB               hab;
   HDC               hdc;
   HPS               hps;
   HWND              hwndClient, hwndFrame, hwndPopupMenu, hwndMyDesktopWnd;
   PFNWP             pOldFrameWndProc;
   POINTL            ptlScreen, ptlBorder;
   TID               tidMyDesktop;
   TITLEBAR          titleBar;
   } MAINWINDOWINSTANCEDATA, *PMAINWINDOWINSTANCEDATA;

VOID RetrieveWindows (PMAINWINDOWINSTANCEDATA pInstanceData);

VOID MainSize (HWND hwnd, MPARAM mp1, MPARAM mp2, PMAINWINDOWINSTANCEDATA pInstanceData, PSETTINGS pSettings){
   LONG   moveX, moveY;
   SWP    swp1, swp2;
   POINTL pointl;
   BOOL   fmove = FALSE;
   RECTL  rcl;

   WinQueryWindowPos(pInstanceData->hwndFrame, (PSWP)&swp1);
   WinQueryMsgPos(pInstanceData->hab, &pointl);
   WinQueryWindowPos(pInstanceData->hwndMyDesktopWnd, (PSWP)&swp2);
   if ((pointl.x >= swp1.x-2) && (pointl.x <= swp1.x+pInstanceData->ptlBorder.x+2)){
      moveX = SHORT1FROMMP(mp2)-SHORT1FROMMP(mp1);
      swp2.x += moveX;
      fmove = TRUE;
      }
   if ((pointl.y >= swp1.y-1) && (pointl.y <= swp1.y+pInstanceData->ptlBorder.y+1)){
      moveY = SHORT2FROMMP(mp2)-SHORT2FROMMP(mp1);
      swp2.y += moveY;
      fmove = TRUE;
      }
   WinQueryWindowRect(hwnd, &rcl);
   if (swp2.x<0){
      swp2.x=0;
      fmove = TRUE;
      }
   else if (swp2.x+swp2.cx>rcl.xRight){
      swp2.x=rcl.xRight-swp2.cx;
      fmove = TRUE;
      }

   if (swp2.y<0){
      swp2.y=0;
      fmove = TRUE;
      }
   else if (swp2.y+swp2.cy>rcl.yTop){
      swp2.y=rcl.yTop-swp2.cy;
      fmove = TRUE;
      }

   if (fmove == TRUE) {
      CHAR   szClass[64];
      HENUM  hEnum;
      HWND   hwndNext;
      PSWP   pswp, pswpBase;

      moveX = swp2.x - pSettings->ptlMyDesktop.x;
      moveY = swp2.y - pSettings->ptlMyDesktop.y;
      pSettings->ptlMyDesktop.x = swp2.x;
      pSettings->ptlMyDesktop.y = swp2.y;
      pswpBase = (PSWP)calloc (2500, sizeof (SWP));
      pswp = pswpBase;
      hEnum = WinBeginEnumWindows (hwnd);
      while (hwndNext = WinGetNextWindow (hEnum)) {
         WinQueryClassName(hwndNext, sizeof(szClass), szClass);
         if (strcmp (szClass, "#1"))
            continue;
         WinQueryWindowPos(hwndNext, pswp);
         pswp->x += moveX;
         pswp->y += moveY;
         pswp->fl = SWP_MOVE;
         pswp++;
         } 
      WinEndEnumWindows (hEnum);
      WinSetMultWindowPos(pInstanceData->hab, pswpBase, ((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP));
      free ((PVOID)pswpBase);
      }
   } // MainSize

MRESULT EXPENTRY MainWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   PMAINWINDOWINSTANCEDATA pInstanceData = WinQueryWindowPtr (hwnd, QWL_USER);
   PSETTINGS pSettings = WinQueryWindowPtr (hwnd, QWL_USER + sizeof (PVOID));

   switch (msg) {
      case WM_CREATE: {
         SIZEL sizl = { 0, 0 };
         pInstanceData = (PMAINWINDOWINSTANCEDATA)calloc (sizeof (MAINWINDOWINSTANCEDATA), 1);
         WinSetWindowPtr (hwnd, QWL_USER, pInstanceData);
         pSettings = (PSETTINGS)calloc (sizeof (SETTINGS), 1);
         WinSetWindowPtr (hwnd, QWL_USER + sizeof (PVOID), pSettings);
         pInstanceData->hwndClient       = hwnd;
         pInstanceData->hwndFrame        = WinQueryWindow (hwnd, QW_PARENT);
         pInstanceData->hab              = WinQueryAnchorBlock (pInstanceData->hwndFrame);
         pInstanceData->hwndPopupMenu    = WinLoadMenu(HWND_OBJECT, NULLHANDLE, IDM_POPUPMENU);
         pInstanceData->ptlScreen.x      = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
         pInstanceData->ptlScreen.y      = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);
         pInstanceData->ptlBorder.x      = WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);
         pInstanceData->ptlBorder.y      = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);
         pInstanceData->pOldFrameWndProc = WinSubclassWindow(pInstanceData->hwndFrame, (PFNWP)SubFrameWndProc);
         pInstanceData->titleBar.hwndTitleBar = WinWindowFromID(pInstanceData->hwndFrame, FID_TITLEBAR);
         pInstanceData->titleBar.hwndSysMenu  = WinWindowFromID(pInstanceData->hwndFrame, FID_SYSMENU);
         pInstanceData->titleBar.hwndMinMax   = WinWindowFromID(pInstanceData->hwndFrame, FID_MINMAX);
//         WinLoadString (pInstanceData->hab, (HMODULE)NULL, SZ_TITLE, sizeof (pInstanceData->szTitle), pInstanceData->szTitle);
         WinLoadString (pInstanceData->hab, (HMODULE)NULL, SZ_MYWINDOWS, sizeof (pInstanceData->szMyWindowClass), pInstanceData->szMyWindowClass);
         if ((pInstanceData->hdc = WinQueryWindowDC (hwnd)) == NULLHANDLE)
            pInstanceData->hdc = WinOpenWindowDC (hwnd);
         pInstanceData->hps = GpiCreatePS (pInstanceData->hab, pInstanceData->hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);
         SetCascadeDefault(pInstanceData->hwndPopupMenu, IDM_HELP, IDM_PRODUCTINFO);
         LoadSettings (pInstanceData->hwndFrame, pSettings);
         if (pSettings->flTitleBarHidden) {
            SetParent (HWND_OBJECT, &pInstanceData->titleBar);
            WinSendMsg(pInstanceData->hwndFrame, WM_UPDATEFRAME, MPFROMLONG(FCF_TITLEBAR | FCF_SYSMENU | FCF_MINMAX), 0);
            }
         if (pSettings->flFloatOnTop)
            WinStartTimer (pInstanceData->hab, hwnd, ID_TIMER, 1000);
         DosCreateThread(&pInstanceData->tidMyDesktop, (PFNTHREAD)MyDesktopThread, (ULONG)hwnd, 0, 65536);
         EnableMenuItem(pInstanceData->hwndPopupMenu, pSettings->lScale+IDM_SCALE, FALSE);
         EnableMenuItem(pInstanceData->hwndPopupMenu, pSettings->lScreenGrid+IDM_SCREENGRIDNO, FALSE);
         EnableMenuItem(pInstanceData->hwndPopupMenu, pSettings->lWindowGrid+IDM_WINDOWGRIDNO, FALSE);
         }
         return ((MRESULT)0);

      case WM_INITMENU:
         switch (SHORT1FROMMP (mp1)) {
            case IDM_HELP:
               EnableMenuItem(pInstanceData->hwndPopupMenu, IDM_HELPUSINGHELP, FALSE);
               EnableMenuItem(pInstanceData->hwndPopupMenu, IDM_HELPGENERAL, FALSE);
               EnableMenuItem(pInstanceData->hwndPopupMenu, IDM_HELPKEYS, FALSE);
               EnableMenuItem(pInstanceData->hwndPopupMenu, IDM_HELPINDEX, FALSE);
               break;

            default:
               // Message came from popupmenu
               CheckMenuItem(pInstanceData->hwndPopupMenu, IDM_TITLEBAR, pSettings->flTitleBarHidden);
               CheckMenuItem(pInstanceData->hwndPopupMenu, IDM_FLOAT, pSettings->flFloatOnTop);
               CheckMenuItem(pInstanceData->hwndPopupMenu, IDM_RETRIEVE, pSettings->flRetrieveWindows);
               break;
            }
           break;

      case WM_CONTEXTMENU: {
         POINTL pointl;

         WinQueryPointerPos(HWND_DESKTOP, &pointl);
         WinMapWindowPoints(HWND_DESKTOP, hwnd, &pointl,1);
         WinPopupMenu(hwnd, hwnd, pInstanceData->hwndPopupMenu,
                      pointl.x, pointl.y, 0, PU_NONE | PU_MOUSEBUTTON1
                      | PU_MOUSEBUTTON2 | PU_KEYBOARD | PU_HCONSTRAIN | PU_VCONSTRAIN );
         }
         return ((MRESULT)0);

      case WM_COMMAND:
         switch (SHORT1FROMMP(mp1)) {

            case IDM_TITLEBAR :
               TitleBar(pInstanceData->hwndFrame, &pInstanceData->titleBar, pSettings);
               break ;

            case IDM_FLOAT :
               if (pSettings->flFloatOnTop)
                  WinStopTimer (pInstanceData->hab, hwnd, ID_TIMER);
               else
                  WinStartTimer (pInstanceData->hab, hwnd, ID_TIMER, 1000);
               pSettings->flFloatOnTop = !pSettings->flFloatOnTop;
               break;

            case IDM_RETRIEVE :
               pSettings->flRetrieveWindows = !pSettings->flRetrieveWindows;
               break;

            case IDM_VISIBLE :
               RetrieveWindows (pInstanceData);
               break;

            case IDM_FIVE:
            case IDM_TEN:
            case IDM_FIFTEEN:
            case IDM_TWENTY:
            case IDM_TWENTYFIVE:
               EnableMenuItem(pInstanceData->hwndPopupMenu, pSettings->lScale+IDM_SCALE, TRUE);
               EnableMenuItem(pInstanceData->hwndPopupMenu, SHORT1FROMMP(mp1), FALSE);
               pSettings->lScale = SHORT1FROMMP(mp1) - IDM_SCALE;
               Scale (pInstanceData->hab, hwnd, pSettings, pInstanceData->ptlScreen, pInstanceData->szMyWindowClass);
               break;

            case IDM_SCREENGRIDNO:
            case IDM_SCREENGRIDONE:
            case IDM_SCREENGRIDTWO:
            case IDM_SCREENGRIDTREE:
            case IDM_SCREENGRIDFOUR:
               EnableMenuItem(pInstanceData->hwndPopupMenu, pSettings->lScreenGrid+IDM_SCREENGRIDNO, TRUE);
               EnableMenuItem(pInstanceData->hwndPopupMenu, SHORT1FROMMP(mp1), FALSE);
               pSettings->lScreenGrid = SHORT1FROMMP(mp1) - IDM_SCREENGRIDNO;
               break;

            case IDM_WINDOWGRIDNO:
            case IDM_WINDOWGRIDONE:
            case IDM_WINDOWGRIDTWO:
            case IDM_WINDOWGRIDTREE:
            case IDM_WINDOWGRIDFOUR:
               EnableMenuItem(pInstanceData->hwndPopupMenu, pSettings->lWindowGrid+IDM_WINDOWGRIDNO, TRUE);
               EnableMenuItem(pInstanceData->hwndPopupMenu, SHORT1FROMMP(mp1), FALSE);
               pSettings->lWindowGrid = SHORT1FROMMP(mp1) - IDM_WINDOWGRIDNO;
               break;

            case IDM_POPUPMENU :
               WinPostMsg(hwnd, WM_CONTEXTMENU, MPVOID, MPVOID);
               break;

            case IDM_MOVEDESKTOP:
               WinPostMsg (pInstanceData->hwndMyDesktopWnd, WM_COMMAND, MPFROM2SHORT(IDM_MOVEDESKTOP, 0), MPVOID );
               break;

            case IDM_PRODUCTINFO:
               HelpProductInfo(pInstanceData->hwndFrame);
               break;

            case SC_RESTORE:
               WinSetWindowPos (pInstanceData->hwndFrame, 0, 0, 0, 0, 0, SWP_RESTORE);
               break;

            case SC_MOVE:
               WinSetFocus (HWND_DESKTOP, pInstanceData->hwndFrame);
               WinSendMsg (pInstanceData->hwndFrame, WM_TRACKFRAME, MPFROMSHORT(TF_SETPOINTERPOS | TF_MOVE), MPVOID );
               break;

            case SC_SIZE:
               WinSendMsg (pInstanceData->hwndFrame, WM_TRACKFRAME, MPFROMSHORT(TF_SETPOINTERPOS), MPVOID );
               break;

            case SC_MAXIMIZE:
               WinSetWindowPos (pInstanceData->hwndFrame, 0, 0, 0, 0, 0, SWP_MAXIMIZE);
               break;

            case SC_HIDE:
               WinShowWindow (pInstanceData->hwndFrame, FALSE);
               break;

            case SC_CLOSE:
               WinPostMsg (pInstanceData->hwndFrame, WM_CLOSE, MPVOID, MPVOID );
               break;

            default:
               break;
            }
            break;

      case UM_INITDESKTOP:
         pInstanceData->hwndMyDesktopWnd = HWNDFROMMP(mp1);
         return ((MRESULT)0);

      case WM_MINMAXFRAME:
            if (((PSWP)mp1)->fl & SWP_MINIMIZE){
               WinShowWindow (pInstanceData->hwndFrame, FALSE);
               return ((MRESULT)0);
               }
            else
               break;

      case WM_SIZE:
            if (pInstanceData->hwndMyDesktopWnd)
               MainSize (hwnd, mp1, mp2, pInstanceData, pSettings);
            break;

      case WM_BUTTON1DOWN: 
         WinSetFocus (HWND_DESKTOP, pInstanceData->hwndFrame);
         WinSendMsg (pInstanceData->hwndFrame, WM_TRACKFRAME, MPFROMSHORT(TF_MOVE), MPVOID);
         break;

      case WM_TIMER:
         if (SHORT1FROMMP (mp1) == ID_TIMER){
            if (pSettings->flFloatOnTop) {
               SWP swp;
               WinQueryWindowPos(pInstanceData->hwndFrame, (PSWP)&swp);
               if (swp.hwndInsertBehind != HWND_TOP) {
//                  WinAlarm(HWND_DESKTOP, WA_ERROR);
                  WinSetWindowPos (pInstanceData->hwndFrame, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy, SWP_MOVE | SWP_ZORDER);
                  }
               }
            return ((MRESULT)0);
            }
         else
            break;

      case WM_PAINT: {
         RECTL rclUpdate;
         WinBeginPaint (hwnd, pInstanceData->hps, &rclUpdate);
         WinFillRect (pInstanceData->hps, &rclUpdate, CLR_BLACK);
         WinEndPaint (pInstanceData->hps);
         }
         return ((MRESULT)0);

      case WM_SAVEAPPLICATION:
         SaveSettings (pInstanceData->hwndFrame, pSettings);
         break;

      case WM_DESTROY:
         if (pSettings->flRetrieveWindows)
            RetrieveWindows (pInstanceData);
         DosWaitThread(&pInstanceData->tidMyDesktop, DCWW_WAIT);
         GpiAssociate (pInstanceData->hps, NULLHANDLE);
         GpiDestroyPS (pInstanceData->hps);
         free ((PVOID)pSettings);
         free ((PVOID)pInstanceData);
         return ((MRESULT)0);

      }
   return (WinDefWindowProc (hwnd, msg, mp1, mp2));
   }

MRESULT EXPENTRY SubFrameWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2){
   PMAINWINDOWINSTANCEDATA  pInstanceData;
   HWND       hwndClient;
   PSETTINGS  pSettings;
   PTRACKINFO ptrack;
   LONG       cxWindowSize, cyWindowSize;
   SWP        swp;

   if(NULLHANDLE != (hwndClient = WinWindowFromID(hwnd, FID_CLIENT))) {
      pInstanceData = WinQueryWindowPtr (hwndClient, QWL_USER);
      pSettings     = WinQueryWindowPtr (hwndClient, QWL_USER + sizeof (PVOID));
      if((NULL != pInstanceData) && (NULL != pSettings)) {

         switch (msg) {

            case WM_QUERYTRACKINFO:
               (*(pInstanceData->pOldFrameWndProc))(hwnd, msg, mp1, mp2);
               WinQueryWindowPos(hwnd, (PSWP)&swp);
               if (swp.fl & SWP_MINIMIZE)
                  break;
               else if (pSettings->flTitleBarHidden){
                  cxWindowSize = pInstanceData->ptlScreen.x / pSettings->lScale + 2*pInstanceData->ptlBorder.x;
                  cyWindowSize = pInstanceData->ptlScreen.y / pSettings->lScale + 2*pInstanceData->ptlBorder.y;
                  }
               else {
                  cxWindowSize = max (pInstanceData->ptlScreen.x / pSettings->lScale + 2*pInstanceData->ptlBorder.x,
                                      WinQuerySysValue(HWND_DESKTOP, SV_CXMINMAXBUTTON)*2 + 2*pInstanceData->ptlBorder.x);
                  cyWindowSize = pInstanceData->ptlScreen.y / pSettings->lScale + WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR) + 2*pInstanceData->ptlBorder.y;
                  }
               ptrack = (PTRACKINFO)mp2;
               ptrack->rclBoundary.yTop    = pInstanceData->ptlScreen.y + pInstanceData->ptlBorder.y;
               ptrack->rclBoundary.yBottom = 0 - pInstanceData->ptlBorder.y;
               ptrack->rclBoundary.xLeft   = 0 - pInstanceData->ptlBorder.x;
               ptrack->rclBoundary.xRight  = pInstanceData->ptlScreen.x + pInstanceData->ptlBorder.x;
               ptrack->cxGrid = 1;                       // smooth tracking with mouse
               ptrack->cyGrid = 1;
               ptrack->cxKeyboard = 1;                   // smooth tracking using cursor keys
               ptrack->cyKeyboard = 1;
               ptrack->ptlMinTrackSize.x = cxWindowSize; // set smallest allowed size of rectangle
               ptrack->ptlMinTrackSize.y = cyWindowSize;
//               ptrack->ptlMaxTrackSize.x = 200;          // set largest allowed size of rectangle
//               ptrack->ptlMaxTrackSize.y = 200;
               ptrack->fs |= TF_GRID;
               return((MRESULT)TRUE);

            default:
               break;
            }
         }
      }
   return (*(pInstanceData->pOldFrameWndProc))(hwnd, msg, mp1, mp2);
   } // End of SubFrameWndProc

VOID RetrieveWindows (PMAINWINDOWINSTANCEDATA pInstanceData) {
   CHAR      szWindowTitle[30], szClassName[10];
   HENUM     hEnum;
   HWND      hwndNext, hwndDesktop;
   PSWP      pswp, pswpBase, pswp2, pswp2Base;
   SWP       swpTemp;
   LONG      lCount;

   pswpBase    = (PSWP)calloc (2500, sizeof (SWP));
   pswp2Base   = (PSWP)calloc (2500, sizeof (SWP));
   pswp        = pswpBase;
   pswp2       = pswp2Base;
   hwndDesktop = WinQueryWindow (pInstanceData->hwndFrame, QW_PARENT);

   hEnum = WinBeginEnumWindows (HWND_DESKTOP);
   while (hwndNext = WinGetNextWindow (hEnum)) {
      if ((WinQueryWindow (hwndNext, QW_PARENT)) != hwndDesktop)
         continue;
      WinQueryClassName (hwndNext, sizeof(szClassName), szClassName);
      if (!(strcmp (szClassName, "#32765"))) // ignore icon title
         continue;
      if (!(strcmp (szClassName, "#4"))) // ignore menu
         continue;
      WinQueryWindowText (hwndNext, sizeof(szWindowTitle), szWindowTitle);
//      if (!(strcmp (szWindowTitle, "Desktop"))) // ignore Desktop window
//         continue;
      if (!(strcmp (szWindowTitle, "WIN-OS/2 window: session"))) // ignore "WIN-OS/2 window: session"
         continue;
      if (!(strcmp (szWindowTitle, "Seamless Window"))) // ignore "Seamless Window"
         continue;
      WinQueryWindowPos (hwndNext, &swpTemp);
      if (swpTemp.cx <= 0) {
//         WinShowWindow (hwndNext, FALSE);
         continue;
         }
      if (swpTemp.cy <= 0) {
//         WinShowWindow (hwndNext, FALSE);
         continue;
         }
      if (swpTemp.fl & SWP_HIDE)
         continue;
      if (swpTemp.fl & SWP_MAXIMIZE)
         WinSetWindowPos (hwndNext, 0,0,0,0,0, SWP_RESTORE);
      if (WinQueryWindow (hwndNext, QW_FRAMEOWNER)) {
         WinQueryWindowPos (hwndNext, pswp2);
         if ((pswp2->x < -10) || (pswp2->x > pInstanceData->ptlScreen.x)) {
            pswp2->x = 5;
            }
         if (((pswp2->y + pswp2->cy) < 0) || ((pswp2->y + pswp2->cy) > (pInstanceData->ptlScreen.y + 5))) {
            pswp2->y = 5;
            }
         pswp2->fl   = (SWP_NOADJUST | SWP_MOVE);
         pswp2++;
         }
      else {
         WinQueryWindowPos (hwndNext, pswp);
         if ((pswp->x < -10) || (pswp->x > pInstanceData->ptlScreen.x)) {
            pswp->x = 5;
            }
         if (((pswp->y + pswp->cy) < 0) || ((pswp->y + pswp->cy) > (pInstanceData->ptlScreen.y + 5))) {
            pswp->y = 5;
            }
         pswp->fl   = (SWP_NOADJUST | SWP_MOVE);
         pswp++;
         }
      }
   WinEndEnumWindows (hEnum);
   lCount = (((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP)) - 1;
   if (lCount > 0)
      WinSetMultWindowPos (pInstanceData->hab, pswpBase, lCount);
   if (pswp2 > pswp2Base)
      WinSetMultWindowPos (pInstanceData->hab, pswp2Base, ((ULONG)pswp2 - (ULONG)pswp2Base) / sizeof (SWP));
   free ((PVOID)pswpBase);
   free ((PVOID)pswp2Base);
   }

