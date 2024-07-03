#include <stdint.h>

#include <Events.h>
#include <Files.h>
#include <Memory.h>

typedef unsigned int MD5_u32plus;

typedef struct {
    MD5_u32plus lo, hi;
    MD5_u32plus a, b, c, d;
    unsigned char buffer[64];
    MD5_u32plus block[16];
} MD5_CTX;

static void MD5_Init(MD5_CTX *ctx);
static void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
static void MD5_Final(unsigned char *result, MD5_CTX *ctx);

short md5MacFile(short refNum, uint8_t *result);