#include "checkTrap.h"
#include "globals.h"

/* Get the number of Toolbox traps based on InitGraf, which all ROMs have.
   Smaller ROMs will have InitGraf at 0xAA6E (kInitGrafTestTrap) */
static short NumToolboxTraps(void) {
    if ( NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(kInitGrafTestTrap, ToolTrap) ) {
        return kTrapCountSmall;
    } else {
        return kTrapCountBig;
    }
}

/* Checks if the trap is implemented. This is more kludgey than you would expect. */
Boolean IsTrapImplemented(long trap) {
    short       romSize;
    Boolean     isToolTrap;

    isToolTrap = (trap & kTrapTypeMask) != 0;
    
    if (NumToolboxTraps() >= kTrapCountBig) {
        romSize = LMGetROM85();
        
        /* 64K ROM */
        if (romSize < 0) {
           trap &= kTrapMask;
           
            if (isToolTrap) {
                if ((trap >= k64KROMToolTrapMin && trap <= k64KROMToolTrapMax) ||
                        trap != k64KROMOSTrapEx1 || trap != k64KROMOStrapEx2) {
                    return true;
                } else {
                    return false;
                }
            /* OSTrap instead */
            } else {
                if ((trap >= k64KROMOSTrapMin && trap <= k64KROMOSTrapMax) ||
                        trap == k64KROMOSTrapEx1 || trap == k64KROMOStrapEx2) {
                    return true;
                } else {
                    return false;
                }
            }
        } else {
            if (NGetTrapAddress(trap, isToolTrap) != NGetTrapAddress(_Unimplemented, ToolTrap)) {
                return true;
            }
        }
    } else {
        /* If the trap number is less than 0x200 and we have a small trap table (i.e. 0x1ff)
           then it's implemented. */
        if (isToolTrap && ((trap & kToolTrapMask) < kTrapCountSmall)) {
            return true;
        }
    }
    
    return false;
}