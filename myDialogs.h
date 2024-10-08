#include <Dialogs.h>
#include <MacTypes.h>

enum {
    kConfirmDialogResID             = 129,
    kConfirmDialogButtonOK          = 1,
    kConfirmDialogButtonCancel      = 2,
    kConfirmDialogButtonDontSave    = 3
};

pascal Boolean ConfirmQuitDialogEventFilter(DialogPtr whichDialog, EventRecord event, short itemHit);