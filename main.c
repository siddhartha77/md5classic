#include "main.h"
#include "globals.h"
#include "prefs.h"
#include "checkTrap.h"
#include "myAppleEvents.h"
#include "myMenus.h"
#include "myScrollBars.h"
#include "myTextEdit.h"
#include "utils.h"
#include "md5.h"

/* Initialize the Toolbox */
void InitToolbox() {
    MaxApplZone();
    
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitDialogs(RestartProc);
    InitMenus();
    InitCursor();
    TEInit();    
    
    FlushEvents(everyEvent, 0);
}

/* Call the open file dialog and process the file if the user selects one. */
void DoOpen() {
    SFReply         reply;
    Point           where;
    TEHandle        teHndl;
    WindowPtr       winPtr;
    
    winPtr = FrontWindow();
    teHndl = ((DocumentPeek)winPtr)->teHndl;
    SetPt(&where, kOpenLeft, kOpenTop);    
    SFGetFile(where, NULL, NULL, -1, NULL, NULL, &reply);
    
    /* Set the cursor to the end of the text */
    TESetSelect((*teHndl)->teLength, (*teHndl)->teLength, teHndl);
        
    if (reply.good) {
        ProcessFile(&reply);
    } else {
        /* User cancelled. */
    }
}

/* Call the save dialog box. */
Boolean DoSaveAs() {
    SFReply         reply;
    ParamBlockRec   paramBlk;
    Point           where;
    TEHandle        teHndl;
    WindowPtr       winPtr;
    OSErr           err;
    
    winPtr = FrontWindow();
    teHndl = ((DocumentPeek)winPtr)->teHndl;
    SetPt(&where, 30, 30);    
    SFPutFile(where, SAVE_PROMPT, ORIG_SAVENAME, NULL, &reply);
        
    if (reply.good) {        
        SetupSaveParamBlock(&paramBlk);
        
        paramBlk.ioParam.ioNamePtr = reply.fName;
        paramBlk.ioParam.ioVRefNum = reply.vRefNum;
        paramBlk.ioParam.ioVersNum = reply.version;
        paramBlk.ioParam.ioBuffer = *(*teHndl)->hText;
        paramBlk.ioParam.ioReqCount = (*teHndl)->teLength;

        /* Write the output file */
        HLock((*teHndl)->hText);
        err = PBCreate(&paramBlk, 0);
        err = PBOpen(&paramBlk, false);
        err = PBWrite(&paramBlk, false);
        HUnlock((*teHndl)->hText);
        err = PBClose(&paramBlk, false);        
        err = PBSetTypeCreator(&paramBlk, 'TEXT', 'ttxt');
        
        PrintError(err);
        
        if (err == noErr) {
            ((DocumentPeek)winPtr)->unsavedData = false;
            
            return true;
        }
        
    } else {
        /* User cancelled. */
    }
    
    return false;
}

/* Quit the application, but confirm with the user if that preference is set. */
void DoQuit() {
    WindowPtr   winPtr;
    short       itemHit;
    
    winPtr = FrontWindow();
    
    if (gPrefs.askToSave && ((DocumentPeek)winPtr)->unsavedData) {
        SetCursor(&qd.arrow);
        itemHit = CautionAlert(kConfirmDialogResID, NULL);
        
        switch (itemHit) {
            case kConfirmDialogButtonOK:
                if (DoSaveAs()) {    
                    ExitApplication();
                }

                break;
            case kConfirmDialogButtonCancel:
                break;
            
            /* Don't Save clicked */
            default:
                ExitApplication();

                break;       
        }
    } else {    
        ExitApplication();        
    }
}

/* Restart procedure for InitDialogs Toolbox call. */
void RestartProc() {
    ExitToShell();
}

