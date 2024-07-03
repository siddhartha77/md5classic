#include "main.h"
#include "globals.h"
#include "myScrollBars.h"
#include "myTextEdit.h"

TEHandle        gTEHndl = NULL;

TEHandle SetupTE(WindowPtr winPtr) {
    Rect            destRect;
    Rect            viewRect;
    
    SetPort(winPtr);
    TextFont(applFont);
    TextSize(9);
    
    /* Get TextEdit bounds and account for scrollbars */
    viewRect = winPtr->portRect;
    viewRect.top += kScrollBarHeight;
    viewRect.right -= kScrollBarWidth;
    viewRect.bottom -= kScrollBarHeight;
    viewRect.left += kScrollBarWidth;
    BlockMove(&viewRect, &destRect, sizeof(Rect));    
    gTEHndl = TENew(&destRect, &viewRect);
    SetWRefCon(winPtr, (long)gTEHndl);
    (*gTEHndl)->clickLoop = NewTEClickLoopUPP(ClickLoop);
    TEScroll(0, 0, gTEHndl);
    TEUpdate(&winPtr->portRect, gTEHndl);
    TEActivate(gTEHndl);
    
    return gTEHndl;
}

pascal Boolean ClickLoop() {
    RgnHandle   oldClip;
    WindowPtr   winPtr;
    GrafPtr     oldPort;
    TEPtr       textPtr;
    Rect        tempRect;
    Rect        *viewRect, *destRect;
    Point       mousePt;
    short       scrollAmount = 0;
    short       viewWidth, lineHeight, destBottom;
    
    HLock((Handle)gTEHndl);
    textPtr = *gTEHndl;
    
    /* All this does is automatically redraw the scroll bar thumb */
    winPtr = FrontWindow();
    GetPort(&oldPort);
    SetPort(winPtr);
    oldClip = NewRgn();
    GetClip(oldClip);
    SetRect(&tempRect, -32767, -32767, 32767, 32767);
    ClipRect(&tempRect);
    
    viewRect = &textPtr->viewRect;
    destRect = &textPtr->destRect;
    lineHeight = textPtr->lineHeight;
    viewWidth = viewRect->right - viewRect->left;
    
    GetMouse(&mousePt);
    
    if (!PtInRect(mousePt, viewRect)) {
        if (mousePt.v > winPtr->portRect.bottom) {
            scrollAmount = -1;
            SetScrollBarValue(vScrollHndl, &scrollAmount);            
            destBottom = destRect->top + textPtr->nLines * lineHeight;
            if (viewRect->bottom < destBottom + kScrollBarHeight) TEScroll(0, -1, gTEHndl);
        } else if ((mousePt.v < viewRect->top) && (viewRect->top > destRect->top)) {
            scrollAmount = 1;
            SetScrollBarValue(vScrollHndl, &scrollAmount);            
            TEScroll(0, 1, gTEHndl);
        }
    }
    
    SetClip(oldClip);
    DisposeRgn(oldClip);
    SetPort(oldPort);
    
    textPtr->clickTime = TickCount();    
    HUnlock((Handle)gTEHndl);
    
    return true;
}

void ChangeMouse(TEHandle textHndl) {
    Point       mousePt;
    CursHandle  iBeam;
    
    if (FrontWindow() == (WindowPtr)((*textHndl)->inPort)) {
        GetMouse(&mousePt);
        
        if (PtInRect(mousePt, &(*textHndl)->viewRect)) {
            iBeam = GetCursor(iBeamCursor);
            SetCursor(*iBeam);
        } else {
            SetCursor(&qd.arrow);
        }
    }
}

void ScrollContents(TEHandle textHndl, short dh, short dv) {
    RgnHandle   updateRgn;
    
    updateRgn = NewRgn();
    TEScroll(dh, dv, textHndl);
    ScrollRect(&lowerRect, dh, dv, updateRgn);
    DisposeRgn(updateRgn);    
    UpdateText(textHndl);
}

void UpdateText(TEHandle textHndl) {
    RgnHandle   updateRgn;
    
    HLock((Handle)textHndl);
    updateRgn = NewRgn();
    TEUpdate(&(**textHndl).viewRect, textHndl);
    DrawControls((WindowPtr)((*textHndl)->inPort));
    DisposeRgn(updateRgn);  
    HUnlock((Handle)textHndl); 
}