#include "prefs.h"

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
    
    gPrefs.askToSave = (prefs & 0x0001) > 0;
    gPrefs.autosaveHash = (prefs & 0x0002) > 0;
    gPrefs.uppercaseHash = (prefs & 0x0004) > 0;
    gPrefs.verbose = (prefs & 0x0008) > 0;
    

    DisposeHandle(prefsHndl);
}

/* Save the preferences to the application resource fork. */
void SavePreferences() {
    Handle  prefsHndl;
    short   prefs = 0;
    
    prefs = gPrefs.askToSave;
    prefs |= gPrefs.autosaveHash << 1;
    prefs |= gPrefs.uppercaseHash << 2;
    prefs |= gPrefs.verbose << 3;
    
    prefsHndl = GetResource(PREFS_RES_TYPE, kPrefsResID);
    
    if (prefsHndl != NULL) {
        *((PrefsBitFieldPtr)(*prefsHndl)) = prefs;
        ChangedResource(prefsHndl);
        WriteResource(prefsHndl);
    }
}