/* Create a new window and DocumentRecord. */
WindowPtr SetupWindow() {
    Rect            winRect;
    Ptr             storage;
    WindowPtr       winPtr;  
    
    /* Init Window */
    SetRect(&winRect, kWindowLeft, kWindowTop, kWindowRight, kWindowBottom);
    
    storage = NewPtr(sizeof(DocumentRecord));
    winPtr = NewWindow(storage, &winRect, APP_NAME, kWindowVisible, zoomDocProc, (WindowPtr)-1, kWindowGoAway, 0);
    
    if (winPtr != NULL) {
        DrawGrowIcon(winPtr);
        SetPort(winPtr);
        
        ((DocumentPeek)winPtr)->unsavedData = false;
        
        /* Limit movements of window */
        SetRect(&((DocumentPeek)winPtr)->dragBoundsRect,
            qd.screenBits.bounds.left + kWindowDragMargin,
            qd.screenBits.bounds.top + kMenubarHeight + kWindowDragMargin,
            qd.screenBits.bounds.right - kWindowDragMargin,
            qd.screenBits.bounds.bottom - kWindowDragMargin);
    } else {
        DisposePtr(storage);
    }
    
    return winPtr;
}

/* Process a file and print the results. */
void ProcessFile(SFReply *replyPtr) {
    ParamBlockRec   paramBlk;
    Str63           processingPStr = CALC_INFO;
    Str32           md5HexResultPStr = CANCEL_INFO;
    Str32           secondsPStr = TIME_2_INFO;
    Str32           bytesPerSecondPStr;
    Str15           fileSizePStr;
    Str15           timerPStr;
    CursHandle      watchCurs;
    TEHandle        teHndl;
    WindowPtr       winPtr;
    OSErr           err;
    unsigned long   startTime;
    unsigned long   endTime;
    unsigned long   diffTime;
    long            logEOF;
    long            bytesPerSecond;
    short           result;
    unsigned char   md5Result[MD5_DIGEST_SIZE];
    
    winPtr = FrontWindow();
    teHndl = ((DocumentPeek)winPtr)->teHndl;
    
    /* Set the cursor to the end of the text */
    TESetSelect((*teHndl)->teLength, (*teHndl)->teLength, teHndl);
    
    paramBlk.ioParam.ioCompletion = NULL;
    paramBlk.ioParam.ioNamePtr = replyPtr->fName;
    paramBlk.ioParam.ioVRefNum = replyPtr->vRefNum;
    paramBlk.ioParam.ioVersNum = replyPtr->version;
    paramBlk.ioParam.ioPermssn = fsRdPerm;
    paramBlk.ioParam.ioMisc = NULL;
    
    err = PBOpen(&paramBlk, false);    
    TEUpdate(&(*teHndl)->viewRect, teHndl);     
    GetEOF(paramBlk.ioParam.ioRefNum, &logEOF);
    NumToString(logEOF, fileSizePStr);
    TEInsert(replyPtr->fName + 1, replyPtr->fName[0], teHndl);
    TEInsert(" (", 2, teHndl);
    TEInsert(fileSizePStr + 1, fileSizePStr[0], teHndl);
    TEInsert(" bytes)", 8, teHndl);
    
    /* Print the error here so we can see the filename */
    PrintError(err);
                
    if (err == noErr) {        
        watchCurs = GetCursor(watchCursor);
        SetCursor(*watchCurs);
        TEInsert(processingPStr + 1, processingPStr[0], teHndl);
        
        GetDateTime(&startTime);

        /* Do the MD5 calculation */
        result = MD5MacFile(&paramBlk, md5Result);
        
        if (result) {
            myMD5ValsToHexChars(md5Result, md5HexResultPStr, gPrefs.uppercaseHash);
        }
        
        GetDateTime(&endTime);        
        NumToString(endTime - startTime, timerPStr);

        /* Clear the line. The magic 10 here as for "( bytes). */
        TESetSelect((*teHndl)->selStart - processingPStr[0] - fileSizePStr[0] - replyPtr->fName[0] - 10, (*teHndl)->selEnd, teHndl);
        TEDelete(teHndl);
        TEInsert(md5HexResultPStr + 1, md5HexResultPStr[0], teHndl);
        TEInsert("  ", 2, teHndl);
        TEInsert(replyPtr->fName + 1, replyPtr->fName[0], teHndl);
        TEInsert("  ", 2, teHndl);
        TEInsert(fileSizePStr + 1, fileSizePStr[0], teHndl);
        TEInsert(" bytes", 7, teHndl);
        
        if (gPrefs.verbose && result) {
            TEInsert("  ", 2, teHndl);
            TEInsert(timerPStr + 1, timerPStr[0], teHndl);
            TEInsert(secondsPStr + 1, secondsPStr[0], teHndl);
            
            diffTime = endTime - startTime;
            
            if (diffTime > 0) {
                bytesPerSecond = logEOF / diffTime;
            } else {
                bytesPerSecond = logEOF;
            }
                
            NumToString(bytesPerSecond, bytesPerSecondPStr);
            TEInsert(bytesPerSecondPStr + 1, bytesPerSecondPStr[0], teHndl);
            TEInsert(" bytes/sec)\r", 13, teHndl);            
        } else {
            TEInsert("\r", 2, teHndl);
        }
        
        ScrollInsertPt(winPtr);
        
        if (gPrefs.autosaveHash) {
            AutoSaveHash(&paramBlk, md5HexResultPStr);
        }
        
        SetCursor(&qd.arrow);
        ((DocumentPeek)winPtr)->unsavedData = true;
    }
}

