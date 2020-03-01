/**************************************************************************
 *  File name  : dialogs.c
 *  (c) Copyright Carrick von Schoultz 1994. All rights reserved.
 *  (c) Copyright Peter Nielsen 1994. All rights reserved.
 *  Description: 
 *************************************************************************/
#define  INCL_WINBUTTONS
#define  INCL_WINDIALOGS
#define  INCL_WINFRAMEMGR
#define  INCL_WINHELP
#define  INCL_WININPUT
#define  INCL_WINMENUS
#define  INCL_WINSTDSPIN
#define  INCL_WINSYS
#define  INCL_WINWINDOWMGR
#include <os2.h>
#include <string.h>
#include "pmvd.h"
#include "dialogs.h"

USHORT ausMoveCloseMenu[] = { 2, SC_MOVE, SC_CLOSE };

VOID SetSysMenu (HWND hwnd, PUSHORT ausItem) {
   MENUITEM mi;
   ULONG    i, ulPos = 0;
   USHORT   usItems, usLastItem, usCount = *ausItem++;

   WinSendDlgItemMsg (hwnd, FID_SYSMENU, MM_QUERYITEM, MPFROM2SHORT (SC_SYSMENU, FALSE), MPFROMP (&mi));
   usItems = SHORT1FROMMR (WinSendMsg (mi.hwndSubMenu, MM_QUERYITEMCOUNT, NULL, NULL));
   while (usItems--) {
      USHORT usItem = SHORT1FROMMR (WinSendMsg (mi.hwndSubMenu, MM_ITEMIDFROMPOSITION, MPFROMLONG (ulPos), NULL));
      for (i = 0; usItem != ausItem[i] && i < usCount; i++);
      if (usItem == ausItem[i] || (usItem > 0x8100 && usLastItem <= 0x8100)) {
         usLastItem = usItem;
         ulPos++;
         }
      else
         WinSendMsg (mi.hwndSubMenu, MM_DELETEITEM, MPFROM2SHORT (usItem, TRUE), NULL);
      }
   if (usLastItem > 0x8100)
      WinSendMsg (mi.hwndSubMenu, MM_DELETEITEM, MPFROM2SHORT (usLastItem, TRUE), NULL);
   }

MRESULT EXPENTRY About2DlgProc (HWND hwnd, LONG msg, MPARAM mp1, MPARAM mp2) {
   switch(msg) {
      case WM_INITDLG:
         SetSysMenu (hwnd, ausMoveCloseMenu);
         return ((MRESULT)FALSE);

      case WM_COMMAND:
         WinDismissDlg (hwnd, TRUE);
         break;
      }
   return (WinDefDlgProc(hwnd, msg, mp1, mp2));
   }

MRESULT EXPENTRY ProductInfoDlgProc (HWND hwnd, LONG msg, MPARAM mp1, MPARAM mp2) {
   switch(msg) {
      case WM_INITDLG:
         {CHAR  szVersion[80];
         SetSysMenu (hwnd, ausMoveCloseMenu);
         WinLoadString (WinQueryAnchorBlock (hwnd), NULLHANDLE, SZ_VERSION, sizeof (szVersion), szVersion);
         WinSetDlgItemText(hwnd, IDC_VERSION, szVersion);
         }return ((MRESULT)FALSE);

      case WM_COMMAND:
         switch (SHORT1FROMMP(mp1)) {
            case DID_OK:
               WinDismissDlg (hwnd, TRUE);
               return ((MRESULT)0);

            case IDC_ICON:
               WinDlgBox (HWND_DESKTOP, hwnd, (PFNWP)About2DlgProc, 0, ID_ABOUT2, (PVOID)NULL);
               return ((MRESULT)0);
            }
         break;
      }
   return (WinDefDlgProc(hwnd, msg, mp1, mp2));
   }

VOID HelpProductInfo (HWND hwnd) {
   WinDlgBox (HWND_DESKTOP, hwnd, (PFNWP)ProductInfoDlgProc, 0, ID_PRODUCTINFO, (PVOID)NULL);
   }

