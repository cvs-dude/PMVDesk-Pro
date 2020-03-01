/**************************************************************************
 *  File name  :  desktop.c
 *  ¸ Copyright Carrick von Schoultz 1994. All rights reserved.
 *  Description:  
 *************************************************************************/

#define INCL_WIN
#define INCL_DOS

#include <os2.h>
#include <string.h>
#include <stdlib.h>

#include "desktop.h"
#include "pmvd.h"
#include "xtrn.h"
#include "pmvddll.h"
#include "windows.h"

MRESULT EXPENTRY MyDesktopWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY SubDesktopWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
// VOID SetDesktopPos (HAB hab, HWND hwndMainClient, LONG lPosX, LONG lPosY);
VOID SetScreenPos (PMYDESKTOPINSTANCEDATA pMyDesktopData, PSETTINGS pSettings, LONG lPosX, LONG lPosY);

VOID DestroyAllMyWindows (HWND hwndParent, CHAR szMyWindowClass[20]) {
   CHAR   szClass[20];
   HENUM  hEnum;
   HWND   hwndNext, hwndClient;

   hEnum = WinBeginEnumWindows (hwndParent);
   while (hwndNext = WinGetNextWindow (hEnum)) {
      WinQueryClassName(hwndNext, sizeof(szClass), szClass);
      if (strcmp (szClass, "#1"))
         continue;
      hwndClient = WinWindowFromID(hwndNext, FID_CLIENT);
      WinQueryClassName(hwndClient, sizeof(szClass), szClass);
      if (strcmp (szClass, szMyWindowClass))
         continue;
      WinDestroyWindow (hwndNext);
      } 
   WinEndEnumWindows (hEnum);
   }

#pragma linkage(MyDesktopThread, system)
VOID  MyDesktopThread (HWND hwnd) {
   CHAR   szMyDesktop[20], szMyWindowClass[20];
   CHAR   szSemaphoreQuit[256];
   HAB    habMyDesktop = WinInitialize (0);
   HMQ    hmqMyDesktop = WinCreateMsgQueue (habMyDesktop, 2500); // default is 10
   HEV    hevQuit;
   QMSG   qmsg;
   HWND   hwndMyDesktop, hwndMyDesktopClient;
   ULONG  flFrameFlags = FCF_BORDER | FCF_NOBYTEALIGN;
   ULONG  ulClr = SYSCLR_BACKGROUND;

   WinCancelShutdown (hmqMyDesktop, TRUE);
   WinLoadString (habMyDesktop, NULLHANDLE, SZ_MYDESKTOP, sizeof (szMyDesktop), szMyDesktop);
   WinRegisterClass (habMyDesktop, szMyDesktop, MyDesktopWndProc, CS_SIZEREDRAW | CS_SYNCPAINT, sizeof (PVOID) * 2);
   WinLoadString (habMyDesktop, (HMODULE)NULL, SZ_MYWINDOWS, sizeof (szMyWindowClass), szMyWindowClass);
   WinRegisterClass (habMyDesktop, szMyWindowClass, MyWindowsWndProc, CS_SIZEREDRAW | CS_SYNCPAINT, sizeof (PVOID) * 3);
   hwndMyDesktop = WinCreateStdWindow (hwnd, 0L, &flFrameFlags, (PSZ)szMyDesktop, NULL, 0L, NULLHANDLE, 0L, &hwndMyDesktopClient);
   WinSetPresParam (hwndMyDesktop, PP_BACKGROUNDCOLOR, sizeof (ulClr), &ulClr);
   WinLoadString (habMyDesktop, NULLHANDLE, SZ_SEMAPHORE_QUIT, sizeof (szSemaphoreQuit), szSemaphoreQuit);
   DosCreateEventSem (szSemaphoreQuit, &hevQuit, DC_SEM_SHARED, FALSE);
   WinPostMsg (hwnd, UM_INITDESKTOP, MPFROMHWND(hwndMyDesktop), MPFROMP(NULL));
   WinPostMsg (hwndMyDesktop, UM_INITDESKTOP, MPFROMHWND(hwnd), MPFROMP(NULL));

   while (WinWaitMsg (habMyDesktop, 0, 0)) {
      while (WinPeekMsg (habMyDesktop, &qmsg, 0, 0, 0, PM_REMOVE) && (qmsg.msg != UM_QUIT))
         WinDispatchMsg (habMyDesktop, &qmsg);
      if (qmsg.msg == UM_QUIT)
         break;
      }

   WinDestroyWindow (hwndMyDesktop);
   DestroyAllMyWindows (hwnd, szMyWindowClass);

   WinDestroyMsgQueue (hmqMyDesktop);
   WinTerminate (habMyDesktop);
   DosPostEventSem (hevQuit);
   DosCloseEventSem (hevQuit);
   DosExit(0, 0);
   }

