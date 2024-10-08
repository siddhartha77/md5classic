#include "prefs.h"
#include "myMenus.h"

Prefs   gPrefs;

/* Initialize the preferences.
   Prefs is just a bitfield struct (2 bytes) of the different preferences. */
void InitPreferences() {
    Handle          prefsHndl;
    short           resRefNum;
    PrefsBitField   prefs = DEFAULT_PREFS;
    
    resRefNum = CurResFile();
    prefsHndl = GetResource(PREFS_RES_TYPE, kPrefsResID);
    
    if (prefsHndl == NULL) {
        prefsHndl = NewHandle(sizeof(prefs));
        *((PrefsBitFieldPtr)(*prefsHndl)) = prefs;
        AddResource(prefsHndl, PREFS_RES_TYPE, kPrefsResID, NULL);
        UpdateResFile(resRefNum);
    }
    
    prefs = *((PrefsBitFieldPtr)(*prefsHndl));
    
    gPrefs.askToSave        = (prefs & (1 << kPrefsMenuAskToSave - 1)) > 0;
    gPrefs.autosaveHash     = (prefs & (1 << kPrefsMenuAutosaveHash - 1)) > 0;
    gPrefs.uppercaseHash    = (prefs & (1 << kPrefsMenuUppercaseHash - 1)) > 0;
    gPrefs.digitGrouping    = (prefs & (1 << kPrefsMenuDigitGrouping - 1)) > 0;
    gPrefs.verbose          = (prefs & (1 << kPrefsMenuVerbose - 1)) > 0;
    

    DisposeHandle(prefsHndl);
}

/* Save the preferences to the application resource fork. */
void SavePreferences() {
    Handle  prefsHndl;
    short   prefs = 0;
    
    prefs = gPrefs.askToSave        << (kPrefsMenuAskToSave - 1);
    prefs |= gPrefs.autosaveHash    << (kPrefsMenuAutosaveHash - 1);
    prefs |= gPrefs.uppercaseHash   << (kPrefsMenuUppercaseHash - 1);
    prefs |= gPrefs.digitGrouping   << (kPrefsMenuDigitGrouping - 1);
    prefs |= gPrefs.verbose         << (kPrefsMenuVerbose - 1);
    
    prefsHndl = GetResource(PREFS_RES_TYPE, kPrefsResID);
    
    if (prefsHndl != NULL) {
        *((PrefsBitFieldPtr)(*prefsHndl)) = prefs;
        ChangedResource(prefsHndl);
        WriteResource(prefsHndl);
    }
}