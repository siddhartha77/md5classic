#include "checkTrap.h"
#include "globals.h"

static short NumToolboxTraps(void) {
    if ( NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(kInitGrafTestTrap, ToolTrap) ) {
        return kTrapCountSmall;
    } else {
        return kTrapCountBig;
    }
}

Boolean IsTrapImplemented(long trap) {
    short       romSize;
    Boolean     isToolTrap;
    
    isToolTrap = (trap & kTrapTypeMask) == 0;
    
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
    }
    
    return false;
}