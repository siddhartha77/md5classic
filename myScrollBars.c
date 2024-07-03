#include "myScrollBars.h"
#include "myTextEdit.h"
#include "globals.h"

Rect            lowerRect;
Rect            vScrollRect, hScrollRect;
ControlHandle   vScrollHndl, hScrollHndl;
short           startValue, endValue;

void SetupScrollBars(WindowPtr winPtr) {
    vScrollHndl = NewControl(winPtr, &vScrollRect,"\p", true, 0, 0, 0, scrollBarProc, kVertical);
    hScrollHndl = NewControl(winPtr, &hScrollRect,"\p", true, 0, 0, 0, scrollBarProc, kHorizontal);
    lowerRect = winPtr->portRect;
    lowerRect.right -= kScrollBarWidth;
    lowerRect.top = ((winPtr->portRect.bottom) >> 1) - 10;
    lowerRect.bottom -= kScrollBarHeight;
}

pascal void ScrollGlue(ControlHandle controlHndl, short partCode) {
    long    direction;
    short   amount;
    
    startValue = GetControlValue(controlHndl);
    direction = GetControlReference(controlHndl);
    
    switch (partCode) {
        case inUpButton:
            if (GetControlValue(controlHndl) > GetControlMinimum(controlHndl)) {
                SetControlValue(controlHndl, startValue - 1);
                
                if (direction == kVertical) {
                    ScrollContents(gTEHndl, 0, 1);
                }
            }
            
            break;
        case inDownButton:
            if (GetControlValue(controlHndl) < GetControlMaximum(controlHndl)) {
                SetControlValue(controlHndl, startValue + 1);
                
                if (direction == kVertical) {
                    ScrollContents(gTEHndl, 0, -1);
                }
            }
            
            break;
        case inPageUp:
            if (startValue - 10 < GetControlMinimum(controlHndl)) {
                amount = startValue - GetControlMinimum(controlHndl);
            } else {
                amount = 10;
            }
            
            SetControlValue(controlHndl, startValue - amount);
            
            if (direction == kVertical) {
                ScrollContents(gTEHndl, 0, amount);
            }
            
            break;        
        case inPageDown:
            if (startValue + 10 > GetControlMaximum(controlHndl)) {
                amount = GetControlMaximum(controlHndl) - startValue;
            } else {
                amount = 10;
            }
            
            SetControlValue(controlHndl, startValue + amount);
            
            if (direction == kVertical) {
                ScrollContents(gTEHndl, 0, -amount);
            }
            
            break;
    }
}

void SetScrollBarValue(ControlHandle controlHndl, short *amount) {
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
}

void ScrollInsertPt() {
    TEPtr   tePtr;
    short   position, line, nLines, linePos, lineHeight, viewTop, viewBot;
    
    HLock((Handle)gTEHndl);
    tePtr = *gTEHndl;
    
    nLines = tePtr->nLines;
    position = tePtr->selEnd;
    viewTop = tePtr->viewRect.top;
    viewBot = tePtr->viewRect.bottom;
    lineHeight = tePtr->lineHeight;
    
    line = 1;
    
    while ((position > tePtr->lineStarts[line]) && (line <= nLines)) line += 1;
    
    linePos = tePtr->destRect.top + lineHeight * (line + 1);
    
    if (linePos < viewTop) {
        do {
            TEScroll(0, lineHeight, gTEHndl);
            SetControlValue(vScrollHndl, GetControlValue(vScrollHndl) - lineHeight);
            linePos += lineHeight;
        } while  (linePos < viewTop);
    } else if (linePos > viewBot) {
        do {
            TEScroll(0, -lineHeight, gTEHndl);
            SetControlMaximum(vScrollHndl, (nLines * lineHeight) - (viewBot - 29));
            SetControlValue(vScrollHndl, GetControlValue(vScrollHndl) + lineHeight);
            linePos -= lineHeight;
        } while (linePos > viewBot);
    }
    
    HUnlock((Handle)gTEHndl);
}