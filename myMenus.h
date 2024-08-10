#include <Controls.h>
#include <Devices.h>
#include <Dialogs.h>
#include <Files.h>
#include <Menus.h>
#include <Scrap.h>
#include <StandardFile.h>
#include <ToolUtils.h>

enum {
    kAboutBoxResID      = 128
};

enum {
    kAppleMenuResID     = 1,
    kFileMenuResID,
    kEditMenuResID,
    kPrefsMenuResID
};

enum {
    kAppleMenuDAType    = 'DRVR'
};

enum {
    kAppleMenuAbout     = 1
};

enum {
    kFileMenuOpen       = 1,
    kFileMenuClose      = 3,
    kFileMenuSaveAs     = 4,
    kFileMenuQuit       = 6
};

enum {
    kEditMenuCut        = 1,
    kEditMenuCopy       = 2,
    kEditMenuSelectAll  = 6
};

enum {
    kPrefsMenuAskToSave     = 1,
    kPrefsMenuAutosaveHash,
    kPrefsMenuUppercaseHash,
    kPrefsMenuDigitGrouping,
    kPrefsMenuVerbose
};

void SetupMenus(void);
void DoMenu(long menuResult);