VOID UpdateMyWindows (PMYDESKTOPINSTANCEDATA pMyDesktopData, PSETTINGS pSettings) {
   CHAR      szWindowTitle[30], szClassName[10];
   HENUM     hEnum;
   HWND      hwndNext, hwndMyWindow, hwndInsertBehind;
   PSWP      pswp, pswpBase;
   SWP       swpOriginal;
   LONG      lCount;

   pswpBase         = (PSWP)calloc (2500, sizeof (SWP));
   pswp             = pswpBase;
   hwndInsertBehind = HWND_TOP;

   hEnum = WinBeginEnumWindows (HWND_DESKTOP);
   while (hwndNext = WinGetNextWindow (hEnum)) {
//      WinQueryClassName (hwndNext, sizeof(szClassName), szClassName);
//      if (strcmp (szClassName, "#1")) // ignore other than frame windows
//         continue;
//      WinQueryWindowText (hwndNext, sizeof(szDesktopTitle), szDesktopTitle);
//      if (strstr (szDesktopTitle, "Desktop")) // ignore Desktop window
//         continue;
// CLASSINFO
// WinQueryClassInfo(hab, szClassName, classInfo);
// if (!(classInfo.flClassStyle & CS_FRAME))
//    continue; // MS Word does not support this!!!
//            if (WinQueryClassInfo(hab, (PSZ)szClassName, &classinfo) &&
//                    (classinfo.flClassStyle & CS_FRAME))
//                break;  /* We have our frame */

//      if ((WinQueryWindow (hwndNext, QW_PARENT)) != pMyDesktopData->hwndDesktop)
//         continue;
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
      WinQueryWindowPos (hwndNext, &swpOriginal);
      if (swpOriginal.fl & SWP_HIDE)
         continue;
      hwndMyWindow = FindMyWindow (pMyDesktopData->hwndMainClient, hwndNext, pMyDesktopData);
      if (hwndMyWindow) {
         PMYWINDOWINSTANCEDATA pMyWindowData = WinQueryWindowPtr (WinWindowFromID(hwndMyWindow, FID_CLIENT), QWL_USER + 2 * sizeof (PVOID));
         pMyWindowData->hwndInsertBehind = hwndInsertBehind;
         }
      else {
         if ((swpOriginal.cx <= 0) || (swpOriginal.cy <= 0))
            continue;
         hwndMyWindow = CreateMyWindow (hwndNext, hwndInsertBehind, pMyDesktopData, pSettings);
      }
//      WinQueryWindowPos (hwndMyWindow, pswp);
      pswp->hwnd              = hwndMyWindow;
      pswp->hwndInsertBehind  = hwndInsertBehind;
      pswp->x                 = swpOriginal.x / pSettings->lScale + pSettings->ptlMyDesktop.x;
      pswp->y                 = swpOriginal.y / pSettings->lScale + pSettings->ptlMyDesktop.y;
      pswp->cx                = swpOriginal.cx / pSettings->lScale;
      pswp->cy                = swpOriginal.cy / pSettings->lScale;
      pswp->fl                = (SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER);
      pswp++;
      hwndInsertBehind = hwndMyWindow;
      }
   WinEndEnumWindows (hEnum);
   pswp--;                        // delete Desktop-window
   WinDestroyWindow (pswp->hwnd); // delete Desktop-window
   lCount = (((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP));
   if (lCount > 0)
      WinSetMultWindowPos (pMyDesktopData->hab, pswpBase, lCount);
//   WinSetMultWindowPos (pMyDesktopData->hab, pswpBase, (((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP)));
   free ((PVOID)pswpBase);
   }

MRESULT EXPENTRY MyDesktopWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   PMYDESKTOPINSTANCEDATA pMyDesktopData = WinQueryWindowPtr (hwnd, QWL_USER);
   PSETTINGS pSettings = WinQueryWindowPtr (hwnd, QWL_USER + sizeof (PVOID));

   switch (msg) {
      case WM_CREATE: {
         SIZEL sizl = { 0, 0 };
         pMyDesktopData = (PMYDESKTOPINSTANCEDATA)calloc (sizeof (MYDESKTOPINSTANCEDATA), 1);
         WinSetWindowPtr (hwnd, QWL_USER, pMyDesktopData);
         pMyDesktopData->hab                       = WinQueryAnchorBlock (hwnd);
         pMyDesktopData->hwndMyDesktopPopupMenu    = WinLoadMenu(HWND_OBJECT, NULLHANDLE, IDM_DESKTOPPOPUPMENU);
         pMyDesktopData->hwndMyWindowsPopupMenu    = WinLoadMenu(HWND_OBJECT, NULLHANDLE, IDM_WINDOWSMENU);
         pMyDesktopData->pOldFrameWndProc          = WinSubclassWindow(WinQueryWindow (hwnd, QW_PARENT), (PFNWP)SubDesktopWndProc);
         pMyDesktopData->ptlScreen.x               = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
         pMyDesktopData->ptlScreen.y               = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);
         pMyDesktopData->sTimer                    = -1;
         WinLoadString (pMyDesktopData->hab, (HMODULE)NULL, SZ_MYWINDOWS, sizeof (pMyDesktopData->szMyWindowClass), pMyDesktopData->szMyWindowClass);
//         WinRegisterClass (pMyDesktopData->hab, pMyDesktopData->szMyWindowClass, MyWindowsWndProc, CS_SIZEREDRAW | CS_SYNCPAINT, sizeof (PVOID) * 3);
         if ((pMyDesktopData->hdc = WinQueryWindowDC (hwnd)) == NULLHANDLE)
            pMyDesktopData->hdc = WinOpenWindowDC (hwnd);
         pMyDesktopData->hps = GpiCreatePS (pMyDesktopData->hab, pMyDesktopData->hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);
         WinStartTimer (pMyDesktopData->hab, hwnd, ID_UPDATETIMER, 250);
         }
         return ((MRESULT)0);

      case UM_INITDESKTOP: {
         RECTL rcl;
         pMyDesktopData->hwndMainClient = HWNDFROMMP(mp1);
         pMyDesktopData->hwndMainFrame = WinQueryWindow (pMyDesktopData->hwndMainClient, QW_PARENT);
         pMyDesktopData->hwndDesktop = WinQueryWindow (pMyDesktopData->hwndMainFrame, QW_PARENT);
         InstallHook (pMyDesktopData->hab, hwnd, pMyDesktopData->hwndMainFrame);
         pSettings = WinQueryWindowPtr (pMyDesktopData->hwndMainClient, QWL_USER + sizeof (PVOID));
         WinSetWindowPtr (hwnd, QWL_USER + sizeof (PVOID), pSettings);
         WinSetWindowPos (WinQueryWindow (hwnd, QW_PARENT), HWND_BOTTOM,
                          pSettings->ptlMyDesktop.x, pSettings->ptlMyDesktop.y,
                          pMyDesktopData->ptlScreen.x / pSettings->lScale, pMyDesktopData->ptlScreen.y / pSettings->lScale, 
                          SWP_ZORDER | SWP_SIZE | SWP_SHOW | SWP_MOVE);
//         WinSetWindowPos (pMyDesktopData->hwndMainFrame, 0,
//                          0, 0, 0, 0, 
//                          SWP_SHOW | SWP_ACTIVATE);
         UpdateMyWindows (pMyDesktopData, pSettings);
         }
         return ((MRESULT)0);

      case WM_CONTEXTMENU: {
         POINTL pointl;

         WinSetActiveWindow (HWND_DESKTOP, pMyDesktopData->hwndMainFrame);
         WinQueryPointerPos(HWND_DESKTOP, &pointl);
         WinMapWindowPoints(HWND_DESKTOP, hwnd, &pointl,1);
         WinPopupMenu(hwnd, hwnd, pMyDesktopData->hwndMyDesktopPopupMenu,
                      pointl.x, pointl.y, 0, PU_NONE | PU_MOUSEBUTTON1
                      | PU_MOUSEBUTTON2 | PU_KEYBOARD | PU_HCONSTRAIN | PU_VCONSTRAIN );
         }
         return ((MRESULT)0);

      case WM_COMMAND:
         switch (SHORT1FROMMP(mp1)) {

            case IDM_MOVEDESKTOP: {
               SWP swp;
               WinSendMsg(WinQueryWindow (hwnd, QW_PARENT), WM_TRACKFRAME,
                           MPFROMSHORT(TF_SETPOINTERPOS | TF_MOVE), MPVOID);
               WinQueryWindowPos (WinQueryWindow (hwnd, QW_PARENT), &swp);
               if ((pSettings->ptlMyDesktop.x != swp.x) || (pSettings->ptlMyDesktop.y != swp.y)){
//                  SetDesktopPos (pMyDesktopData->hab, pMyDesktopData->hwndMainClient, (pSettings->ptlMyDesktop.x - swp.x) * pSettings->lScale, (pSettings->ptlMyDesktop.y - swp.y) * pSettings->lScale);
                  SetScreenPos (pMyDesktopData, pSettings, (pSettings->ptlMyDesktop.x - swp.x) * pSettings->lScale, (pSettings->ptlMyDesktop.y - swp.y) * pSettings->lScale);
                  pSettings->ptlMyDesktop.x = swp.x;
                  pSettings->ptlMyDesktop.y = swp.y;
                  }
               }
               break;

            default:
               break;
            }
            break;

      case WM_TIMER:
         if (SHORT1FROMMP (mp1) == ID_UPDATETIMER){
            if (pMyDesktopData->sTimer == 0) {
               pMyDesktopData->sTimer--;
               UpdateMyWindows (pMyDesktopData, pSettings);
               }
            else if (pMyDesktopData->sTimer > 0)
               pMyDesktopData->sTimer--;
//            WinAlarm(HWND_DESKTOP, WA_ERROR);
            return ((MRESULT)0);
            }
         else
            break;

      case UM_DESTROY: {
         HWND hwndMyWindow = FindMyWindow (pMyDesktopData->hwndMainClient, HWNDFROMMP(mp1), pMyDesktopData);
         if (hwndMyWindow) {
            WinDestroyWindow (hwndMyWindow);
            WinSendMsg (hwndMyWindow, WM_CLOSE, MPVOID, MPVOID);
//            UpdateMyWindows (pMyDesktopData, pSettings);
            pMyDesktopData->sTimer = 1;
            }
         }
         return ((MRESULT)0);

      case UM_ADJUSTFRAMEPOS: {
         HWND hwndMyWindow = FindMyWindow (pMyDesktopData->hwndMainClient, HWNDFROMMP(mp1), pMyDesktopData);
         if (hwndMyWindow)
            WinPostMsg (hwndMyWindow, UM_ADJUSTFRAMEPOS, mp1, mp2);
//         UpdateMyWindows (pMyDesktopData, pSettings);
         pMyDesktopData->sTimer = 1;
         }
         return ((MRESULT)0);

      case WM_BUTTON1DOWN: {
         SWP swp;
         if (WinGetKeyState(HWND_DESKTOP, VK_CTRL) >= 0) // Ctrl key is not down so process
            WinSetActiveWindow (HWND_DESKTOP, pMyDesktopData->hwndMainFrame);
         WinSendMsg(WinQueryWindow (hwnd, QW_PARENT), WM_TRACKFRAME, MPFROMSHORT(TF_MOVE), MPVOID);
         WinQueryWindowPos (WinQueryWindow (hwnd, QW_PARENT), &swp);
         if ((pSettings->ptlMyDesktop.x != swp.x) || (pSettings->ptlMyDesktop.y != swp.y)){
//            SetDesktopPos (pMyDesktopData->hab, pMyDesktopData->hwndMainClient, (pSettings->ptlMyDesktop.x - swp.x) * pSettings->lScale, (pSettings->ptlMyDesktop.y - swp.y) * pSettings->lScale);
            SetScreenPos (pMyDesktopData, pSettings, (pSettings->ptlMyDesktop.x - swp.x) * pSettings->lScale, (pSettings->ptlMyDesktop.y - swp.y) * pSettings->lScale);
            pSettings->ptlMyDesktop.x = swp.x;
            pSettings->ptlMyDesktop.y = swp.y;
            }
         }
         break;

      case WM_PAINT: {
         RECTL rclUpdate;
         WinBeginPaint (hwnd, pMyDesktopData->hps, &rclUpdate);
         WinFillRect (pMyDesktopData->hps, &rclUpdate, SYSCLR_BACKGROUND);
         WinEndPaint (pMyDesktopData->hps);
         }
         return ((MRESULT)0);

      case WM_DESTROY:
         ReleaseHook();
         WinStopTimer (pMyDesktopData->hab, hwnd, ID_UPDATETIMER);
         GpiAssociate (pMyDesktopData->hps, NULLHANDLE);
         GpiDestroyPS (pMyDesktopData->hps);
         free ((PVOID)pMyDesktopData);
//         WinAlarm(HWND_DESKTOP, WA_ERROR);
         return ((MRESULT)0);

      default:
         break;

      }
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
   }

