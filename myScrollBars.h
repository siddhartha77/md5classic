#include <Controls.h>
#include <ToolUtils.h>

enum {
    /* kScrollbarAdjust and kScrollbarWidth are used in calculating
       values for control positioning and sizing. */
    kScrollbarWidth     = 16,
    kScrollbarAdjust    = 15,
    
    /* kScrollTweek compensates for off-by-one requirements of the scrollbars
       to have borders coincide with the growbox. */
    kScrollTweek        = 2
};

/*enum {
    kScrollBarBottomMargin = 10
};*/

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