#include <stdint.h>

#include <Devices.h>
#include <Events.h>
#include <Memory.h>

#define MD5_DIGEST_SIZE 16

typedef struct {
    uint32_t buf[4];
    uint32_t bits[2];
    unsigned char in[64];
} MD5Context;


static void MD5Init(MD5Context *ctx);
static void MD5Update(MD5Context *ctx, const void *buf, unsigned len);
static void MD5Transform(register uint32_t buf[4], register uint32_t in[16]);
static void MD5Final(unsigned char digest[MD5_DIGEST_SIZE], MD5Context *ctx);

short MD5MacFile(ParmBlkPtr pbOpenBlkPtr, unsigned char *result);
static void SetupParamBlk(ParmBlkPtr pbReadBlkPtr, ParmBlkPtr pbOpenBlkPtr, long bufferSize);
static void DestroyParamBlk(ParmBlkPtr pbPtr);