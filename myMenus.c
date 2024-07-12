#include "main.h"
#include "globals.h"
#include "myMenus.h"
#include "myTextEdit.h"
#include "utils.h"

void SetupMenus(void) {
    MenuHandle  appleMenuHndl;
    MenuHandle  fileMenuHndl;
    MenuHandle  editMenuHndl;
    
    appleMenuHndl = GetMenu(kAppleMenuResID);
    AppendResMenu(appleMenuHndl, kAppleMenuDAType);
    InsertMenu(appleMenuHndl, 0);
    
    fileMenuHndl = GetMenu(kFileMenuResID);
    InsertMenu(fileMenuHndl, 0);
    
    editMenuHndl = GetMenu(kEditMenuResID);
    InsertMenu(editMenuHndl, 0);
    
    DrawMenuBar();
}

void DoMenu(long menuResult) {
    Str255      itemName;
    GrafPtr     currentPort;
    short       menuID;
    short       menuItem;
    
    menuID = HIWORD(menuResult);
    menuItem = LOWORD(menuResult);
    
    switch (menuID) {
        case kAppleMenuResID :
            switch (menuItem) {
                case kAppleMenuAbout:
                    NoteAlert(kAboutBoxResID, NULL);
                    
                    break;                    
                default:
                    GetMenuItemText(GetMenuHandle(kAppleMenuResID), menuItem, itemName);
                    GetPort(&currentPort);
				    OpenDeskAcc(itemName);
				    SetPort(currentPort);
				    			    
				    break;
            }
            
            break;
            
        case kFileMenuResID:
            switch (menuItem) {
                case kFileMenuOpen:
                    DoOpen();
                    
                    break;
                case kFileMenuSaveAs:
                    DoSaveAs();
                    
                    break;
                case kFileMenuQuit:
                    DoQuit();
                    
                    break;
            }
            
            break;
            
        case kEditMenuResID:
            switch (menuItem) {
                case kEditMenuCopy:
                    TECopy(gTEHndl);
                    ZeroScrap();
                    TEToScrap();
                    
                    break;                    
                case kEditMenuSelectAll:
                    TESetSelect(0, (*gTEHndl)->teLength, gTEHndl);
                    
                    break;
            }
            
            break;
    }
    
    HiliteMenu(0);
}

void DoOpen() {
    SFReply         reply;
    Point           where;
    
    SetPt(&where, 30, 30);    
    SFGetFile(where, NULL, NULL, -1, NULL, NULL, &reply);
    
    /* Set the cursor to the end of the text */
    TESetSelect((*gTEHndl)->teLength, (*gTEHndl)->teLength, gTEHndl);
        
    if (reply.good) {
        ProcessFile(&reply);
    } else {
        /* User cancelled. */
    }
}

void DoSaveAs() {
    SFReply         reply;
    ParamBlockRec   paramBlock;
    Point           where;
    OSErr           err;
    long            len;
    short           refNum;
    
    SetPt(&where, 30, 30);    
    SFPutFile(where, SAVE_PROMPT, ORIG_SAVENAME, NULL, &reply);
        
    if (reply.good) {
        paramBlock.ioParam.ioNamePtr = reply.fName;
        paramBlock.ioParam.ioVRefNum = reply.vRefNum;
        paramBlock.ioParam.ioVersNum = reply.version;
        
        err = PBCreate(&paramBlock, 0);
    
        err = FSOpen(reply.fName, reply.vRefNum, &refNum);
        len = (*gTEHndl)->teLength;
        err = FSWrite(refNum, &len, *(*gTEHndl)->hText);
        err = FSClose(refNum);
    } else {
        /* User cancelled. */
    }
}

void DoQuit() {
    TEDispose(gTEHndl);    
    DisposeWindow(gWinPtr);
    
    ExitToShell();
}