/* Save the file hash to a .md5 text file. */
void AutoSaveHash(ParmBlkPtr processedFileParamBlkPtr, StringPtr md5HexResultPStr) {
    Str255          buffer;
    Str63           filename;
    ParamBlockRec   paramBlk;
    OSErr           err;
    
    myCopyPStr(processedFileParamBlkPtr->ioParam.ioNamePtr, filename);
    myAppendPStr(filename, "\p.md5");
    mySafeFilename(filename);
    myCopyPStr(md5HexResultPStr, buffer);
    myAppendPStr(buffer, "\p *");
    myAppendPStr(buffer, processedFileParamBlkPtr->ioParam.ioNamePtr);
    
    SetupSaveParamBlock(&paramBlk);
        
    paramBlk.ioParam.ioNamePtr = filename;
    paramBlk.ioParam.ioVRefNum = processedFileParamBlkPtr->ioParam.ioVRefNum;
    paramBlk.ioParam.ioVersNum = processedFileParamBlkPtr->ioParam.ioVersNum;
    paramBlk.ioParam.ioBuffer = (char *)&buffer[1];
    paramBlk.ioParam.ioReqCount = buffer[0];
    
    /* Write the output file */
    err = PBCreate(&paramBlk, 0);
    err = PBOpen(&paramBlk, false);
    err = PBWrite(&paramBlk, false);
    err = PBClose(&paramBlk, false);        
    err = PBSetTypeCreator(&paramBlk, 'TEXT', 'ttxt');
        
    PrintError(err);
}

/* Print and error to the window. */
void PrintError(OSErr err) {
    Str255      errorPStr = ERROR_INFO;
    Str255      newLinePStr = "\p\r";
    Str255      errorNumPStr;
    TEHandle        teHndl;
    WindowPtr       winPtr;
    
    if (err != noErr) {
        winPtr = FrontWindow();
        teHndl = ((DocumentPeek)winPtr)->teHndl;
    
        /* Set the cursor to the end of the text */
        TESetSelect((*teHndl)->teLength, (*teHndl)->teLength, teHndl);
        TEInsert(errorPStr + 1, errorPStr[0], teHndl);
        NumToString(err, errorNumPStr);
        TEInsert(errorNumPStr + 1, errorNumPStr[0], teHndl);
        TEInsert(newLinePStr + 1, newLinePStr[0], teHndl);
        TEInsert(newLinePStr + 1, newLinePStr[0], teHndl);
        ScrollInsertPt(winPtr);
    }
}

