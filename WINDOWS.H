/**************************************************************************
 *  File name  :  Windows.h
 *  ¸ Copyright Carrick von Schoultz 1994. All rights reserved.
 *  Description:  
 *************************************************************************/

typedef struct {
   BOOL              flSticky;
   HDC               hdc;
   HPS               hps;
   HWND              hwndOriginal, hwndInsertBehind;
   PFNWP             pOldFrameWndProc;
   } MYWINDOWINSTANCEDATA, *PMYWINDOWINSTANCEDATA;

typedef struct {
   CHAR              szMyWindowClass[20];
   HAB               hab;
   HDC               hdc;
   HPS               hps;
   HWND              hwndMyDesktopPopupMenu, hwndMyWindowsPopupMenu;
   HWND              hwndMainFrame, hwndMainClient, hwndDesktop;
   PFNWP             pOldFrameWndProc;
   POINTL            ptlScreen;
   SHORT             sTimer;
   } MYDESKTOPINSTANCEDATA, *PMYDESKTOPINSTANCEDATA;

MRESULT EXPENTRY MyWindowsWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND CreateMyWindow (HWND hwndOriginal, HWND hwndInsertBehind, PMYDESKTOPINSTANCEDATA pInstanceData, PSETTINGS pSettings);
HWND FindMyWindow (HWND hwndParent, HWND hwndOriginal, PMYDESKTOPINSTANCEDATA pMyDesktopData);

