#include <Controls.h>
#include <Devices.h>
#include <Dialogs.h>
#include <Files.h>
#include <Menus.h>
#include <Scrap.h>
#include <StandardFile.h>

enum {
    kAboutBoxResID      = 128
};

enum {
    kAppleMenuResID     = 1,
    kFileMenuResID,
    kEditMenuResID
};

enum {
    kAppleMenuDAType    = 'DRVR'
};

enum {
    kAppleMenuAbout     = 1
};

enum {
    kFileMenuOpen       = 1,
    kFileMenuSaveAs     = 3,
    kFileMenuQuit       = 5
};

enum {
    kEditMenuCut        = 1,
    kEditMenuCopy       = 2,
    kEditMenuSelectAll  = 6
};

void SetupMenus(void);
void DoMenu(long menuResult);
void DoOpen(void);
void DoSaveAs(void);
void DoQuit(void);