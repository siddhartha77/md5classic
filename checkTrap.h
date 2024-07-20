#include <Gestalt.h>
#include <LowMem.h>
#include <Traps.h>

enum {
    kTrapTypeMask       = 0x0800,
    kTrapMask           = 0x07ff,
    kToolTrapMask       = 0x03ff
};

enum {
    kInitGrafTestTrap   = 0xAA6E
};

enum {
    kTrapCountSmall     = 0x200,
    kTrapCountBig       = 0x400
};

enum {
    k64KROMOSTrapMin    = 0x000,
    k64KROMOSTrapMax    = 0x04f,
    
    /* These two traps are also in a 64K ROM */
    k64KROMOSTrapEx1    = _UprString,
    k64KROMOStrapEx2    = _SetAppBase,
    
    k64KROMToolTrapMin  = 0x050,
    k64KROMToolTrapMax  = 0x19f
};

static short NumToolboxTraps(void);
Boolean IsTrapImplemented(long trap);