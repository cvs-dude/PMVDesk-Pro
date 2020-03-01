/**************************************************************************
 *  File name  :  xtrn.h
 *  ¸ Copyright Carrick von Schoultz 1994. All rights reserved.
 *  Description:  This header file contains the application-wide function
 *                declarations
 *************************************************************************/

/* from pmvd.c */
VOID EnableMenuItem(HWND hwndMenu, USHORT idItem, BOOL flEnable);
VOID CheckMenuItem(HWND hwndMenu, SHORT idItem, BOOL flEnable);

/* from dialog.c */
VOID HelpProductInfo (HWND hwndClient);

/* from desktop.c */
VOID  MyDesktopThread (HWND hwnd);

/* from windows.c */
VOID Scale (HAB hab, HWND hwndParent, PSETTINGS pSettings, POINTL ptlScreen, CHAR szMyWindowClass[64]);
