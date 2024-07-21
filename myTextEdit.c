#include "main.h"
#include "globals.h"
#include "myScrollBars.h"
#include "myTextEdit.h"

TEClickLoopUPP  gSavedClickLoopUPP = NULL;

/* Setup the TextEditControl and enable word wrap. */
void SetupTE(WindowPtr winPtr) {
    Rect            destRect;
    Rect            viewRect;
    DocumentPeek    docPtr;
    TEPtr           tePtr;
    
    SetPort(winPtr);
    TextFont(applFont);
    TextSize(9);    
    GetTERect(winPtr, &viewRect);
    destRect = viewRect;
    docPtr = (DocumentPeek)winPtr;
    
    docPtr->teHndl = TENew(&destRect, &viewRect);
    tePtr = *docPtr->teHndl;
    
    /* Enable word wrap */
    tePtr->crOnly = +1;
    
    /* ClickLoop is used to move the thumb in the scrollbar when
       highlighting text outside of the current view. */
    gSavedClickLoopUPP = tePtr->clickLoop;
    tePtr->clickLoop = NewTEClickLoopUPP(ClickLoop);
    TEScroll(0, 0, docPtr->teHndl);
    TEUpdate(&tePtr->viewRect, docPtr->teHndl);
    TEActivate(docPtr->teHndl);
}

    
/* All this does is automatically redraw the scrollbar thumb */   
pascal Boolean ClickLoop() {
    RgnHandle       savedClip;
    WindowPtr       winPtr;
    DocumentPeek    docPtr;
    GrafPtr         savedPort;
    TEPtr           tePtr;
    Rect            *viewRect, *destRect;
    Point           mousePt;
    short           scrollAmount = 0;
    short           viewWidth, lineHeight, destBottom;
    
    /* Call the saved ClickLoop function */
#ifdef __POWERPC__
    CallUniversalProc(gSavedClickLoopUPP, kPascalStackBased);
#endif

#ifdef __MC68K__
    gSavedClickLoopUPP();
#endif
    
    /* Get the window's graphics port */
    winPtr = FrontWindow();
    GetPort(&savedPort);
    SetPort(winPtr);
    docPtr = (DocumentPeek)winPtr;
    HLock((Handle)docPtr->teHndl);
    tePtr = *docPtr->teHndl;
    
    /* Save the old clipping region */
    savedClip = NewRgn();
    GetClip(savedClip);
    
    /* Set a new clipping region to the entire window port */  
    ClipRect(&winPtr->portRect);
    
    viewRect = &tePtr->viewRect;
    destRect = &tePtr->destRect;
    lineHeight = tePtr->lineHeight;
    viewWidth = viewRect->right - viewRect->left;
    
    GetMouse(&mousePt);
    
    if (!PtInRect(mousePt, viewRect)) {
        if (mousePt.v > viewRect->bottom) {
            scrollAmount = -1;
            ScrollbarAction(docPtr->vScrollHndl, &scrollAmount);
            destBottom = destRect->top + tePtr->nLines * lineHeight;
            
            if (viewRect->bottom < destBottom + kScrollbarAdjust) {            
                /* Small delay so scrolling isn't so fast */
                Delay(kClickLoopScrollDelay, NULL);
                TEScroll(0, scrollAmount * tePtr->lineHeight, docPtr->teHndl);
            }
        } else if ((mousePt.v < viewRect->top) && (viewRect->top > destRect->top)) {
            scrollAmount = 1;
            ScrollbarAction(docPtr->vScrollHndl, &scrollAmount);
            
            /* Small delay so scrolling isn't so fast */
            Delay(kClickLoopScrollDelay, NULL);
            TEScroll(0, scrollAmount * tePtr->lineHeight, docPtr->teHndl);
        }
    }
    
    /* Restore the saved clipping region */
    SetClip(savedClip);
    DisposeRgn(savedClip);
    SetPort(savedPort);    
    HUnlock((Handle)docPtr->teHndl);
    
    return true;
}

/* Change the mouse to an iBeam when it's over the TextEdit control. */
void ChangeMouse(TEHandle teHndl) {
    Point       mousePt;
    CursHandle  iBeam;
    
    if (FrontWindow() == (WindowPtr)((*teHndl)->inPort)) {
        GetMouse(&mousePt);
        
        if (PtInRect(mousePt, &(*teHndl)->viewRect)) {
            iBeam = GetCursor(iBeamCursor);
            SetCursor(*iBeam);
        } else {
            SetCursor(&qd.arrow);
        }
    }
}

/* Return a rectangle that is inset from the portRect by the size of
   the scrollbars and a little extra margin. */
void GetTERect(WindowPtr winPtr, Rect *teRect) {
	*teRect = winPtr->portRect;
	
	/* adjust for margin */
	InsetRect(teRect, kTextMargin, kTextMargin);
	
	/* and for the scrollbars */
	teRect->bottom = teRect->bottom - kScrollbarAdjust;
	teRect->right = teRect->right - kScrollbarAdjust;
}

/* Update the TERec's view rect so that it is the greatest multiple of
   the lineHeight that still fits in the old viewRect. */
void AdjustViewRect(TEHandle teHndl) {
	TEPtr		tePtr;
	
	tePtr = *teHndl;
	tePtr->viewRect.bottom = (((tePtr->viewRect.bottom - tePtr->viewRect.top) / tePtr->lineHeight)
							* tePtr->lineHeight) + tePtr->viewRect.top;
}

/* Scroll the TERec around to match up to the potentially updated scrollbar
   values. This is really useful when the window has been resized such that the
   scrollbars became inactive but the TERec was already scrolled. */
void AdjustTE(WindowPtr winPtr) {
    DocumentPeek    docPtr;
	TEPtr		    tePtr;
	
	docPtr = (DocumentPeek)winPtr;
	tePtr = *docPtr->teHndl;
	
	TEScroll((tePtr->viewRect.left - tePtr->destRect.left) -
			GetControlValue(docPtr->hScrollHndl),
			(tePtr->viewRect.top - tePtr->destRect.top) -
				(GetControlValue(docPtr->vScrollHndl) *
				tePtr->lineHeight),
			docPtr->teHndl);
}

/* When the window is resized, recalculate the TextEdit fields.
  This needs to be called before updating the scrollbars. */
void UpdateTEWordWrap(WindowPtr winPtr) {
    DocumentPeek    docPtr;
	TEPtr		    tePtr;

    docPtr = (DocumentPeek)winPtr;
	tePtr = *docPtr->teHndl;
	GetTERect(winPtr, &tePtr->viewRect);
	
	/* Only update the width of the destRect
	   since that's all word wrapping cares about. */
	tePtr->destRect.right = tePtr->viewRect.right;	
	TECalText(docPtr->teHndl);
}