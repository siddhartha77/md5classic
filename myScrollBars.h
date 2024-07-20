#include <Controls.h>
#include <ToolUtils.h>

enum {
    kScrollBarHeight = 15,
    kScrollBarWidth = 15
};

enum {
    kScrollBarBottomMargin = 10
};

enum {
    scrollBarProc       = 16,
    kHorizontal         = 1,
    kVertical           = 2
};

enum {
    inUpButton          = 20,
    inDownButton,
    inPageUp,
    inPageDown,
    inThumb             = 129
};

void SetupScrollBars(WindowPtr winPtr);
void AdjustHV(Boolean isVert, ControlHandle controlHndl, TEHandle teHandl,Boolean canRedraw);
void AdjustScrollValues(WindowPtr winPtr, Boolean canRedraw);
void AdjustScrollSizes(WindowPtr winPtr);
void AdjustScrollBars(WindowPtr winPtr, Boolean needsResize);
pascal void ScrollGlue(ControlHandle controlHndl, short partCode);
void ScrollbarAction(ControlHandle controlHndl, short *amount);
void ScrollInsertPt(WindowPtr winPtr);