/* Default values for a save file Parameter Block. */
void SetupSaveParamBlock(ParmBlkPtr paramBlkPtr) {    
    paramBlkPtr->ioParam.ioCompletion = NULL;
    paramBlkPtr->ioParam.ioPermssn = fsRdWrPerm;
    paramBlkPtr->ioParam.ioMisc = NULL;
    paramBlkPtr->ioParam.ioPosMode = fsFromStart;
    paramBlkPtr->ioParam.ioPosOffset = 0L;
}

/* Set the type and creator of a file referenced by Parameter Block. */
OSErr PBSetTypeCreator(ParmBlkPtr paramBlkPtr, OSType fileType, OSType fileCreator) {
    OSErr   err;
    
    /* Set the Type and Creator */
    paramBlkPtr->fileParam.ioFDirIndex = 0;
    
    err = PBGetFInfo(paramBlkPtr, false);
    paramBlkPtr->fileParam.ioFlFndrInfo.fdType = fileType;
    paramBlkPtr->fileParam.ioFlFndrInfo.fdCreator = fileCreator;
    err = PBSetFInfo(paramBlkPtr, false);
    
    return err;
}

/* The main event loop. */
void EventLoop() {
    EventRecord     event;
	Point		    aPoint;
    WindowPtr       winPtr;
    WindowPartCode  windowCode;
    char            c;

    while (true) {
        SystemTask();
        
        if (GetNextEvent(everyEvent, &event)) {        
            switch (event.what) {
                case kHighLevelEvent:
                    AEProcessAppleEvent(&event);
                    
                    break;
                case mouseDown:
                    switch (windowCode = FindWindow(event.where, &winPtr)) {
                        case inDesk:
                            if (winPtr != NULL) {
                                HiliteWindow(winPtr, 0);
                                DrawGrowIcon(winPtr);
                            }
                            
                            break;
                        case inMenuBar:
                            DoMenu(MenuSelect(event.where));
                            
                            break;
                        case inGoAway:
                            if (TrackGoAway(winPtr, event.where)) {
                                DoQuit();
                            }
                            
                            break;
                        case inDrag:
                            DragWindow(winPtr, event.where, &((DocumentPeek)winPtr)->dragBoundsRect);
                            DrawGrowIcon(winPtr);
                            
                            break;
                        case inGrow:
                            DoGrowWindow(winPtr, &event);
                        
                            break;
                        case inZoomIn:
                        case inZoomOut:
                            if (TrackBox(winPtr, event.where, windowCode)) {
                                ZoomWindow(winPtr, windowCode, winPtr == FrontWindow());
                                DoResizeWindow(winPtr);
                            }
                            
                            break;
                        case inContent:
                            DoContentClick(winPtr, event);
                            
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
                        /* Check for any other keys (e.g. 'd')
                           I guess easter eggs can go here... */        
                        switch (c) {
                            default:
                                break;
                        }
                    }
                    
                    break;
                case activateEvt:
                    DoActivate((WindowPtr)event.message, (event.modifiers & activeFlag) != 0);
                    
			        break;
                case updateEvt:
                    DoUpdate((WindowPtr)event.message);
                                        
                    break;
                /* It is not a bad idea (apparently) to at least call DIBadMount in response
                    to a diskEvt, so that the user can format a floppy. */
                case diskEvt:
                    if ( HiWord(event.message) != noErr ) {
                        SetPt(&aPoint, kDILeft, kDITop);
                        DIBadMount(aPoint, event.message);
                    }
                    break;
                case osEvt:
                    /* high byte of message */
                    switch ((event.message >> 24) & 0xff) {
        				case mouseMovedMessage:
        					DoIdle();
        					
        					break;
        				case suspendResumeMessage:
        					DoActivate(FrontWindow(), (event.message & resumeFlag) != 0);
        					
        					break;
        			}
        			
                    break;
                default:
                    break;
            }
        } else {
            DoIdle();
        }
    }
}

/* This is called when a window is activated or deactivated.
   It calls TextEdit to deal with the selection. */
