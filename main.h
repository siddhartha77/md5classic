#include <AppleEvents.h>
#include <Controls.h>
#include <Dialogs.h>
#include <Events.h>
#include <Files.h>
#include <MacTypes.h>
#include <MixedMode.h>
#include <Quickdraw.h>
#include <Resources.h>
#include <StandardFile.h>
#include <Strings.h>
#include <TextEdit.h>
#include <Windows.h>

extern WindowPtr       gWinPtr;

void InitToolbox(void);
void SetupWindow(void);
void ProcessFile(SFReply *replyPtr);
void PrintError(OSErr err);
void MainEventLoop(void);

int main(void);