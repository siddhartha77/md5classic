#include <Controls.h>

enum {
    kScrollBarHeight = 15,
    kScrollBarWidth = 15
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

extern Rect            lowerRect;
extern Rect            vScrollRect, hScrollRect;
extern ControlHandle   vScrollHndl, hScrollHndl;
extern short           startValue, endValue;

void SetupScrollBars(WindowPtr winPtr);
pascal void ScrollGlue(ControlHandle controlHndl, short partCode);
void SetScrollBarValue(ControlHandle controlHndl, short *amount);
void ScrollInsertPt(void);