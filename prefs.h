#include <MacTypes.h>
#include <Resources.h>

#define PREFS_RES_TYPE  'PREF'

/* Preferences are stored in a single 16-bit short */
#define DEFAULT_PREFS   0x0000

enum {
    kPrefsResID     = 128
};

typedef short PrefsBitField;
typedef PrefsBitField * PrefsBitFieldPtr;

typedef struct {
    unsigned short      askToSave : 1;
    unsigned short      autosaveHash : 1;
    unsigned short      uppercaseHash : 1;
    unsigned short      digitGrouping : 1;
    unsigned short      verbose : 1;
} Prefs;

extern Prefs    gPrefs;

void InitPreferences(void);
void SavePreferences(void);