MRESULT EXPENTRY SubDesktopWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2){
   HWND hwndClient = WinWindowFromID(hwnd, FID_CLIENT);
   PMYDESKTOPINSTANCEDATA pMyDesktopData = WinQueryWindowPtr (hwndClient, QWL_USER);
   PSETTINGS pSettings    = WinQueryWindowPtr (hwndClient, QWL_USER + sizeof (PVOID));

   if((NULL != pMyDesktopData) && (NULL != pSettings)) {

//   PMYDESKTOPINSTANCEDATA  pMyDesktopData;
//   HWND       hwndClient;
//   PTRACKINFO ptrack;
//   if(NULLHANDLE != (hwndClient = WinWindowFromID(hwnd, FID_CLIENT))) {
//      SWP swp;
//      pMyDesktopData = WinQueryWindowPtr (hwndClient, QWL_USER);
//      WinQueryWindowPos (pMyDesktopData->hwndMainFrame, &swp);
//      if ((pMyDesktopData != NULL) && (pMyDesktopData->hwndMainClient != NULLHANDLE) && (!(swp.fl & SWP_MINIMIZE))){
//         RECTL      rcl;
//         PSWP       pswp;

         switch (msg) {

            case WM_ADJUSTWINDOWPOS:{
               PSWP     pswp    = (PSWP)mp1;
               pswp->hwndInsertBehind = HWND_BOTTOM;
               pswp->fl |= SWP_ZORDER | SWP_DEACTIVATE;
               }
               return((MRESULT)AWP_DEACTIVATE);

            case WM_QUERYTRACKINFO:{
               PTRACKINFO ptrack;
               RECTL      rcl;
               (*(pMyDesktopData->pOldFrameWndProc))(hwnd, msg, mp1, mp2);
               ptrack = (PTRACKINFO)mp2;
               WinQueryWindowRect(pMyDesktopData->hwndMainClient, &rcl);
               ptrack->rclBoundary.yTop    = rcl.yTop;
               ptrack->rclBoundary.yBottom = rcl.yBottom;
               ptrack->rclBoundary.xLeft   = rcl.xLeft;
               ptrack->rclBoundary.xRight  = rcl.xRight;
               ptrack->cxBorder = 1;
               ptrack->cyBorder = 1;
               if (pSettings->lScreenGrid) {
                  ptrack->cxGrid = (pMyDesktopData->ptlScreen.x / pSettings->lScale) / pSettings->lScreenGrid;
                  ptrack->cyGrid = (pMyDesktopData->ptlScreen.y / pSettings->lScale) / pSettings->lScreenGrid;
                  ptrack->cxKeyboard = (pMyDesktopData->ptlScreen.x / pSettings->lScale) / pSettings->lScreenGrid;
                  ptrack->cyKeyboard = (pMyDesktopData->ptlScreen.y / pSettings->lScale) / pSettings->lScreenGrid;
                  ptrack->fs |= TF_GRID;
                  }
               ptrack->fs |= TF_ALLINBOUNDARY; // | TF_MOVE | TF_GRID | TF_STANDARD
               }
               return((MRESULT)TRUE);

            }
//         }
      }
   return (*(pMyDesktopData->pOldFrameWndProc))(hwnd, msg, mp1, mp2);
   } // End of SubDesktopWndProc
