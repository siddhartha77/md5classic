#include <Aliases.h>
#include <Drag.h>
#include <Files.h>
#include <Finder.h>
#include <StandardFile.h>

enum {
    kDragFlavorTypeFileURL      = FOUR_CHAR_CODE('furl')
};

void InitDragManager(void);
pascal OSErr MyTrackingHandler(DragTrackingMessage theMessage, WindowPtr theWindow, void *handlerRefCon, DragReference theDrag);
pascal OSErr MyReceiveHandler(WindowPtr theWindow, void *handlerRefCon, DragReference theDrag);
