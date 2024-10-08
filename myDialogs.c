#include "myDialogs.h"
#include "myMenus.h"

pascal Boolean ConfirmQuitDialogEventFilter(DialogPtr whichDialog, EventRecord event, short itemHit) {
    while (!itemHit) {
        ModalDialog(NULL, &itemHit);
    }
    
    switch (itemHit) {
       case kConfirmDialogButtonOK:
           if (DoSaveAs()) {    
               ExitToShell();
           }
           
           break;
       case kConfirmDialogButtonCancel:
           break;
       case kConfirmDialogButtonDontSave:  
           ExitToShell();
           
           break;       
   }
}