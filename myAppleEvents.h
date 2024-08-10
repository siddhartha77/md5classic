#include <AppleEvents.h>
#include <Files.h>

void InitAppleEvents(void);
OSErr GotRequiredParams (AppleEvent *appleEvent);
pascal OSErr HandleODOC(AppleEvent *appleEvent, AppleEvent *aeReply, long handlerRefcon);