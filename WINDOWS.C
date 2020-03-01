/**************************************************************************
 *  File name  :  windows.c
 *  ¸ Copyright Carrick von Schoultz 1994. All rights reserved.
 *  Description:  
 *************************************************************************/

#define INCL_WIN
#define INCL_DOS
#define INCL_GPI

#include <os2.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "pmvd.h"
#include "xtrn.h"
#include "windows.h"

// Undocumented PM Functions
extern BOOL APIENTRY Win32StretchPointer (HPS hps, SHORT x, SHORT y,
                        SHORT cx, SHORT cy, HPOINTER hptr, USHORT fs);
#define QWL_HICON 0x0020

MRESULT EXPENTRY SubWindowsWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

HWND CreateMyWindow (HWND hwndOriginal, HWND hwndInsertBehind, PMYDESKTOPINSTANCEDATA pMyDesktopData, PSETTINGS pSettings){
   PMYWINDOWINSTANCEDATA pMyWindowData;
   HWND  hwndMyWindow, hwndMyWindowClient;
   ULONG flFrameFlags = FCF_BORDER | FCF_NOBYTEALIGN;
   ULONG ulClr = SYSCLR_WINDOW;
   SIZEL sizl = { 0, 0 };
//   CHAR  szTitle[20];

   hwndMyWindow = WinCreateStdWindow (pMyDesktopData->hwndMainClient, 0L, &flFrameFlags, pMyDesktopData->szMyWindowClass, NULL, 0L, NULLHANDLE, 0L, &hwndMyWindowClient);
   WinSetPresParam (hwndMyWindow, PP_BACKGROUNDCOLOR, sizeof (ulClr), &ulClr);
   WinSetWindowPtr (hwndMyWindowClient, QWL_USER, pMyDesktopData);
   WinSetWindowPtr (hwndMyWindowClient, QWL_USER + sizeof (PVOID), pSettings);
   pMyWindowData = (PMYWINDOWINSTANCEDATA)calloc (sizeof (MYWINDOWINSTANCEDATA), 1);
   WinSetWindowPtr (hwndMyWindowClient, QWL_USER + 2 * sizeof (PVOID), pMyWindowData);
   pMyWindowData->pOldFrameWndProc = WinSubclassWindow(hwndMyWindow, (PFNWP)SubWindowsWndProc);
   pMyWindowData->hwndOriginal              = hwndOriginal;
   pMyWindowData->hwndInsertBehind          = hwndInsertBehind;
//   pMyWindowData->hwndMyWindowsPopupMenu    = WinLoadMenu(HWND_OBJECT, NULLHANDLE, IDM_WINDOWSMENU);
   if (pMyDesktopData->hwndMainFrame == hwndOriginal)
      pMyWindowData->flSticky               = TRUE;
   else
      pMyWindowData->flSticky               = FALSE;
//   WinQueryWindowText(pMyWindowData->hwndOriginal, sizeof(szTitle), szTitle);
//   WinSendMsg (pMyWindowData->hwndMyWindowsPopupMenu, MM_SETITEMTEXT,
//               MPFROM2SHORT (IDM_WINDOWSTITLE, sizeof(szTitle)), MPFROMP (szTitle));
   if ((pMyWindowData->hdc = WinQueryWindowDC (hwndMyWindowClient)) == NULLHANDLE)
      pMyWindowData->hdc = WinOpenWindowDC (hwndMyWindowClient);
   pMyWindowData->hps = GpiCreatePS (pMyDesktopData->hab, pMyWindowData->hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);
   return hwndMyWindow;
   }

