#include "main.h"
#include "globals.h"
#include "checkTrap.h"
#include "myAppleEvents.h"
#include "myMenus.h"
#include "myScrollBars.h"
#include "myTextEdit.h"
#include "utils.h"
#include "md5.h"

WindowRecord    gWinRecord;
WindowPtr       gWinPtr = NULL;
Boolean         gDebug = false;

void InitToolbox() {
    MaxApplZone();
    
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    InitCursor();
    TEInit();    
    
    FlushEvents(everyEvent, 0);
}

void SetupWindow() {
    Rect            winRect;
    
    /* Init Window */
    SetRect(&winRect, 50, 50, 350, 250);
    gWinPtr = NewWindow(&gWinRecord, &winRect, APP_NAME, true, noGrowDocProc, (WindowPtr)-1, false, 0);
    SetPort(gWinPtr);
    
    /* Create margins for scrollbars */
    SetRect(&vScrollRect, (*gWinPtr).portRect.right - kScrollBarWidth, 
        (*gWinPtr).portRect.top - 1,
        (*gWinPtr).portRect.right + 1,
        (*gWinPtr).portRect.bottom - (kScrollBarHeight - 1));
                
    SetRect(&hScrollRect, (*gWinPtr).portRect.left - 1, 
        (*gWinPtr).portRect.bottom - kScrollBarHeight,
        (*gWinPtr).portRect.right - (kScrollBarWidth - 1),
        (*gWinPtr).portRect.bottom + 1);
}

void PrintError(OSErr err) {
    Str255      errorPStr = ERROR_INFO;
    Str255      newLinePStr = "\p\r";
    Str255      errorNumPStr;
    
    if (err != noErr) {        
        /* Set the cursor to the end of the text */
        TESetSelect((*gTEHndl)->teLength, (*gTEHndl)->teLength, gTEHndl);
        TEInsert(errorPStr + 1, errorPStr[0], gTEHndl);
        NumToString(err, errorNumPStr);
        TEInsert(errorNumPStr + 1, errorNumPStr[0], gTEHndl);
        TEInsert(newLinePStr + 1, newLinePStr[0], gTEHndl);
        TEInsert(newLinePStr + 1, newLinePStr[0], gTEHndl);
        ScrollInsertPt();
    }
}

void ProcessFile(Str63 fName, short vRefNum) {
    Str63           processingPStr = CALC_INFO;
    Str32           md5HexResult = CANCEL_INFO;
    Str32           thatTookPStr = TIME_1_INFO;
    Str32           secondsPStr = TIME_2_INFO;
    Str15           fileSizePStr;
    Str15           timerPStr;
    CursHandle      watchCurs;
    OSErr           err;
    unsigned long   startTime;
    unsigned long   endTime;
    long            logEOF;
    short           refNum;
    short           result;
    unsigned char   md5Result[16];
    
    gTEHndl = (TEHandle)GetWRefCon(gWinPtr);
    
    /* Set the cursor to the end of the text */
    TESetSelect((*gTEHndl)->teLength, (*gTEHndl)->teLength, gTEHndl);
    
    err = FSOpen(fName, vRefNum, &refNum);    
    TEUpdate(&gWinPtr->portRect, gTEHndl);     
    GetEOF(refNum, &logEOF);
    NumToString(logEOF, fileSizePStr);
    TEInsert(fName + 1, fName[0], gTEHndl);
    TEInsert(" (", 2, gTEHndl);
    TEInsert(fileSizePStr + 1, fileSizePStr[0], gTEHndl);        
    TEInsert(" bytes)\r", 8, gTEHndl);
    
    /* Print the error here so we can see the filename */
    PrintError(err);
                
    if (err == noErr) {        
        watchCurs = GetCursor(watchCursor);
        SetCursor(*watchCurs);
        TEInsert(processingPStr + 1, processingPStr[0], gTEHndl);
        
        GetDateTime(&startTime);

        /* Do the MD5 calculation */
        result = md5MacFile(refNum, md5Result);
        
        if (result) {
            myMD5ValsToHexChars(md5Result, md5HexResult);
        }
        
        GetDateTime(&endTime);        
        NumToString(endTime - startTime, timerPStr);

        /* Print results */
        TESetSelect((*gTEHndl)->selStart - processingPStr[0], (*gTEHndl)->selEnd, gTEHndl);
        TEDelete(gTEHndl);
        TEInsert(md5HexResult + 1, md5HexResult[0], gTEHndl);
        
        if (gDebug && result) {
            TEInsert(thatTookPStr + 1, thatTookPStr[0], gTEHndl);
            TEInsert(timerPStr + 1, timerPStr[0], gTEHndl);
            TEInsert(secondsPStr + 1, secondsPStr[0], gTEHndl);
        } else {
            TEInsert("\r\r", 2, gTEHndl);
        }
        
        ScrollInsertPt();           
        SetCursor(&qd.arrow);
    }
}

