#include "utils.h"

void myPStrToCStr(StringPtr s) {
    short i;
    short len = s[0];
    
    for (i = 1 ; i <= len ; ++i) {
        s[i - 1] = s[i];
    }
    
    s[len] = '\0';
}

void myCopyPStr(const Str255 s,Str255 t) {
	BlockMove((Ptr) s,(Ptr) t,s[0]+1);
}

/*************************************************
    Append a PString with a PString.
        s:          A pointer to the source PString.
        suffixStr:  A pointer to the destination PString.
        Returns:    Nothing.
*************************************************/
void appendPStr(Str255 s, const Str255 suffixStr) {
    register unsigned short	i = s[0];
    register unsigned short	j = suffixStr[0];
	
	if (j) {
		if ((i+j) <= 255) {
			BlockMove((Ptr)&suffixStr[1], (Ptr)&s[i + 1], j);
			s[0]+=j;
		} else {
		    DebugStr("\pOverflow");
		}
	}
}

unsigned char myValToBaseXChar(unsigned short v) {
	if (v<10) return ('0'+v);		//00..09  -> '0'..'9'
	if (v<36) return ('a'-10+v);	//10..35  -> 'A'..'Z'
	
	return ('A'-36+v);				//36..61+ -> 'a'..'z'+
}

void myMD5ValsToHexChars(unsigned char *s, Str32 t) {
    short i;
    short j;
    
    t[0] = 32;
            
    for (i = 0, j = 1 ; i < 16 ; ++i) {
         t[j++] = myValToBaseXChar(s[i] >> 4);
         t[j++] = myValToBaseXChar(s[i] & 0xf);
    }
}