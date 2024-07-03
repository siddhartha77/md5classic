#include <Memory.h>
#include <Types.h>

#define HIWORD(x) \
    x >> 16

#define LOWORD(x) \
    x & 0xffff

void myPStrToCStr(StringPtr s);
void myCopyPStr(const Str255 s,Str255 t);
void appendPStr(Str255 s, const Str255 suffixStr);
unsigned char myValToBaseXChar(unsigned short v);
void myMD5ValsToHexChars(unsigned char *s, Str31 t);