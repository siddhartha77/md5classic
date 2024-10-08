#include "main.h"
#include "globals.h"
#include "myMenus.h"
#include "prefs.h"
#include "myTextEdit.h"
#include "utils.h"

/* Setup the menus. These are pulled in from the resource file. */
void SetupMenus(void) {
    MenuHandle  appleMenuHndl;
    MenuHandle  fileMenuHndl;
    MenuHandle  editMenuHndl;
    MenuHandle  prefsMenuHndl;
    
    appleMenuHndl = GetMenu(kAppleMenuResID);
    AppendResMenu(appleMenuHndl, kAppleMenuDAType);
    InsertMenu(appleMenuHndl, 0);
    
    fileMenuHndl = GetMenu(kFileMenuResID);
    InsertMenu(fileMenuHndl, 0);
    
    editMenuHndl = GetMenu(kEditMenuResID);
    InsertMenu(editMenuHndl, 0);
    
    prefsMenuHndl = GetMenu(kPrefsMenuResID);
    InsertMenu(prefsMenuHndl, 0);
            
    CheckItem(prefsMenuHndl, kPrefsMenuAskToSave, gPrefs.askToSave);
    CheckItem(prefsMenuHndl, kPrefsMenuAutosaveHash, gPrefs.autosaveHash);
    CheckItem(prefsMenuHndl, kPrefsMenuUppercaseHash, gPrefs.uppercaseHash);
    CheckItem(prefsMenuHndl, kPrefsMenuDigitGrouping, gPrefs.digitGrouping);
    CheckItem(prefsMenuHndl, kPrefsMenuVerbose, gPrefs.verbose);
    
    DrawMenuBar();
}

/* Handle menu clicks. */
void DoMenu(long menuResult) {
    Str255          itemName;
    GrafPtr         currentPort;
    MenuHandle      menuHndl;
    CharParameter   menuItemMark;
    short           menuID;
    short           menuItem;
    
    menuID = HiWord(menuResult);
    menuItem = LoWord(menuResult);
    
    switch (menuID) {
        case kAppleMenuResID :
            switch (menuItem) {
                case kAppleMenuAbout:
                    NoteAlert(kAboutBoxResID, NULL);
                    
                    break;                    
                default:
                    GetMenuItemText(GetMenuHandle(kAppleMenuResID), menuItem, itemName);
                    GetPort(&currentPort);
				    OpenDeskAcc(itemName);
				    SetPort(currentPort);
				    			    
				    break;
            }
            
            break;
            
        case kFileMenuResID:
            switch (menuItem) {
                case kFileMenuOpen:
                    DoOpen();
                    
                    break;
                case kFileMenuClose:
                    DoQuit();
                    
                    break;
                case kFileMenuSaveAs:
                    DoSaveAs();
                    
                    break;
                case kFileMenuQuit:
                    DoQuit();
                    
                    break;
            }
            
            break;
            
        case kEditMenuResID:
            switch (menuItem) {
                case kEditMenuCopy:
                    TECopy(((DocumentPeek)FrontWindow())->teHndl);
                    ZeroScrap();
                    TEToScrap();
                    
                    break;                    
                case kEditMenuSelectAll:
                    TESetSelect(0, (*((DocumentPeek)FrontWindow())->teHndl)->teLength, ((DocumentPeek)FrontWindow())->teHndl);
                    
                    break;
            }
            
            break;
            
        case kPrefsMenuResID:
            menuHndl = GetMenu(kPrefsMenuResID);
            GetItemMark(menuHndl, menuItem, &menuItemMark);
            CheckItem(menuHndl, menuItem, (menuItemMark == noMark));
            
            switch (menuItem) {
                case kPrefsMenuAskToSave:
                    gPrefs.askToSave = (menuItemMark == noMark);
                
                    break;
                case kPrefsMenuAutosaveHash:
                    gPrefs.autosaveHash = (menuItemMark == noMark);
                
                    break;
                case kPrefsMenuUppercaseHash:
                    gPrefs.uppercaseHash = (menuItemMark == noMark);
                
                    break;
                case kPrefsMenuDigitGrouping:
                    gPrefs.digitGrouping = (menuItemMark == noMark);
                
                    break;
                case kPrefsMenuVerbose:
                    gPrefs.verbose = (menuItemMark == noMark);
                
                    break;
            }
            
            SavePreferences();
            
            break;
    }
    
    HiliteMenu(0);
}