void DoActivate(WindowPtr winPtr, Boolean becomingActive) {
	RgnHandle	    tempRgn, clipRgn;
	Rect		    growRect;
	DocumentPeek    docPtr;
	
	if ( IsAppWindow(winPtr) ) {
		docPtr = (DocumentPeek) winPtr;
		if ( becomingActive ) {
			/* Since we don't want TEActivate to draw a selection in an area where
               we're going to erase and redraw, we'll clip out the update region
               before calling it. */
			tempRgn = NewRgn();
			clipRgn = NewRgn();
			/* get localized update region */
			GetLocalUpdateRgn(winPtr, tempRgn);			
	        GetClip(clipRgn);
			/* subtract updateRgn from clipRgn */
		    DiffRgn(clipRgn, tempRgn, tempRgn);
			SetClip(tempRgn);
			TEActivate(docPtr->teHndl);
			/* restore the full-blown clipRgn */
			SetClip(clipRgn);
			DisposeRgn(tempRgn);
			DisposeRgn(clipRgn);
			
			/* the controls must be redrawn on activation */
			(*docPtr->vScrollHndl)->contrlVis = kControlVisible;
			(*docPtr->hScrollHndl)->contrlVis = kControlVisible;
			InvalRect(&(*docPtr->vScrollHndl)->contrlRect);
			InvalRect(&(*docPtr->hScrollHndl)->contrlRect);
			
			/* the growbox needs to be redrawn on activation */
			growRect = winPtr->portRect;
			
			/* adjust for the scrollbars */
			growRect.top = growRect.bottom - kScrollbarAdjust;
			growRect.left = growRect.right - kScrollbarAdjust;
			InvalRect(&growRect);
		}
		else {		
			TEDeactivate(docPtr->teHndl);
			
			/* the controls must be hidden on deactivation */
			HideControl(docPtr->vScrollHndl);
			HideControl(docPtr->hScrollHndl);
			
			/* the growbox should be changed immediately on deactivation */
			DrawGrowIcon(winPtr);
		}
	}
}

/* If no events then idle... */
void DoIdle() {
    WindowPtr   winPtr;
    
    winPtr = FrontWindow();
    
    if (IsAppWindow(winPtr)) {
        ChangeMouse(((DocumentPeek)winPtr)->teHndl);
        TEIdle(((DocumentPeek)winPtr)->teHndl);
    }
}

/* Application windows have windowKinds = userKind (8). */
Boolean IsAppWindow(WindowPtr winPtr) {
    short   windowKind;

	if ( winPtr == nil )
		return false;
	else {
		windowKind = ((WindowPeek) winPtr)->windowKind;
		return (windowKind == userKind);
	}
}

/* Content click event handling. */
void DoContentClick(WindowPtr winPtr, EventRecord event) {
    ControlHandle   controlHndl;
    TEHandle        teHndl;
    WindowPartCode  partCode;
    short           value;
    
    if ((FrontWindow() != winPtr) && (winPtr != NULL)) {
       SelectWindow(winPtr);
    }
    
    teHndl = ((DocumentPeek)winPtr)->teHndl;

    GlobalToLocal(&event.where);

    if (PtInRect(event.where, &(*teHndl)->viewRect)) {
        TEClick(event.where, event.modifiers & shiftKey, teHndl);
    } else {
        partCode = FindControl(event.where, winPtr, &controlHndl);

        switch (partCode) {
            case kControlNoPart:
                break;
            case inThumb:
                value = GetControlValue(controlHndl);
                partCode = TrackControl(controlHndl, event.where, NULL);

                if (partCode != 0) {
                    value -= GetControlValue(controlHndl);

                    if (value != 0) {
                        if (controlHndl == ((DocumentPeek)winPtr)->vScrollHndl) {
                            TEScroll(0, value * (*teHndl)->lineHeight, teHndl);
                        }
                    }
                }

                break;
            /* arrow clicked so track and scroll */
            default:
                if (controlHndl == ((DocumentPeek)winPtr)->vScrollHndl) {
                    TrackControl(controlHndl, event.where, NewControlActionUPP(ScrollGlue));
                }

                break;
        }
    }
}

