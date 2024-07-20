#include "utils.h"

/* Converts a PString to a CString. */
void myPStrToCStr(StringPtr s) {
    short i;
    short len = s[0];
    
    for (i = 1 ; i <= len ; ++i) {
        s[i - 1] = s[i];
    }
    
    s[len] = '\0';
}

/* Copy a PString. */
void myCopyPStr(const Str255 s,Str255 t) {
	BlockMove((Ptr) s,(Ptr) t,s[0]+1);
}

/* Append a PString to another PString. */
void myAppendPStr(Str255 s, const Str255 suffixStr) {
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

/* Convert a number to any base using uppercase letters. */
unsigned char myValToBaseXUCaseChar(unsigned short v) {
	if (v<10) return ('0'+v);		//00..09  -> '0'..'9'
	if (v<36) return ('A'-10+v);	//10..35  -> 'A'..'Z'
	
	return ('a'-36+v);				//36..61+ -> 'a'..'z'+
}

/* Convert a number to any base using lowercase letters. */
unsigned char myValToBaseXLCaseChar(unsigned short v) {
	if (v<10) return ('0'+v);		//00..09  -> '0'..'9'
	if (v<36) return ('a'-10+v);	//10..35  -> 'a'..'z'
	
	return ('A'-36+v);				//36..61+ -> 'A'..'Z'+
}

/* Converts 32 MD5 values into 16 hexadecimal values. */
void myMD5ValsToHexChars(unsigned char *s, Str32 t, Boolean uppercase) {
    short i;
    short j;
    
    t[0] = 32;
    
    if (uppercase) {
        for (i = 0, j = 1 ; i < 16 ; ++i) {
             t[j++] = myValToBaseXUCaseChar(s[i] >> 4);
             t[j++] = myValToBaseXUCaseChar(s[i] & 0xf);
        }
    } else {
        for (i = 0, j = 1 ; i < 16 ; ++i) {
             t[j++] = myValToBaseXLCaseChar(s[i] >> 4);
             t[j++] = myValToBaseXLCaseChar(s[i] & 0xf);
        }
    }
}

/* Deleted the char at the specified index from a PString. */
void myDeleteElementFromPStr(Str255 s, unsigned short index) {
    register unsigned short		i, len = s[0];

	if (index <= len)
	{
		for (i = index ; i < len ; i++) s[i] = s[i + 1];
		s[0]--;
	}
}

/* Truncate filename and add ellipsis if necessary.
   Also preserves the file extension. */
void mySafeFilename(Str255 s) {
    #define         MAX_FILENAME_LEN    31
    #define         MAX_EXTENSION_LEN   4
    
    int             i;
    int             extensionRIndex = 0;

    if (s[0] > MAX_FILENAME_LEN)
    {
        for (i = s[0] ; i > s[0] - MAX_EXTENSION_LEN ; --i)
        {
            if (s[i] == '.')
            {
                extensionRIndex = s[0] - i + 1;
                break;
            }
        }
        
        while (s[0] > MAX_FILENAME_LEN) {
            myDeleteElementFromPStr(s, s[0] - extensionRIndex);
        }
        
        s[s[0] - extensionRIndex] = 'É';
    }
}