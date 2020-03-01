VOID SetDesktopPos (HAB hab, HWND hwndMainClient, LONG lPosX, LONG lPosY) {
   PSWP    pswp, pswpBase, pswp2, pswp2Base;
   HWND    hwndMain, hwndOwner, hwndTop; 
   BOOL    fNotMoved;
   LONG    lCount;

   pswpBase = (PSWP)calloc (2500, sizeof (SWP));
   pswp2Base = (PSWP)calloc (2500, sizeof (SWP));

   pswp = pswpBase;
   pswp2 = pswp2Base;
   
   hwndTop = hwndMain = WinQueryWindow(hwndMainClient, QW_PARENT);
   WinSetActiveWindow (HWND_DESKTOP, hwndMain);

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
   lCount = (((ULONG)pswp - (ULONG)pswpBase) / sizeof (SWP)) - 1;
   if (lCount > 0)
      WinSetMultWindowPos (hab, pswpBase, lCount);

   if (pswp2 > pswp2Base){
      PSWP pswp2Tmp;
      for (pswp2Tmp = pswp2Base; pswp2Tmp < pswp2; pswp2Tmp++){
//         if (WinQueryWindow(pswp2Tmp->hwnd, QW_FRAMEOWNER) != NULLHANDLE){
            pswp2Tmp->x += lPosX;
            pswp2Tmp->y += lPosY;
            pswp2Tmp->fl = (SWP_NOADJUST | SWP_MOVE);
//            }
         }
      WinSetMultWindowPos (hab, pswp2Base, ((ULONG)pswp2 - (ULONG)pswp2Base) / sizeof (SWP));
      }

   free ((PVOID)pswpBase);
   free ((PVOID)pswp2Base);
   } // end SetDesktopPos
