#define APP_NAME        "\pmd5classic"

#define SAVE_PROMPT     "\pSave output as..."
#define ORIG_SAVENAME   "\poutput.txt"

#define ERROR_INFO      "\pAn error occurred: "

#define CALC_INFO       "\p..."
#define CANCEL_INFO     "\pOperation canceled."
#define TIME_2_INFO     "\p seconds ("

enum {
    /* Used in positioning dialogs. */
    kOpenTop            = 30,
    kOpenLeft           = 30,
    kDITop              = 80,
    kDILeft		        = 112,
    
    /* Initial window size */
    kWindowLeft         = 50,
    kWindowTop          = 50,
    kWindowRight        = 400,
    kWindowBottom       = 250,
    
    /* Window drag bounds */
    kWindowDragMargin   = 4,
    kMenubarHeight      = 20,

    /* kTextMargin is the number of pixels we leave blank at the edge of the window. */
    kTextMargin         = 2,
    
    /* kMaxDocWidth is an arbitrary number used to specify the width of the TERec's
	   destination rectangle so that word wrap and horizontal scrolling can be
	   demonstrated. */
    kMaxDocWidth        = 576,
    
    /* kMinDocDim is used to limit the minimum dimension of a window when GrowWindow
	   is called. */
    kMinDocDim          = 64,
    
    /* kScrollbarAdjust and kScrollbarWidth are used in calculating
	   values for control positioning and sizing. */
    kScrollbarWidth     = 16,
    kScrollbarAdjust    = 15,
    
    /* kScrollTweek compensates for off-by-one requirements of the scrollbars
       to have borders coincide with the growbox. */
    kScrollTweek        = 2,
    
    /* kControlInvisible is used to 'turn off' controls (i.e., cause the control not
	   to be redrawn as a result of some Control Manager call such as SetCtlValue)
	   by being put into the contrlVis field of the record. kControlVisible is used
	   the same way to 'turn on' the control. */
    kControlInvisible   = 0,
    kControlVisible     = 0xff
    
};

/* A DocumentRecord contains the WindowRecord for one of our document windows,
   as well as the TEHandle for the text we are editing. The window's scroll bars
   are also included. */
typedef struct {
	WindowRecord	docWindow;
	TEHandle		teHndl;
	ControlHandle	vScrollHndl;
	ControlHandle	hScrollHndl;
	Rect            dragBoundsRect;
	Rect            limitRect;
	Boolean         unsavedData;
} DocumentRecord, *DocumentPeek;