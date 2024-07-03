#include <stdint.h>

#include <Events.h>
#include <Files.h>
#include <Memory.h>

typedef struct{
	uint64_t size;			// Size of input in bytes
	uint32_t buffer[4];		// Current accumulation of hash
	uint8_t input[64];		// Input to be used in the next step
	uint8_t digest[16];		// Result of algorithm
}MD5Context;

void md5Init(MD5Context *ctx);
void md5Update(MD5Context *ctx, uint8_t *input, uint32_t input_len);
void md5Finalize(MD5Context *ctx);
void md5Step(uint32_t *buffer, uint32_t *input);

short md5MacFile(short refNum, uint8_t *result);

#define ROL(x, n) \
    ((x << n) | (x >> (32 - n)))

#define F(X, Y, Z) \
    ((X & Y) | (~X & Z))
    
#define G(X, Y, Z) \
    ((X & Z) + (Y & ~Z))
    
#define H(X, Y, Z) \
    (X ^ Y ^ Z)

#define I(X, Y, Z) \
    (Y ^ (X | ~Z))