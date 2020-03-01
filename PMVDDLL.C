/****************************************************************************/
/* PMVD DLL : PMVDDLL.C
/* ¸ Copyright Carrick von Schoultz 1994. All rights reserved.
/****************************************************************************/

#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "pmvddll.h"
#include "pmvd.h"

/* This is the global data segment that is shared by every process. */
#pragma data_seg (GLOBAL_SEG)

static HAB      habOwner;    // owner application anchor block handle
static HMODULE  hModuleDLL;  // DLL module handle
static HWND     hwndOwner;   // owner application window handle
static HWND     hwndDesktop;
static BOOL     fInputHookActive;
static BOOL     fSendMessageHookActive;

int _CRT_init(void);

void _CRT_term(void);

unsigned long _System _DLL_InitTerm(HMODULE hModule, unsigned long ulFlag){

   switch (ulFlag) {
      case 0 :
         if (_CRT_init() == -1)
            return 0UL;
         hModuleDLL = hModule;           /* save handle for later use */
         fSendMessageHookActive = FALSE; /* hook not active */
         fInputHookActive = FALSE;       /* hook not active */
         break;

      case 1 :
         /* call ReleaseHook to be sure hooks are released before termination */
         ReleaseHook();
         _CRT_term();
         break;

      default  :
         return 0UL;
      }
   return 1UL;
   } // End of automatic DLL initialization code.

BOOL EXPENTRY MyInputHookProc(HAB hab, PQMSG pQmsg, ULONG ulRemove) {

//   switch(pQmsg->msg) {
//      case WM_CHAR: // check if <SHIFT> and <CTRL> keys are down
//         if(CHARMSG(&pQmsg->msg)->fs & KC_KEYUP) break;
//         if(!(CHARMSG(&pQmsg->msg)->fs & KC_ALT)) break;
//         if(!(CHARMSG(&pQmsg->msg)->fs & KC_SHIFT)) break;
//         if(CHARMSG(&pQmsg->msg)->fs & KC_CHAR){
//         switch ((CHAR)(CHARMSG(&pQmsg->msg)->chr)){
//            case 'a':
//            case 'A':
//            WinAlarm(HWND_DESKTOP, WA_ERROR);
//            return(TRUE); /* Do not pass message */
//            }
//            }
//         break;
//      }
   return(FALSE); /* Pass message where it belongs */
   } // MyInputHookProc

BOOL EXPENTRY MySendMsgHookProc(HAB hab, PSMHSTRUCT psmh, BOOL fInterTask) {
   CHAR szClassName[20];

   switch(psmh->msg) {
/*
      case WM_CREATE:
         if (WinQueryWindow (psmh->hwnd, QW_PARENT) == hwndDesktop) {
            WinQueryClassName(psmh->hwnd, sizeof(szClassName), szClassName);
            if (!strcmp(szClassName, "#1")){ // only interested in frame window messages
               WinPostMsg (hwndOwner, UM_CREATE, MPFROMHWND(psmh->hwnd), MPFROMP(NULL));
               }
            }
         break;
*/
      case WM_DESTROY:
         if (WinQueryWindow (psmh->hwnd, QW_PARENT) == hwndDesktop)
            WinPostMsg (hwndOwner, UM_DESTROY, MPFROMHWND(psmh->hwnd), MPFROMP(NULL));
         break;

      case WM_ADJUSTFRAMEPOS:
         if (WinQueryWindow (psmh->hwnd, QW_PARENT) == hwndDesktop) {
            WinQueryClassName(psmh->hwnd, sizeof(szClassName), szClassName);
            if (strcmp(szClassName, "#32765"))
               WinPostMsg (hwndOwner, UM_ADJUSTFRAMEPOS, MPFROMHWND(psmh->hwnd), psmh->mp1);
//            WinAlarm(HWND_DESKTOP, WA_ERROR);
            }
         break;
      }
   return(FALSE); // Pass message where it belongs
   } // MySendMsgHookProc

BOOL InstallHook(HAB hab, HWND hwnd, HWND hwndMainFrame){
   if (!fSendMessageHookActive) {
      fSendMessageHookActive = WinSetHook(hab, NULLHANDLE, HK_SENDMSG, (PFN)MySendMsgHookProc, hModuleDLL);
      habOwner = hab;
      hwndOwner = hwnd;
//      hwndDesktop = WinQueryDesktopWindow (hab, NULLHANDLE);
      hwndDesktop = WinQueryWindow (hwndMainFrame, QW_PARENT);
      }
   if (!fInputHookActive)
      fInputHookActive = WinSetHook(hab, NULLHANDLE, HK_INPUT, (PFN)MyInputHookProc, hModuleDLL);
   return (fSendMessageHookActive & fInputHookActive);
   } // InstallHook
#pragma handler(InstallHook)

BOOL ReleaseHook (VOID) {
   if (fSendMessageHookActive) {
      fSendMessageHookActive ^= WinReleaseHook(habOwner, NULLHANDLE, HK_SENDMSG,
      (PFN)MySendMsgHookProc, hModuleDLL);
      }
   if (fInputHookActive) {
      fInputHookActive ^= WinReleaseHook(habOwner, NULLHANDLE, HK_INPUT,
      (PFN)MyInputHookProc, hModuleDLL);
      }
   return ((BOOL)(!fSendMessageHookActive) & (BOOL)(!fInputHookActive));
   } // ReleaseHook
#pragma handler(ReleaseHook)
