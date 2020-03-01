UpdateMyWindows (PMAINWINDOWINSTANCEDATA pInstanceData, PSETTINGS pSettings) {
   CHAR      szWindowTitle[8], szMyWindowClass[64];
   HWND      hwndNext, hwndMyWindow, hwndInsertBehind;
   PSWP      pswp, pswpBase;
   SWP       swpOriginal;

   pswpBase         = (PSWP)calloc (2500, sizeof (SWP));
   pswp             = pswpBase;
   hwndInsertBehind = HWND_TOP;

   hEnum = WinBeginEnumWindows (pInstanceData->hwndDesktop);
   while (hwndNext = WinGetNextWindow (hEnum)) {
      WinQueryClassName (hwndNextFrame, sizeof(szClassName), szClassName);
      if (!(strcmp (szClassName, "#32765"))) // ignore Icon text windows
         continue;
      WinQueryWindowText (hwndNext, sizeof(szWindowTitle), szWindowTitle);
      if (strstr (szWindowTitle, "Desktop")) // ignore Dessktop window
         continue;
      WinQueryWindowPos (hwndNext, &swpOriginal);
      if (swpOriginal.fl & SWP_HIDE)
         continue;
      hwndMyWindow = FindMyWindow (pInstanceData->hwndClient, hwndNext);
      if (hwndMyWindow == NULLHANDLE) 
         hwndMyWindow = CreateMyWindow (pInstanceData->hab, pInstanceData->hwndClient, pInstanceData->szMyWindowClass, pInstanceData->ptlScreen, pSettings);
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
   WinSetMultWindowPos (pInstanceData->hab, pswpBase, (((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP)));
   free ((PVOID)pswpBase);
   }



   hwndNext      = WinQueryWindow (HWND_DESKTOP, QW_TOP);
   hwndParent = WinQueryWindow (hwndNext, QW_PARENT);
   while ((hwndParent != hwndDesktop) && (hwndNext != NULLHANDLE)) {
      hwndNext = WinQueryWindow (hwndNext, QW_NEXT);
      hwndParent = WinQueryWindow (hwndNext, QW_PARENT);
      }
   hwndNextFrame = hwndNext;

   HWND      hwndMain, hwndOwner, hwndTop; 
   BOOL      fNotMoved;
   PSWP      pswp, pswpBase, pswp2, pswp2Base;
   hwndTop = hwndMain = WinQueryWindow(hwndMainClient, QW_PARENT);

   while (((hwndTop = WinQueryWindow(hwndTop, QW_NEXTTOP)) != hwndMain)) {
      fNotMoved = TRUE;
      if (WinQueryWindow(hwndTop, QW_FRAMEOWNER) == NULLHANDLE) {
         PSWP pswpTmp = pswpBase;
         while (pswpTmp < pswp){
            if (hwndTop == pswpTmp++->hwnd){
               fNotMoved = FALSE;
               break;
               }
            }
         if (fNotMoved == FALSE)
            continue;
         WinQueryWindowPos(hwndTop, pswp);
         if (!(pswp->fl & SWP_MINIMIZE)) {
            pswp->x += lPosX;
            pswp->y += lPosY;
            pswp->fl = (SWP_NOADJUST | SWP_MOVE);
            pswp++;
            }
         } 
      else {
         if (WinQueryWindow (hwndTop, QW_FRAMEOWNER) == hwndMain)
            continue;
         WinQueryWindowPos(hwndTop, pswp2++);
         hwndOwner = hwndTop;
         while ((hwndOwner = WinQueryWindow(hwndOwner, QW_FRAMEOWNER)) != NULLHANDLE) {
            if (hwndOwner != hwndMain){
               if (WinQueryWindow(hwndOwner, QW_FRAMEOWNER) == NULLHANDLE){
                  PSWP pswpTmp;   
                  for (pswpTmp = pswpBase; pswpTmp < pswp; pswpTmp++){
                     if (hwndOwner == pswpTmp->hwnd){
                        fNotMoved = FALSE;
                        break;
                        }
                     }
                  if (fNotMoved == FALSE)
                     continue;
                  WinQueryWindowPos(hwndOwner, pswp);
                  if (!(pswp->fl & SWP_MINIMIZE)) {
                     pswp->x += lPosX;
                     pswp->y += lPosY;
                     pswp->fl = (SWP_NOADJUST | SWP_MOVE);
                     pswp++;
                     }
                  }
               else {
                  WinQueryWindowPos(hwndOwner, pswp2++);
                  }
               }
            }
         }
      }

   WinSetMultWindowPos(hab, pswpBase, (((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP)) - 1);

   if (pswp2 > pswp2Base){
      PSWP pswp2Tmp;
      for (pswp2Tmp = pswp2Base; pswp2Tmp < pswp2; pswp2Tmp++){
//         if (WinQueryWindow(pswp2Tmp->hwnd, QW_FRAMEOWNER) != NULLHANDLE){
            pswp2Tmp->x += lPosX;
            pswp2Tmp->y += lPosY;
            pswp2Tmp->fl = (SWP_NOADJUST | SWP_MOVE);
//            }
         }
      WinSetMultWindowPos(hab, pswp2Base, ((ULONG)pswp2 - (ULONG)pswp2Base) / sizeof (SWP));
      }

   free ((PVOID)pswpBase);
   free ((PVOID)pswp2Base);
   } // end SetDesktopPos

   }