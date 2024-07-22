#include <AppleEvents.h>
#include <Controls.h>
#include <Dialogs.h>
#include <DiskInit.h>
#include <Events.h>
#include <Files.h>
#include <MacTypes.h>
#include <MixedMode.h>
#include <Quickdraw.h>
#include <Resources.h>
#include <StandardFile.h>
#include <Strings.h>
#include <TextEdit.h>
#include <ToolUtils.h>
#include <Windows.h>

enum {
    kWindowVisible      = true,
    kWindowGoAway       = true
};

/* Resource IDs */
enum {
    kConfirmDialogResID             = 129,
    kConfirmDialogButtonOK          = 1,
    kConfirmDialogButtonCancel      = 2,
    kConfirmDialogButtonDontSave    = 3
};

void InitToolbox(void);
void DoOpen(void);
Boolean DoSaveAs(void);
void DoQuit(void);
void RestartProc(void);
WindowPtr SetupWindow(void);
void ProcessFile(void *replyPtr, Boolean isHFSPlusReply);
void AutoSaveHash(Str255 processedFileNamePStr, short vRefNum, StringPtr md5HexResultPStr);
void PrintError(OSErr err);
void SetupSaveParamBlock(ParmBlkPtr paramBlkPtr);
OSErr PBSetTypeCreator(ParmBlkPtr paramBlkPtr, OSType fileType, OSType fileCreator);
void EventLoop(void);
void DoActivate(WindowPtr winPtr, Boolean becomingActive);
void DoIdle(void);
Boolean IsAppWindow(WindowPtr winPtr);
void DoContentClick(WindowPtr winPtr, EventRecord event);
void DoUpdate(WindowPtr winPtr);
void GetLocalUpdateRgn(WindowPtr winPtr, RgnHandle localRgn);
void DoGrowWindow(WindowPtr winPtr, EventRecord *event);
void DoZoomWindow(WindowPtr winPtr, short part);
void DoResizeWindow(WindowPtr winPtr);
void ExitApplication(void);

int main(void);