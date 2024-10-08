#include "myScrollBars.h"
#include "myTextEdit.h"
#include "globals.h"

/* This was a major pain in the ass. Most of this code was lifted from some old "MacFS" code I found at x.org. */

/* Create the scroll bars. Note that the horizontal scroll bar is never enabled
   since the window word wraps. */
void SetupScrollBars(WindowPtr winPtr) {
    Rect            vScrollBoundsRect;
    Rect            hScrollBoundsRect;
    DocumentPeek    docPtr;
    
    SetRect(&vScrollBoundsRect,
        winPtr->portRect.right - kScrollbarAdjust,
        winPtr->portRect.top - 1,
        winPtr->portRect.right + 1,
        winPtr->portRect.bottom - (kScrollbarAdjust - 1)
    );
    
    /* Since we aren't using the horizontal scrollbar,
       set its top to just out of view. */
    SetRect(&hScrollBoundsRect,
        winPtr->portRect.left - 1,
        winPtr->portRect.bottom + kScrollbarAdjust,
        winPtr->portRect.right - (kScrollbarAdjust - 1),
        winPtr->portRect.bottom + 1
    );
    
    docPtr = (DocumentPeek)winPtr;    
    docPtr->vScrollHndl = NewControl(winPtr, &vScrollBoundsRect, "\p", true, 0, 0, 0, scrollBarProc, kVertical);
    
    /* This control is always hidden */
    docPtr->hScrollHndl = NewControl(winPtr, &hScrollBoundsRect, "\p", false, 0, 0, 0, scrollBarProc, kHorizontal);
}

/* Calculate the new control maximum value and current value, whether it is the horizontal or
   vertical scrollbar. The vertical max is calculated by comparing the number of lines to the
   vertical size of the viewRect. The horizontal max is calculated by comparing the maximum document
   width to the width of the viewRect. The current values are set by comparing the offset between
   the view and destination rects. If necessary and we canRedraw, have the control be re-drawn by
   calling ShowControl. */
void AdjustHV(Boolean isVert, ControlHandle controlHndl, TEHandle teHndl, Boolean canRedraw) {
	short		value, lines, max;
	short		oldValue, oldMax;
	TEPtr		tePtr;

#ifndef H_SCROLLBARS
	if (!isVert) return;
#endif
	
	oldValue = GetControlValue(controlHndl);
	oldMax = GetControlMaximum(controlHndl);
	tePtr = *teHndl;
	
	if (isVert) {
		lines = tePtr->nLines;
		
		/* since nLines isn't right if the last character is a return, check for that case */
		if (*(*tePtr->hText + tePtr->teLength - 1) == kReturnCharCode)
			lines += 1;
		max = lines - ((tePtr->viewRect.bottom - tePtr->viewRect.top) /
				tePtr->lineHeight);
	} else
		max = kMaxDocWidth - (tePtr->viewRect.right - tePtr->viewRect.left);
	
	if (max < 0) max = 0;
	SetControlMaximum(controlHndl, max);
	
	/* Must deref. after SetCtlMax since, technically, it could draw and therefore move
       memory. This is why we don't just do it once at the beginning. */
	tePtr = *teHndl;
	
	if (isVert)
		value = (tePtr->viewRect.top - tePtr->destRect.top) / tePtr->lineHeight;
	else
		value = tePtr->viewRect.left - tePtr->destRect.left;
	
	if (value < 0) value = 0;
	else if (value >  max) value = max;
	
	SetControlValue(controlHndl, value);
	
	/* now redraw the control if it needs to be and can be */
	if (canRedraw || (max != oldMax) || (value != oldValue)) {
	    ShowControl(controlHndl);
	}
}


/* Simply call the common adjust routine for the vertical and horizontal scrollbars. */
void AdjustScrollValues(WindowPtr winPtr, Boolean canRedraw) {
	DocumentPeek docPtr;
	
	docPtr = (DocumentPeek)winPtr;
	AdjustHV(true, docPtr->vScrollHndl, docPtr->teHndl, canRedraw);
	
#ifdef H_SCROLLBARS
    AdjustHV(false, docPtr->hScrollHndl, docPtr->teHndl, canRedraw);
#endif
}


/*	Re-calculate the position and size of the viewRect and the scrollbars.
	kScrollTweek compensates for off-by-one requirements of the scrollbars
	to have borders coincide with the growbox. */
void AdjustScrollSizes(WindowPtr winPtr) {
	Rect		    teRect;
	DocumentPeek    docPtr;
	
	docPtr = (DocumentPeek)winPtr;
	GetTERect(winPtr, &teRect);                     /* start with TERect */
	(*docPtr->teHndl)->viewRect = teRect;
	AdjustViewRect(docPtr->teHndl);                 /* snap to nearest line */
	MoveControl(docPtr->vScrollHndl, winPtr->portRect.right - kScrollbarAdjust, -1);
	SizeControl(docPtr->vScrollHndl, kScrollbarWidth, (winPtr->portRect.bottom - 
                winPtr->portRect.top) - (kScrollbarAdjust - kScrollTweek));
                
#ifdef H_SCROLLBARS
   MoveControl(docPtr->hScrollHndl, -1, winPtr->portRect.bottom - kScrollbarAdjust);
   SizeControl(docPtr->hScrollHndl, (winPtr->portRect.right - 
                winPtr->portRect.left) - (kScrollbarAdjust - kScrollTweek),
                kScrollbarWidth);
#endif
}