HWND FindMyWindow (HWND hwndParent, HWND hwndOriginal, PMYDESKTOPINSTANCEDATA pMyDesktopData) {
   PMYWINDOWINSTANCEDATA pMyWindowData;
   CHAR   szClass[20];
   HENUM  hEnum;
   HWND   hwndMyWindow = NULLHANDLE;
   HWND   hwndNext;

   hEnum = WinBeginEnumWindows (hwndParent);
   while (hwndNext = WinGetNextWindow (hEnum)) {
      WinQueryClassName(hwndNext, sizeof(szClass), szClass);
      if (strcmp (szClass, "#1"))
         continue;
      WinQueryClassName(WinWindowFromID(hwndNext, FID_CLIENT), sizeof(szClass), szClass);
      if (strcmp (szClass, pMyDesktopData->szMyWindowClass))
         continue;
      pMyWindowData = WinQueryWindowPtr (WinWindowFromID(hwndNext, FID_CLIENT), QWL_USER + 2 * sizeof (PVOID));
      if (pMyWindowData->hwndOriginal == hwndOriginal) {
         hwndMyWindow = hwndNext;
         break;
         }
      } 
   WinEndEnumWindows (hEnum);
   return hwndMyWindow;
   }

VOID Scale (HAB hab, HWND hwndParent, PSETTINGS pSettings, POINTL ptlScreen, CHAR szMyWindowClass[20]){
   PMYWINDOWINSTANCEDATA pMyWindowData;
   CHAR   szClass[20];
   HENUM  hEnum;
   HWND   hwndNext;
   PSWP   pswp, pswpBase;
   SWP    swpOriginal;

   pswpBase = (PSWP)calloc (2500, sizeof (SWP));
   pswp = pswpBase;
   hEnum = WinBeginEnumWindows (hwndParent);
   while (hwndNext = WinGetNextWindow (hEnum)) {
      WinQueryClassName(hwndNext, sizeof(szClass), szClass);
      if (strcmp (szClass, "#1"))
         continue;
      WinQueryWindowPos(hwndNext, pswp);
      WinQueryClassName(WinWindowFromID(hwndNext, FID_CLIENT), sizeof(szClass), szClass);
      if (strcmp (szClass, szMyWindowClass)){
         pswp->x  = pSettings->ptlMyDesktop.x;
         pswp->y  = pSettings->ptlMyDesktop.y;
         pswp->cx = ptlScreen.x / pSettings->lScale;
         pswp->cy = ptlScreen.y / pSettings->lScale;
         pswp->fl = SWP_MOVE | SWP_SIZE;
         pswp++;
         }
      else {
         pMyWindowData = WinQueryWindowPtr (WinWindowFromID(hwndNext, FID_CLIENT), QWL_USER + 2 * sizeof (PVOID));
         if (pMyWindowData->hwndOriginal) {
            WinQueryWindowPos(pMyWindowData->hwndOriginal, &swpOriginal);
            pswp->x  = swpOriginal.x / pSettings->lScale + pSettings->ptlMyDesktop.x;
            pswp->y  = swpOriginal.y / pSettings->lScale + pSettings->ptlMyDesktop.y;
            pswp->cx = swpOriginal.cx / pSettings->lScale;
            pswp->cy = swpOriginal.cy / pSettings->lScale;
            pswp->fl = SWP_MOVE | SWP_SIZE;
            pswp++;
            }
         }
      } 
   WinEndEnumWindows (hEnum);
   WinSetMultWindowPos(hab, pswpBase, ((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP));
   free ((PVOID)pswpBase);
   } // Scale

MRESULT EXPENTRY MyWindowsWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   PMYDESKTOPINSTANCEDATA pMyDesktopData = WinQueryWindowPtr (hwnd, QWL_USER);
   PSETTINGS pSettings   = WinQueryWindowPtr (hwnd, QWL_USER + sizeof (PVOID));
   PMYWINDOWINSTANCEDATA pMyWindowData = WinQueryWindowPtr (hwnd, QWL_USER + 2 * sizeof (PVOID));

   switch (msg) {

      case WM_INITMENU:
         switch (SHORT1FROMMP (mp1)) {
            default:
               // Message came from popupmenu                
               CheckMenuItem(pMyDesktopData->hwndMyWindowsPopupMenu, IDM_STICKY, pMyWindowData->flSticky);
//               EnableMenuItem(pMyDesktopData->hwndMyWindowsPopupMenu, IDM_STICKY, FALSE);
               EnableMenuItem(pMyDesktopData->hwndMyWindowsPopupMenu, IDM_POSITIONSIZE, FALSE);
               break;
            }
           break;

      case WM_CONTEXTMENU: {
         POINTL pointl;
         CHAR  szTitle[20];
         WinQueryWindowText(pMyWindowData->hwndOriginal, sizeof(szTitle), szTitle);
         WinSendMsg (pMyDesktopData->hwndMyWindowsPopupMenu, MM_SETITEMTEXT,
                     MPFROM2SHORT (IDM_WINDOWSTITLE, sizeof(szTitle)), MPFROMP (szTitle));
         WinSetActiveWindow (HWND_DESKTOP, pMyDesktopData->hwndMainFrame);
         WinQueryPointerPos(HWND_DESKTOP, &pointl);
         WinMapWindowPoints(HWND_DESKTOP, hwnd, &pointl,1);
         WinPopupMenu(hwnd, hwnd, pMyDesktopData->hwndMyWindowsPopupMenu,
                      pointl.x, pointl.y, 0, PU_NONE | PU_MOUSEBUTTON1
                      | PU_MOUSEBUTTON2 | PU_KEYBOARD | PU_HCONSTRAIN | PU_VCONSTRAIN );
         }
         return ((MRESULT)0);

      case WM_COMMAND:
         switch (SHORT1FROMMP(mp1)) {

            case IDM_MOVEDESKTOP:
               WinPostMsg (pMyDesktopData->hwndMainClient, WM_COMMAND, MPFROM2SHORT(IDM_MOVEDESKTOP, 0), MPVOID );
               break;

            case IDM_STICKY:
               if (pMyDesktopData->hwndMainFrame != pMyWindowData->hwndOriginal)
                  pMyWindowData->flSticky = !pMyWindowData->flSticky;
               break;

            default:
               break;
            }
            break;

      case WM_BUTTON1DOWN: {
         HWND hwndInsertBehind;
         SWP swpOld, swpNew, swpOriginal;
         ULONG fl;
//         WinSetActiveWindow (HWND_DESKTOP, pMyDesktopData->hwndMainFrame);
         if (WinGetKeyState(HWND_DESKTOP, VK_CTRL) < 0) { // Ctrl key is down so process
            hwndInsertBehind = NULLHANDLE;
            fl = SWP_MOVE;
            }
         else {
            WinSetActiveWindow (HWND_DESKTOP, pMyDesktopData->hwndMainFrame);
            hwndInsertBehind = pMyDesktopData->hwndMainFrame;
            fl = SWP_MOVE | SWP_ZORDER; // | SWP_ACTIVATE
            }
         WinQueryWindowPos(WinQueryWindow (hwnd, QW_PARENT), &swpOld);
         WinSendMsg(WinQueryWindow (hwnd, QW_PARENT), WM_TRACKFRAME, MPFROMSHORT(TF_MOVE), MPVOID);
         WinQueryWindowPos(WinQueryWindow (hwnd, QW_PARENT), &swpNew);
         WinQueryWindowPos(pMyWindowData->hwndOriginal, &swpOriginal);
         if (swpOriginal.fl & SWP_MINIMIZE){
            CHAR szClassBuffer[80];
            HWND hwndIconText = WinQueryWindow(pMyWindowData->hwndOriginal, QW_NEXT);
            SWP swpIcon;
            WinSetWindowUShort (pMyWindowData->hwndOriginal, QWS_XMINIMIZE, (USHORT)-1);
            WinSetWindowUShort (pMyWindowData->hwndOriginal, QWS_YMINIMIZE, (USHORT)-1);
            WinQueryClassName (hwndIconText, sizeof (szClassBuffer), szClassBuffer);
            if (!strcmp (szClassBuffer, "#32765"))
               WinQueryWindowPos(hwndIconText, &swpIcon);
            fl |= SWP_FOCUSDEACTIVATE;
            WinSetWindowPos (pMyWindowData->hwndOriginal, hwndInsertBehind,
                         (swpNew.x - swpOld.x) * pSettings->lScale + swpOriginal.x,
                         (swpNew.y - swpOld.y) * pSettings->lScale + swpOriginal.y,
                         0, 0, fl);
            if (!strcmp (szClassBuffer, "#32765"))
               WinSetWindowPos (hwndIconText, hwndInsertBehind,
                         (swpNew.x - swpOld.x) * pSettings->lScale + swpIcon.x,
                         (swpNew.y - swpOld.y) * pSettings->lScale + swpIcon.y,
                         0, 0, fl);
            }
         else {
            CHAR  szClassName[20];
            HENUM hEnum;
            HWND  hwndNext;
            PSWP  pswp, pswpBase;

            fl |= SWP_NOADJUST;
            pswpBase         = (PSWP)calloc (2500, sizeof (SWP));
            pswp             = pswpBase;
            hEnum = WinBeginEnumWindows (HWND_DESKTOP);
            while (hwndNext = WinGetNextWindow (hEnum)) {
               WinQueryClassName (hwndNext, sizeof(szClassName), szClassName);
               if (strcmp (szClassName, "#1")) // ignore other than frame windows
                  continue;
               if ((WinQueryWindow (hwndNext, QW_OWNER)) != pMyWindowData->hwndOriginal)
                  continue;
               if ((WinQueryWindowULong (hwndNext, QWL_STYLE)) & FS_NOMOVEWITHOWNER)
                  continue;
               WinQueryWindowPos (hwndNext, pswp);
               pswp->hwndInsertBehind  = hwndInsertBehind;
               pswp->x                 = (swpNew.x - swpOld.x) * pSettings->lScale + pswp->x;
               pswp->y                 = (swpNew.y - swpOld.y) * pSettings->lScale + pswp->y;
               pswp->fl                = fl;
               pswp++;
               hwndInsertBehind = hwndNext;
               }
            WinEndEnumWindows (hEnum);
            WinSetWindowPos (pMyWindowData->hwndOriginal, hwndInsertBehind,
                         (swpNew.x - swpOld.x) * pSettings->lScale + swpOriginal.x,
                         (swpNew.y - swpOld.y) * pSettings->lScale + swpOriginal.y,
                         0, 0, fl);
            if (pswp > pswpBase)
               WinSetMultWindowPos (pMyDesktopData->hab, pswpBase, (((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP)));
            free ((PVOID)pswpBase);
            }
         }
         break;

      case UM_ADJUSTFRAMEPOS: {
         PSWP  pswp = (PSWP)mp2;
         ULONG fl = SWP_MOVE | SWP_SIZE | SWP_SHOW;
         SWP   swp;
         WinQueryWindowPos (pMyWindowData->hwndOriginal, &swp);
         if ((swp.fl & SWP_HIDE) || (swp.cx <= 0) || (swp.cy <= 0))
            WinShowWindow (WinQueryWindow (hwnd, QW_PARENT), FALSE);
         else {
            WinSetWindowPos (WinQueryWindow (hwnd, QW_PARENT), 0,
                          swp.x / pSettings->lScale + pSettings->ptlMyDesktop.x, swp.y / pSettings->lScale + pSettings->ptlMyDesktop.y,
                          swp.cx / pSettings->lScale, swp.cy / pSettings->lScale, 
                          fl);
            }
         }
         return ((MRESULT)0);

      case WM_PAINT: {
         HPOINTER hIcon;
         RECTL    rclUpdate;
         WinBeginPaint (hwnd, pMyWindowData->hps, &rclUpdate);
         WinFillRect (pMyWindowData->hps, &rclUpdate, SYSCLR_WINDOW);
         hIcon = WinQueryWindowULong (pMyWindowData->hwndOriginal, QWL_HICON);
         if (hIcon) {
            SHORT  x, y, cx, cy, cxy;
            RECTL  rclSrc;
            WinQueryWindowRect (hwnd, &rclSrc);
            cx = rclSrc.xRight - rclSrc.xLeft;
            cy = rclSrc.yTop - rclSrc.yBottom;
            if ((cx > 20) && (cy > 20)) {
               cxy = 20;
               x = cx / 2 - cxy / 2;
               y = cy / 2 - cxy / 2;
               Win32StretchPointer (pMyWindowData->hps, x, y, cxy, cxy, hIcon, DP_NORMAL);
               }
            }
         WinEndPaint (pMyWindowData->hps);
         }
         return ((MRESULT)0);

      case WM_DESTROY:
         GpiAssociate (pMyWindowData->hps, NULLHANDLE);
         GpiDestroyPS (pMyWindowData->hps);
         free ((PVOID)pMyWindowData);
//         WinAlarm(HWND_DESKTOP, WA_ERROR);
         return ((MRESULT)0);

      default:
         break;
      }
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
   }

MRESULT EXPENTRY SubWindowsWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2){
   HWND hwndClient = WinWindowFromID(hwnd, FID_CLIENT);
   PMYDESKTOPINSTANCEDATA pMyDesktopData = WinQueryWindowPtr (hwndClient, QWL_USER);
   PSETTINGS pSettings    = WinQueryWindowPtr (hwndClient, QWL_USER + sizeof (PVOID));
   PMYWINDOWINSTANCEDATA pMyWindowData = WinQueryWindowPtr (hwndClient, QWL_USER + 2 * sizeof (PVOID));

   if((NULL != pMyDesktopData) && (NULL != pSettings) && (NULL != pMyWindowData)) {

         switch (msg) {

            case WM_ADJUSTWINDOWPOS:{
               PSWP     pswp    = (PSWP)mp1;
               ULONG    mresult = 0;
               if (!(pswp->fl & SWP_HIDE)) {
                  pswp->hwndInsertBehind = pMyWindowData->hwndInsertBehind;
                  pswp->fl              |= SWP_DEACTIVATE | SWP_ZORDER;
                  mresult                = AWP_DEACTIVATE;
                  }
               return ((MRESULT)mresult);
               }

            case WM_QUERYTRACKINFO: {
               PTRACKINFO ptrack;
               (*(pMyWindowData->pOldFrameWndProc))(hwnd, msg, mp1, mp2);
               ptrack = (PTRACKINFO)mp2;
               if (pMyWindowData->hwndOriginal == WinQueryWindow (pMyDesktopData->hwndMainClient, QW_PARENT)) {
                  ptrack->rclBoundary.yTop    = pSettings->ptlMyDesktop.y + pMyDesktopData->ptlScreen.y / pSettings->lScale;
                  ptrack->rclBoundary.yBottom = pSettings->ptlMyDesktop.y;
                  ptrack->rclBoundary.xLeft   = pSettings->ptlMyDesktop.x;
                  ptrack->rclBoundary.xRight  = pSettings->ptlMyDesktop.x + pMyDesktopData->ptlScreen.x / pSettings->lScale;
                  ptrack->fs |= TF_ALLINBOUNDARY;
                  }
               if (pSettings->lWindowGrid) {
                  ptrack->cxGrid = (pMyDesktopData->ptlScreen.x / pSettings->lScale) / pSettings->lWindowGrid;
                  ptrack->cyGrid = (pMyDesktopData->ptlScreen.y / pSettings->lScale) / pSettings->lWindowGrid;
                  ptrack->cxKeyboard = (pMyDesktopData->ptlScreen.x / pSettings->lScale) / pSettings->lWindowGrid;
                  ptrack->cyKeyboard = (pMyDesktopData->ptlScreen.y / pSettings->lScale) / pSettings->lWindowGrid;
                  ptrack->fs |= TF_GRID;
                  }
               ptrack->cxBorder = 1;
               ptrack->cyBorder = 1;
               }
               return((MRESULT)TRUE);

            }
      }
   return (*(pMyWindowData->pOldFrameWndProc))(hwnd, msg, mp1, mp2);
   } // End of SubWindowsWndProc