/* Called when a mouseDown occurs in the grow box of an active window. In
   order to eliminate any 'flicker', we want to invalidate only what is
   necessary. Since ResizeWindow invalidates the whole portRect, we save
   the old TE viewRect, intersect it with the new TE viewRect, and
   remove the result from the update region. However, we must make sure
   that any old update region that might have been around gets put back. */
void DoGrowWindow(WindowPtr winPtr, EventRecord *event) {
	long		    growResult;
	Rect		    tempRect;
	RgnHandle	    tempRgn;
	
	tempRect = qd.screenBits.bounds;					/* set up limiting values */
	tempRect.left = kMinDocDim;
	tempRect.top = kMinDocDim;
	growResult = GrowWindow(winPtr, event->where, &tempRect);
	
	/* see if it really changed size */
	if ( growResult != 0 ) {
		tempRgn = NewRgn();
		GetLocalUpdateRgn(winPtr, tempRgn);				/* get localized update region */
		SizeWindow(winPtr, LoWord(growResult), HiWord(growResult), true);
		DoResizeWindow(winPtr);
		InvalRgn(tempRgn);								/* put back any prior update */
		DisposeRgn(tempRgn);
	}
}


/* Called when a mouseClick occurs in the zoom box of an active window.
   Everything has to get re-drawn here, so we don't mind that
   ResizeWindow invalidates the whole portRect. */
void DoZoomWindow(WindowPtr winPtr, short part) {
	EraseRect(&winPtr->portRect);
	ZoomWindow(winPtr, part, winPtr == FrontWindow());
	DoResizeWindow(winPtr);
}


/* Called when the window has been resized to fix up the controls and content. */
void DoResizeWindow(WindowPtr winPtr) {
    UpdateTEWordWrap(winPtr);
    AdjustScrollBars(winPtr, true);
    AdjustTE(winPtr);
    InvalRect(&winPtr->portRect);
}

/* Update event. Draw all the controls and update the TextEdit control. */
void DoUpdate(WindowPtr winPtr) {
    if (IsAppWindow(winPtr)) {
        BeginUpdate(winPtr);
        
        if (!EmptyRgn(winPtr->visRgn)) {
            SetPort(winPtr);
            EraseRect(&winPtr->portRect);
            DrawControls(winPtr);
            DrawGrowIcon(winPtr);
            TEUpdate(&winPtr->portRect, ((DocumentPeek)winPtr)->teHndl);
        }
        
        EndUpdate(winPtr);
    }
}

/* Returns the update region in local coordinates */
void GetLocalUpdateRgn(WindowPtr winPtr, RgnHandle localRgn) {
	CopyRgn(((WindowPeek) winPtr)->updateRgn, localRgn);	/* save old update region */
	OffsetRgn(localRgn, winPtr->portBits.bounds.left, winPtr->portBits.bounds.top);
}

/* Clear some stuff and then exit. */
void ExitApplication() {
    WindowPtr   winPtr;
    
    winPtr = FrontWindow();
    TEDispose(((DocumentPeek)winPtr)->teHndl);
    DisposeWindow(winPtr);
    ExitToShell();
}

/* It's the main function. */
int main() {
    WindowPtr   winPtr;
    long        aLong;
    
    MaxApplZone();
    MoreMasters();
    MoreMasters();
    InitToolbox();
    InitPreferences();
    SetupMenus();
    winPtr = SetupWindow();
    ShowWindow(winPtr);    
    SetupScrollBars(winPtr);
    SetupTE(winPtr);
    AdjustScrollBars(winPtr, true);
    
    /* If we support Drag and Drog, then initialize it. */
    if (IsTrapImplemented(_Gestalt) && Gestalt(gestaltAppleEventsAttr, &aLong) == noErr) {
        InitAppleEvents();
    } else {
        /* Open File Dialog */
        DoOpen();
    }
    
    EventLoop();
    
    /* Quit */
    DoQuit();
    
    return 0;
}
