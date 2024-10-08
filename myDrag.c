#include "myDrag.h"
#include "main.h"
#include "globals.h"
#include "utils.h"

void InitDragManager() {
    InstallTrackingHandler(NewDragTrackingHandlerUPP(MyTrackingHandler), 0L, NULL);
    InstallReceiveHandler(NewDragReceiveHandlerUPP(MyReceiveHandler), 0L, NULL);
}

pascal OSErr MyTrackingHandler(DragTrackingMessage theMessage, WindowPtr theWindow, void *handlerRefCon, DragReference theDrag) {
#pragma unused (handlerRefCon)
    RgnHandle   hiliteRgn;
    
    switch(theMessage) {
        case kDragTrackingEnterHandler:
            break;
        case kDragTrackingEnterWindow:
            RectRgn(hiliteRgn = NewRgn(), &theWindow->portRect);
            ShowDragHilite(theDrag, hiliteRgn, true);
            DisposeRgn(hiliteRgn);
           
            break;
        case kDragTrackingInWindow:            
            break;
        case kDragTrackingLeaveWindow:
            HideDragHilite(theDrag);
            
            break;
        case kDragTrackingLeaveHandler:
            break;
    }
    return noErr;
}

pascal OSErr MyReceiveHandler(WindowPtr theWindow, void *handlerRefCon, DragReference theDrag) {
#pragma unused (handlerRefCon)
    OSErr               err = noErr;
    SFReply             sfReply;
    StandardFileReply   stdReply;
    HFSFlavor           dragHFSData;
    ItemReference       theItem;
    FlavorType          theFlavor;
    Size                dataSize;
    TEHandle            teHndl;
    unsigned short      i;
    unsigned short      numItems;
    Boolean             targetIsFolder;
    Boolean             wasAliased;
    
    teHndl = ((DocumentPeek)theWindow)->teHndl;
    err = CountDragItems(theDrag, &numItems);
    
    PrintError(err);
    
    for (i = 1 ; i <= numItems ; ++i) {
        GetDragItemReferenceNumber(theDrag, i, &theItem);
        GetFlavorType(theDrag, theItem, 1, &theFlavor);

        /* kDragFlavorTypeFileURL is 'furl' and is the flavor used when dragging
           in Classic Mode OS X */
        if (theFlavor == kDragFlavorTypeHFS || theFlavor == kDragFlavorTypeFileURL) {
            err = GetFlavorDataSize(theDrag, theItem, kDragFlavorTypeHFS, &dataSize);
            err = GetFlavorData(theDrag, theItem, kDragFlavorTypeHFS, &dragHFSData, &dataSize, 0L);
            
            if (err) return err;
            
            if (dragHFSData.fileCreator != kDragPseudoCreatorVolumeOrDirectory) {
                    err = ResolveAliasFile(&dragHFSData.fileSpec, true, &targetIsFolder, &wasAliased);
                
                if (err == fnfErr) {
                    HideDragHilite(theDrag);
                } else {
                    if (gHasHFSPlusAPIs) {
                        stdReply.sfGood = true;
                        stdReply.sfType = dragHFSData.fileType;
                        stdReply.sfFile = dragHFSData.fileSpec;
                        stdReply.sfIsFolder = false;
                        stdReply.sfIsVolume = false;
                        
                        ProcessFile(&stdReply, gHasHFSPlusAPIs);
                    } else {                    
                        sfReply.good = true;
                        sfReply.fType = dragHFSData.fileType;
                        OpenWD(dragHFSData.fileSpec.vRefNum, dragHFSData.fileSpec.parID, NULL, &sfReply.vRefNum);
                        sfReply.version = NULL;
                        myCopyPStr(dragHFSData.fileSpec.name, sfReply.fName);
                        
                        ProcessFile(&sfReply, gHasHFSPlusAPIs);
                    }
                }
                
                /* Update the dialog and maintain dimming */
                DoUpdate(theWindow);
            }
        }
    }

    return err;
}