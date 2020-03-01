         SWP swp;
         WinQueryWindowPos(pInstanceData->hwndOriginal, &swp);
         if (!(swp.fl & SWP_HIDE)) {
//            hwndInsertBehind = FindMyWindow (pInstanceData->hwndMainClient, swp.hwndInsertBehind);
//            if (hwndInsertBehind == NULLHANDLE)
//               hwndInsertBehind = HWND_TOP;
            WinSetWindowPos (pInstanceData->hwndFrame, pInstanceData->hwndInsertBehind,
                          swp.x / pSettings->lScale + pSettings->ptlMyDesktop.x, swp.y / pSettings->lScale + pSettings->ptlMyDesktop.y,
                          swp.cx / pSettings->lScale, swp.cy / pSettings->lScale, 
                          fl); // | SWP_ZORDER
            }
//         WinBroadcastMsg (pInstanceData->hwndMainClient, UM_QUERYINSERTBEHIND, MPVOID, MPVOID, BMSG_POST);

      case UM_QUERYINSERTBEHIND: {
         SWP   swp;
         WinQueryWindowPos(pInstanceData->hwndOriginal, &swp);
         pInstanceData->hwndInsertBehind = FindMyWindow (pInstanceData->hwndMainClient, swp.hwndInsertBehind);
         if (pInstanceData->hwndInsertBehind == NULLHANDLE)
            pInstanceData->hwndInsertBehind = HWND_TOP;
         WinSetWindowPos (pInstanceData->hwndFrame, pInstanceData->hwndInsertBehind,
                          0, 0,
                          0, 0, 
                          SWP_ZORDER);
         }
         return ((MRESULT)0);

      case UM_ADJUSTFRAMEPOS: {
         PSWP  pswp = (PSWP)mp2;
         ULONG fl = SWP_MOVE | SWP_SIZE | SWP_SHOW;
         SWP   swp;
         WinQueryWindowPos(pInstanceData->hwndOriginal, &swp);
         if (!(swp.fl & SWP_HIDE)) {
            WinSetWindowPos (pInstanceData->hwndFrame, pInstanceData->hwndInsertBehind,
                          swp.x / pSettings->lScale + pSettings->ptlMyDesktop.x, swp.y / pSettings->lScale + pSettings->ptlMyDesktop.y,
                          swp.cx / pSettings->lScale, swp.cy / pSettings->lScale, 
                          fl); // | SWP_ZORDER
            }
         else
            WinShowWindow (pInstanceData->hwndFrame, FALSE);
         }
         return ((MRESULT)0);


                  SWP   swpOriginal, swpMy;
                  swpOriginal.hwndInsertBehind = pInstanceData->hwndOriginal;
                  do {
                     WinQueryWindowPos(swpOriginal.hwndInsertBehind, &swpOriginal);
                     if (swpOriginal.hwndInsertBehind == NULLHANDLE) {
                        pswp->hwndInsertBehind = HWND_TOP;
                        break;
                        }
                     pswp->hwndInsertBehind = FindMyWindow (pInstanceData->hwndMainClient, swpOriginal.hwndInsertBehind);
                     if (pswp->hwndInsertBehind == NULLHANDLE) {
                        pswp->hwndInsertBehind = HWND_TOP;
                        break;
                        }
                     WinQueryWindowPos(pswp->hwndInsertBehind, &swpMy);
                     } while (swpMy.fl & SWP_HIDE);


      case UM_ADJUSTFRAMEPOS: {
         HWND hwndOriginal = HWNDFROMMP(mp1);
         HWND hwndMyWindow = FindMyWindow (hwnd, hwndOriginal);
         PSWP pswp = (PSWP)mp2;
         if (hwndMyWindow) {
            WinPostMsg (hwndMyWindow, UM_ADJUSTFRAMEPOS, mp1, mp2);
            if (pswp->fl & SWP_ZORDER)
               WinBroadcastMsg (hwnd, UM_QUERYINSERTBEHIND, MPVOID, MPVOID, BMSG_POST);
            }
         else {
            if (!(pswp->fl & SWP_HIDE)){
               CHAR   szTitle[64], szWindowTitle[8];
               HWND   hwndMyWindow, hwndMyWindowsClient;
               ULONG  flFrameFlags = FCF_BORDER | FCF_NOBYTEALIGN;
               ULONG  ulClr = SYSCLR_WINDOW;
               WinLoadString (pInstanceData->hab, NULLHANDLE, SZ_MYWINDOWS, sizeof (szTitle), szTitle);
               WinQueryWindowText(HWNDFROMMP(mp1), sizeof(szWindowTitle), szWindowTitle);
               if (!(strstr (szWindowTitle, "Desktop"))) {
                  hwndMyWindow = WinCreateStdWindow (hwnd, 0L, &flFrameFlags, (PSZ)szTitle, NULL, 0L, NULLHANDLE, 0L, &hwndMyWindowsClient);
                  WinSetPresParam (hwndMyWindow, PP_BACKGROUNDCOLOR, sizeof (ulClr), &ulClr);
                  WinPostMsg (hwndMyWindow, UM_INITWINDOW, MPFROMHWND(hwnd), mp1);
                  WinBroadcastMsg (hwnd, UM_QUERYINSERTBEHIND, MPVOID, MPVOID, BMSG_POST);
                  }
               }
            }
         }
         return ((MRESULT)0);


   hEnumMyWindows = WinBeginEnumWindows (pMyDesktopData->hwndMainClient);
   while (hwndNextMyWindow = WinGetNextWindow (hEnumMyWindows)) {
      WinQueryClassName(hwndNextMyWindow, sizeof(szClassName), szClassName);
      if (strcmp (szClassName, "#1"))
         continue;
      WinQueryClassName(WinWindowFromID(hwndNextMyWindow, FID_CLIENT), sizeof(szClassName), szClassName);
      if (strcmp (szClassName, pMyDesktopData->szMyWindowClass))
         continue;
      pMyWindowData = WinQueryWindowPtr (WinWindowFromID(hwndNextMyWindow, FID_CLIENT), QWL_USER + 2 * sizeof (PVOID));
      if (WinQueryWindow (pMyWindowData->hwndOriginal, QW_FRAMEOWNER))
         continue;
      else {
         if (!pMyWindowData->flSticky) {
            WinQueryWindowPos (pMyWindowData->hwndOriginal, pswp);
            if (!(pswp->fl & SWP_MINIMIZE)) {
               if (!(pswp->fl & SWP_HIDE)) {
                  pswp->x += lPosX;
                  pswp->y += lPosY;
                  pswp->fl = (SWP_NOADJUST | SWP_MOVE);
                  pswp++;
                  }
               }
            }
         hEnumChildren = WinBeginEnumWindows (HWND_DESKTOP);
         while (hwndNextChild = WinGetNextWindow (hEnumChildren)) {
            if ((WinQueryWindow (hwndNextChild, QW_PARENT)) != pMyDesktopData->hwndDesktop)
               continue;
            WinQueryClassName (hwndNextChild, sizeof(szClassName), szClassName);
            if (!(strcmp (szClassName, "#32765")))
               continue; // ignore icon title
            if (!(strcmp (szClassName, "#4")))
               continue; // ignore menu
            WinQueryWindowText (hwndNextChild, sizeof(szWindowTitle), szWindowTitle);
            if (!(strcmp (szWindowTitle, "Desktop")))
               continue; // ignore Desktop window
            if (!(strcmp (szWindowTitle, "WIN-OS/2 window: session")))
               continue; // ignore "WIN-OS/2 window: session"
            if (!(strcmp (szWindowTitle, "Seamless Window")))
               continue; // ignore "Seamless Window"
            if ((WinQueryWindow (hwndNextChild, QW_OWNER)) != pMyWindowData->hwndOriginal)
               continue;
            WinQueryWindowPos (hwndNext, pswp2);
            if (pswp2->fl & SWP_HIDE)
               continue;
            if (pswp2->fl & SWP_MINIMIZE)
               continue;
            pswp->x += lPosX;
            pswp->y += lPosY;
            pswp->fl = (SWP_NOADJUST | SWP_MOVE);
            pswp++;
            }
         }
      } 
   WinEndEnumWindows (hEnumMyWindows);