void MainEventLoop() {
    EventRecord     event;
    GrafPtr         savedPort;
    ControlHandle   controlHndl;
    WindowPtr       winPtr;
    long            direction;
    short           partCode;
    char            c;

    while (true) {
        SystemTask();
        ChangeMouse(gTEHndl);
        TEIdle(gTEHndl);
        
        if (GetNextEvent(everyEvent, &event)) {
            switch (event.what) {
                case kHighLevelEvent:
                    AEProcessAppleEvent(&event);
                    
                    break;
                case mouseDown:
                    switch (FindWindow(event.where, &winPtr)) {
                        case inDesk:
                            if (winPtr != NULL) {
                                HiliteWindow(gWinPtr, 0);
                            }
                            
                            break;
                        case inMenuBar:
                            DoMenu(MenuSelect(event.where));
                            
                            break;
                        /* Unused */
                        /*
                        case inGoAway:
                            if (TrackGoAway(winPtr, event.where)) {
                                DoQuit();
                            }
                            
                            break; 
                        */
                        case inDrag:
                            DragWindow(winPtr, event.where, &qd.screenBits.bounds);
                            
                            break;
                        case inContent:
                            if ((FrontWindow() != winPtr) && (winPtr != NULL)) {
                                SelectWindow(winPtr);
                            }
                            
                            GlobalToLocal(&event.where);
                            if (PtInRect(event.where, &(*gTEHndl)->viewRect)) {
                                TEClick(event.where, event.modifiers & shiftKey, gTEHndl);
                            } else {
                                partCode = FindControl(event.where, winPtr, &controlHndl);
                                
                                if (partCode) {
                                    direction = GetControlReference(controlHndl);
                                }
                                
                                if (partCode == inThumb) {
                                    startValue = GetControlValue(controlHndl);
                                    TrackControl(controlHndl, event.where, NULL);
                                    endValue = GetControlValue(controlHndl);
                                    
                                    if (direction == kVertical) {
                                        ScrollContents(gTEHndl, 0, startValue - endValue);
                                    }
                                } else if (partCode != 0) {
                                    TrackControl(controlHndl, event.where, NewControlActionUPP(ScrollGlue));
                                }
                            }
                            
                            break;
                        case inSysWindow:
                            SystemClick(&event, winPtr);
                            
                            break;
                        default:
                            break;
                    }
                    break;
                case keyDown:
                case autoKey:
                    c = event.message & charCodeMask;
                    
                    /* Check for shortcut */
                    if (event.modifiers & cmdKey) {
                        DoMenu(MenuKey(c));
                    } else {                
                        switch (c) {
                            case 'd':
                                if (gDebug) {
                                    gDebug = false;
                                    SetWTitle(gWinPtr, APP_NAME);
                                } else {
                                    gDebug = true;
                                    SetWTitle(gWinPtr, APP_NAME DEBUG_TITLE);
                                }
                                
                                break;
                            default:
                                break;
                        }
                    }
                    
                    break;
                case activateEvt:
                    winPtr = (WindowPtr)event.message;
                    SetPort(winPtr);
                    
                    if ((event.modifiers & activeFlag) == activeFlag) {
                        TEActivate(gTEHndl);
                    }
                    
                    break;
                case updateEvt:
                    winPtr = (WindowPtr)event.message;                                        
                    GetPort(&savedPort);
                    SetPort(winPtr);
                    BeginUpdate(winPtr);
                    DrawControls(winPtr);
                    UpdateText(gTEHndl);                   
                    EndUpdate(winPtr);
                    SetPort(savedPort);
                    break;
                case osEvt:
                    if (event.message & resumeFlag) {
                        TEActivate(gTEHndl);
                        HiliteControl(vScrollHndl, 0);
                        HiliteControl(hScrollHndl, 0);
                    } else {
                        TEDeactivate(gTEHndl);
                        HiliteControl(vScrollHndl, 0xff);
                        HiliteControl(hScrollHndl, 0xff);
                    }
                default:
                    break;
            }
        }
    }
}

int main() {
    long    aLong;
    
    InitToolbox();    
    SetupMenus();    
    SetupWindow();        
    SetupScrollBars(gWinPtr);
    gTEHndl = SetupTE(gWinPtr);
            
    /* Show the window */
    ShowWindow(gWinPtr);
        
    if (IsTrapImplemented(_Gestalt) && Gestalt(gestaltAppleEventsAttr, &aLong) == noErr) {
        InitAppleEvents();
    } else {
        /* Open File Dialog */
        DoOpen();
    }
    
    MainEventLoop();
    
    /* Quit */
    DoQuit();
    
    return 0;
}