/* Turn off the controls by jamming a zero into their contrlVis fields (HideControl erases them
   and we don't want that). If the controls are to be resized as well, call the procedure to do that,
   then call the procedure to adjust the maximum and current values. Finally re-enable the controls
   by jamming a $FF in their contrlVis fields. */
void AdjustScrollBars(WindowPtr winPtr, Boolean needsResize) {
	DocumentPeek docPtr;
	
	docPtr = (DocumentPeek)winPtr;
	
	/* First, turn visibility of scrollbars off so we won't get unwanted redrawing */
	(*docPtr->vScrollHndl)->contrlVis = kControlInvisible;	
    (*docPtr->hScrollHndl)->contrlVis = kControlInvisible;
	
	/* move & size as needed */
	if (needsResize) {							
		AdjustScrollSizes(winPtr);
    }
	
	/* fool with max and current value */
	AdjustScrollValues(winPtr, needsResize);			
	
	/* Now, restore visibility in case we never had to ShowControl during adjustment */
	(*docPtr->vScrollHndl)->contrlVis = kControlVisible;
	
#ifdef H_SCROLLBARS
    (*docPtr->hScrollHndl)->contrlVis = kControlVisible;
#endif
}

/* Glue proc for handling scrollbar events. */
pascal void ScrollGlue(ControlHandle controlHndl, short partCode) {
    WindowPtr   winPtr;
    TEPtr       tePtr;
    short       linesPerPage;
    short       amount;
    
    if (partCode) {
        winPtr = (*controlHndl)->contrlOwner;
        tePtr = *((DocumentPeek)winPtr)->teHndl;
        linesPerPage = (tePtr->viewRect.bottom - tePtr->viewRect.top) / tePtr->lineHeight;
    
        switch (partCode) {
            case inUpButton:
            case inDownButton:
                amount = 1;            
                break;
            case inPageUp:
            case inPageDown:
                /* Subtract 1 when paging so we can see the
                   last line from the previous page */
                amount = linesPerPage - 1;            
                break;
        }
        
        if ((partCode == inDownButton) || (partCode == inPageDown)) {
            amount = -amount;
        }
        
        ScrollbarAction(controlHndl, &amount);
        
        /* If there is an amount to scroll, then scroll the TextEdit control by
           the amount * lineHeight. The scrollbar is normalized to the number of lines
           in the TextEdit control, but we scroll the TextEdit control by pixels. */
        if (amount != 0) {
            TEScroll(0, amount * tePtr->lineHeight, ((DocumentPeek)winPtr)->teHndl);
        }
    }
}

/* Get the right amount to scroll the scrollbar by. */
void ScrollbarAction(ControlHandle controlHndl, short *amount) {
    short   value;
    short   max;
    
    value = GetControlValue(controlHndl);
    max = GetControlMaximum(controlHndl);
    
    *amount = value - *amount;
    
    if (*amount < 0) {
        *amount = 0;
    } else if (*amount > max) {
        *amount = max;
    }
    
    SetControlValue(controlHndl, *amount);
    
    *amount = value - *amount;
}

/* Scroll to the insertion point of the TextEdit control. */
void ScrollInsertPt(WindowPtr winPtr) {
    DocumentPeek    docPtr;
    TEPtr           tePtr;
    short           position, line, nLines, linePos, lineHeight, viewTop, viewBot;
    
    docPtr = (DocumentPeek)winPtr;
    HLock((Handle)docPtr->teHndl);
    tePtr = *docPtr->teHndl;
    
    nLines = tePtr->nLines;
    position = tePtr->selEnd;
    viewTop = tePtr->viewRect.top;
    viewBot = tePtr->viewRect.bottom;
    lineHeight = tePtr->lineHeight;
    
    AdjustHV(true, docPtr->vScrollHndl, docPtr->teHndl, true);    
    line = 1;
    
    while ((position > tePtr->lineStarts[line]) && (line <= nLines)) line += 1;
    
    linePos = tePtr->destRect.top + lineHeight * line;
    
    if (linePos < viewTop) {
        do {
            TEScroll(0, lineHeight, docPtr->teHndl);
            linePos += lineHeight;
            SetControlValue(docPtr->vScrollHndl, GetControlValue(docPtr->vScrollHndl) - 1);
        } while  (linePos < viewTop);
    } else if (linePos > viewBot) {
        do {
          TEScroll(0, -lineHeight, docPtr->teHndl);
          linePos -= lineHeight;
          SetControlValue(docPtr->vScrollHndl, GetControlValue(docPtr->vScrollHndl) + 1);
        } while (linePos > viewBot);
    }
    
    HUnlock((Handle)docPtr->teHndl);
}