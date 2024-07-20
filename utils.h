#include <Memory.h>
#include <Types.h>

void myPStrToCStr(StringPtr s);
void myCopyPStr(const Str255 s,Str255 t);
void myAppendPStr(Str255 s, const Str255 suffixStr);
unsigned char myValToBaseXLCaseChar(unsigned short v);
unsigned char myValToBaseXUCaseChar(unsigned short v);
void myMD5ValsToHexChars(unsigned char *s, Str31 t, Boolean uppercase);
void myDeleteElementFromPStr(Str255 s, unsigned short index);
void mySafeFilename(Str255 s);