/*
VOID SetScreenPos (PMYDESKTOPINSTANCEDATA pMyDesktopData, PSETTINGS pSettings, LONG lPosX, LONG lPosY) {
   SWP       swpTemp;
   WinQueryWindowPos (pMyDesktopData->hwndDesktop, &swpTemp);
            swpTemp.x += lPosX;
            swpTemp.y += lPosY;
            swpTemp.fl = (SWP_MOVE);
   WinSetMultWindowPos (pMyDesktopData->hab, &swpTemp, 1);
   }
*/
VOID SetScreenPos (PMYDESKTOPINSTANCEDATA pMyDesktopData, PSETTINGS pSettings, LONG lPosX, LONG lPosY) {
   PMYWINDOWINSTANCEDATA pMyWindowData;
   CHAR      szWindowTitle[30], szClassName[20];
   HENUM     hEnumMyWindows, hEnum;
   HWND      hwndNextMyWindow, hwndNext;
   PSWP      pswp, pswpBase, pswp2, pswp2Base;
   SWP       swpTemp;
   BOOL      flSticky;
   LONG      lCount;

   pswpBase    = (PSWP)calloc (2500, sizeof (SWP));
   pswp2Base   = (PSWP)calloc (2500, sizeof (SWP));
   pswp        = pswpBase;
   pswp2       = pswp2Base;

   hEnum = WinBeginEnumWindows (HWND_DESKTOP);
   while (hwndNext = WinGetNextWindow (hEnum)) {
      if ((WinQueryWindow (hwndNext, QW_PARENT)) != pMyDesktopData->hwndDesktop)
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
      if (swpTemp.fl & SWP_MINIMIZE)
         continue;
      flSticky = FALSE;
      hEnumMyWindows = WinBeginEnumWindows (pMyDesktopData->hwndMainClient);
      while (hwndNextMyWindow = WinGetNextWindow (hEnumMyWindows)) {
         WinQueryClassName(hwndNextMyWindow, sizeof(szClassName), szClassName);
         if (strcmp (szClassName, "#1"))
            continue;
         WinQueryClassName(WinWindowFromID(hwndNextMyWindow, FID_CLIENT), sizeof(szClassName), szClassName);
         if (strcmp (szClassName, pMyDesktopData->szMyWindowClass))
            continue;
         pMyWindowData = WinQueryWindowPtr (WinWindowFromID(hwndNextMyWindow, FID_CLIENT), QWL_USER + 2 * sizeof (PVOID));
         if (pMyWindowData->hwndOriginal == hwndNext) {
            flSticky = pMyWindowData->flSticky;
            break;
            }
         } 
      WinEndEnumWindows (hEnumMyWindows);
//      if (flSticky)
//         continue;
      if (WinQueryWindow (hwndNext, QW_FRAMEOWNER)) {
         WinQueryWindowPos (hwndNext, pswp2);
         if (flSticky)
            pswp2++;
         else {
            pswp2->x += lPosX;
            pswp2->y += lPosY;
            pswp2->fl = (SWP_NOADJUST | SWP_MOVE);
            pswp2++;
            }
         }
      else {
         if (!flSticky) {
            WinQueryWindowPos (hwndNext, pswp);
            pswp->x += lPosX;
            pswp->y += lPosY;
            pswp->fl = (SWP_NOADJUST | SWP_MOVE);
            pswp++;
            }
         }
      }
   WinEndEnumWindows (hEnum);
   lCount = (((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP)) - 1;
   if (lCount > 0)
      WinSetMultWindowPos (pMyDesktopData->hab, pswpBase, lCount);
   if (pswp2 > pswp2Base)
      WinSetMultWindowPos (pMyDesktopData->hab, pswp2Base, ((ULONG)pswp2 - (ULONG)pswp2Base) / sizeof (SWP));
   free ((PVOID)pswpBase);
   free ((PVOID)pswp2Base);
   }
