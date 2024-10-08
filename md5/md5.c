#include "md5.h"
#include "prefs.h"

#define memcpy(d, s, l) BlockMove(s, d, l)
#define memset(p, a, l) {unsigned long i; for (i = 0 ; i < (l) ; ++i) ((Ptr)(p))[i] = (a);}

volatile Boolean gIOComplete = false;

#ifdef __MC68K__
static void byteReverse(register unsigned char *buf, register unsigned longs) {
    asm {                                   // Timings
        movem.l     d0/d1/d7/a0, -(sp)      // 8+8(4)
        move.l      longs, d7               // 4
        move.l      buf, a0                 // 4
        subi.w      #1, d7                  // 8
    _loop:
        move.l      (a0), d0                // 12
        rol.l       #8, d0                  // 8+2(8)
        move.l      d0, d1                  // 4
        andi.l      #0x00ff00ff, d0         // 16
        swap        d1                      // 4
        andi.l      #0xff00ff00, d1         // 16
        or.l        d0, d1                  // 6
        move.l      d1, (a0)+               // 12
        dbf         d7, _loop               // 10
        movem.l     (sp)+, d0/d1/d7/a0      // 12+8(4)
                                            // --------
                                            // 204
    }
}
#endif

#ifdef __POWERPC__
static void byteReverse(register unsigned char *buf, register unsigned longs) {
    register long   index;
    register long   tmp;
    
    asm {
        li          index, 0
    _loop:
        lwbrx       tmp, index, buf
        subi        longs, longs, 1
        stwx        tmp, index, buf
        cmpi        longs, 0
        addi        index, index, 4
        bge        _loop
    }
}
#endif

#ifdef __MC68K__
static void MD5Transform(register uint32_t buf[4], register uint32_t in[16]) {
    #define dA      d0
    #define dB      d1
    #define dC      d2
    #define dD      d3
    #define dData   d4
    #define dS      d5
    #define dResult d6
    
    #define aBuff   a0
    #define aIn     a1

    /* MD5 functions */
    
    /* F1(x, y, z) (z ^ (x & (y ^ z))) */
    #define F1(x, y, z)                 \
        move.l      z, dResult;         \
        eor.l       y, dResult;         \
        and.l       x, dResult;         \
        eor.l       z, dResult;
    
    /* F2(x, y, z) F1(z, x, y) */
    #define F2(x, y, z)                 \
        F1      (z, x, y)
    
    /* F3(x, y, z) (x ^ y ^ z) */
    #define F3(x, y, z)                 \
        move.l      x, dResult;         \
        eor.l       y, dResult;         \
        eor.l       z, dResult;
        
    /* F4(x, y, z) (y ^ (x | ~z)) */
    #define F4(x, y, z)                 \
        move.l      z, dResult;         \
        not.l       dResult;            \
        or.l        x, dResult;         \
        eor.l       y, dResult;
            
    /* Core MD5 step */
    #define MD5STEP(F, w, x, y, z, index, c, s) \
        move.l       index(aIn), dData; \
        addi.l       %c, dData;         \
        moveq.l      %s, dS;            \
        /* Call the MD5 function */     \
        F            (x, y, z)          \
        add.l        dResult, w;        \
        add.l        dData, w;          \
        rol.l        dS, w;             \
        add.l        x, w;
    
    /* We can save a few clock cycles when there is no index */
    #define MD5STEP_INDEX0(F, w, x, y, z, c, s) \
        move.l       (aIn), dData;      \
        addi.l       %c, dData;         \
        moveq.l      %s, dS;            \
        /* Call the MD5 function */     \
        F            (x, y, z)          \
        add.l        dResult, w;        \
        add.l        dData, w;          \
        rol.l        dS, w;             \
        add.l        x, w;    
        
    asm {
        movem.l     d0-d6/a0-a1, -(sp);
        move.l      buf, aBuff;
        move.l      in, aIn;
        movem.l     (aBuff), dA-dD;
        
        MD5STEP_INDEX0(F1, dA, dB, dC, dD, 0xd76aa478, 7)
        MD5STEP(F1, dD, dA, dB, dC, 1 * 4, 0xe8c7b756, 12)
        MD5STEP(F1, dC, dD, dA, dB, 2 * 4, 0x242070db, 17)
        MD5STEP(F1, dB, dC, dD, dA, 3 * 4, 0xc1bdceee, 22)
        MD5STEP(F1, dA, dB, dC, dD, 4 * 4, 0xf57c0faf, 7)
        MD5STEP(F1, dD, dA, dB, dC, 5 * 4, 0x4787c62a, 12)
        MD5STEP(F1, dC, dD, dA, dB, 6 * 4, 0xa8304613, 17)
        MD5STEP(F1, dB, dC, dD, dA, 7 * 4, 0xfd469501, 22)
        MD5STEP(F1, dA, dB, dC, dD, 8 * 4, 0x698098d8, 7)
        MD5STEP(F1, dD, dA, dB, dC, 9 * 4, 0x8b44f7af, 12)
        MD5STEP(F1, dC, dD, dA, dB, 10 * 4, 0xffff5bb1, 17)
        MD5STEP(F1, dB, dC, dD, dA, 11 * 4, 0x895cd7be, 22)
        MD5STEP(F1, dA, dB, dC, dD, 12 * 4, 0x6b901122, 7)
        MD5STEP(F1, dD, dA, dB, dC, 13 * 4, 0xfd987193, 12)
        MD5STEP(F1, dC, dD, dA, dB, 14 * 4, 0xa679438e, 17)
        MD5STEP(F1, dB, dC, dD, dA, 15 * 4, 0x49b40821, 22)

        MD5STEP(F2, dA, dB, dC, dD, 1 * 4, 0xf61e2562, 5)
        MD5STEP(F2, dD, dA, dB, dC, 6 * 4, 0xc040b340, 9)
        MD5STEP(F2, dC, dD, dA, dB, 11 * 4, 0x265e5a51, 14)
        MD5STEP_INDEX0(F2, dB, dC, dD, dA, 0xe9b6c7aa, 20)
        MD5STEP(F2, dA, dB, dC, dD, 5 * 4, 0xd62f105d, 5)
        MD5STEP(F2, dD, dA, dB, dC, 10 * 4, 0x02441453, 9)
        MD5STEP(F2, dC, dD, dA, dB, 15 * 4, 0xd8a1e681, 14)
        MD5STEP(F2, dB, dC, dD, dA, 4 * 4, 0xe7d3fbc8, 20)
        MD5STEP(F2, dA, dB, dC, dD, 9 * 4, 0x21e1cde6, 5)
        MD5STEP(F2, dD, dA, dB, dC, 14 * 4, 0xc33707d6, 9)
        MD5STEP(F2, dC, dD, dA, dB, 3 * 4, 0xf4d50d87, 14)
        MD5STEP(F2, dB, dC, dD, dA, 8 * 4, 0x455a14ed, 20)
        MD5STEP(F2, dA, dB, dC, dD, 13 * 4, 0xa9e3e905, 5)
        MD5STEP(F2, dD, dA, dB, dC, 2 * 4, 0xfcefa3f8, 9)
        MD5STEP(F2, dC, dD, dA, dB, 7 * 4, 0x676f02d9, 14)
        MD5STEP(F2, dB, dC, dD, dA, 12 * 4, 0x8d2a4c8a, 20)

        MD5STEP(F3, dA, dB, dC, dD, 5 * 4, 0xfffa3942, 4)
        MD5STEP(F3, dD, dA, dB, dC, 8 * 4, 0x8771f681, 11)
        MD5STEP(F3, dC, dD, dA, dB, 11 * 4, 0x6d9d6122, 16)
        MD5STEP(F3, dB, dC, dD, dA, 14 * 4, 0xfde5380c, 23)
        MD5STEP(F3, dA, dB, dC, dD, 1 * 4, 0xa4beea44, 4)
        MD5STEP(F3, dD, dA, dB, dC, 4 * 4, 0x4bdecfa9, 11)
        MD5STEP(F3, dC, dD, dA, dB, 7 * 4, 0xf6bb4b60, 16)
        MD5STEP(F3, dB, dC, dD, dA, 10 * 4, 0xbebfbc70, 23)
        MD5STEP(F3, dA, dB, dC, dD, 13 * 4, 0x289b7ec6, 4)
        MD5STEP_INDEX0(F3, dD, dA, dB, dC, 0xeaa127fa, 11)
        MD5STEP(F3, dC, dD, dA, dB, 3 * 4, 0xd4ef3085, 16)
        MD5STEP(F3, dB, dC, dD, dA, 6 * 4, 0x04881d05, 23)
        MD5STEP(F3, dA, dB, dC, dD, 9 * 4, 0xd9d4d039, 4)
        MD5STEP(F3, dD, dA, dB, dC, 12 * 4, 0xe6db99e5, 11)
        MD5STEP(F3, dC, dD, dA, dB, 15 * 4, 0x1fa27cf8, 16)
        MD5STEP(F3, dB, dC, dD, dA, 2 * 4, 0xc4ac5665, 23)

        MD5STEP_INDEX0(F4, dA, dB, dC, dD, 0xf4292244, 6)
        MD5STEP(F4, dD, dA, dB, dC, 7 * 4, 0x432aff97, 10)
        MD5STEP(F4, dC, dD, dA, dB, 14 * 4, 0xab9423a7, 15)
        MD5STEP(F4, dB, dC, dD, dA, 5 * 4, 0xfc93a039, 21)
        MD5STEP(F4, dA, dB, dC, dD, 12 * 4, 0x655b59c3, 6)
        MD5STEP(F4, dD, dA, dB, dC, 3 * 4, 0x8f0ccc92, 10)
        MD5STEP(F4, dC, dD, dA, dB, 10 * 4, 0xffeff47d, 15)
        MD5STEP(F4, dB, dC, dD, dA, 1 * 4, 0x85845dd1, 21)
        MD5STEP(F4, dA, dB, dC, dD, 8 * 4, 0x6fa87e4f, 6)
        MD5STEP(F4, dD, dA, dB, dC, 15 * 4, 0xfe2ce6e0, 10)
        MD5STEP(F4, dC, dD, dA, dB, 6 * 4, 0xa3014314, 15)
        MD5STEP(F4, dB, dC, dD, dA, 13 * 4, 0x4e0811a1, 21)
        MD5STEP(F4, dA, dB, dC, dD, 4 * 4, 0xf7537e82, 6)
        MD5STEP(F4, dD, dA, dB, dC, 11 * 4, 0xbd3af235, 10)
        MD5STEP(F4, dC, dD, dA, dB, 2 * 4, 0x2ad7d2bb, 15)
        MD5STEP(F4, dB, dC, dD, dA, 9 * 4, 0xeb86d391, 21)
        
        add.l       dA, (aBuff)+
        add.l       dB, (aBuff)+
        add.l       dC, (aBuff)+
        add.l       dD, (aBuff)
        movem.l     (sp)+, d0-d6/a0-a1
    }
    
    #undef dA
    #undef dB
    #undef dC
    #undef dD
    #undef dData
    #undef dS
    #undef dResult
    
    #undef aBuff
    #undef aIn
}
#endif

#ifdef __POWERPC__
static void MD5Transform(register uint32_t buf[4], register uint32_t in[16]) {
    #define rBuf    r15
    #define rIn     r16
    
    #define rA      r5
    #define rB      r6
    #define rC      r7
    #define rD      r8
    #define rData   r9
    #define rResult r10
    #define rATmp   r11
    #define rBTmp   r12
    #define rCTmp   r13
    #define rDTmp   r14

    /* MD5 functions */
    
    /* F1(x, y, z) (z ^ (x & (y ^ z))) */
    #define F1(x, y, z)                         \
        xor         rResult, y, z;              \
        and         rResult, rResult, x;        \
        xor         rResult, rResult, z;
    
    /* F2(x, y, z) F1(z, x, y) */
    #define F2(x, y, z)                         \
        F1      (z, x, y)
    
    /* F3(x, y, z) (x ^ y ^ z) */
    #define F3(x, y, z)                         \
        xor         rResult, x, y;              \
        xor         rResult, rResult, z;
        
    /* F4(x, y, z) (y ^ (x | ~z)) */        
    #define F4(x, y, z)                         \
        not         rResult, z;                 \
        or          rResult, rResult, x;        \
        xor         rResult, rResult, y;
    
    /* Core MD5 step */
    #define MD5STEP(F, w, x, y, z, index, c, s) \
        lwz         rData, index(rIn);          \
        lis         rResult, c@h;               \
        ori         rResult, rResult, c@l;      \
        add         rData, rData, rResult;      \
        /* Call the MD5 function */             \
        F           (x, y, z)                   \
        add         w, w, rData;                \
        add         w, w, rResult;              \
        rlwinm      w, w, s, 0, 31;             \
        add         w, w, x;

    asm {
        lwz     rBuf, buf
        
        lwz     rA, 0x00(rBuf)
        lwz     rB, 0x04(rBuf)
        lwz     rC, 0x08(rBuf)
        lwz     rD, 0x0C(rBuf)
        
        lwz     rIn, in
        
        MD5STEP(F1, rA, rB, rC, rD, 0 * 4, 0xd76aa478, 7)        
        MD5STEP(F1, rD, rA, rB, rC, 1 * 4, 0xe8c7b756, 12)
        MD5STEP(F1, rC, rD, rA, rB, 2 * 4, 0x242070dB, 17)
        MD5STEP(F1, rB, rC, rD, rA, 3 * 4, 0xc1bdCeee, 22)
        MD5STEP(F1, rA, rB, rC, rD, 4 * 4, 0xf57c0faf, 7)
        MD5STEP(F1, rD, rA, rB, rC, 5 * 4, 0x4787c62a, 12)
        MD5STEP(F1, rC, rD, rA, rB, 6 * 4, 0xa8304613, 17)
        MD5STEP(F1, rB, rC, rD, rA, 7 * 4, 0xfd469501, 22)
        MD5STEP(F1, rA, rB, rC, rD, 8 * 4, 0x698098d8, 7)
        MD5STEP(F1, rD, rA, rB, rC, 9 * 4, 0x8b44f7af, 12)
        MD5STEP(F1, rC, rD, rA, rB, 10 * 4, 0xffff5bb1, 17)
        MD5STEP(F1, rB, rC, rD, rA, 11 * 4, 0x895cd7be, 22)
        MD5STEP(F1, rA, rB, rC, rD, 12 * 4, 0x6b901122, 7)
        MD5STEP(F1, rD, rA, rB, rC, 13 * 4, 0xfd987193, 12)
        MD5STEP(F1, rC, rD, rA, rB, 14 * 4, 0xa679438e, 17)
        MD5STEP(F1, rB, rC, rD, rA, 15 * 4, 0x49b40821, 22)

        MD5STEP(F2, rA, rB, rC, rD, 1 * 4, 0xf61e2562, 5)
        MD5STEP(F2, rD, rA, rB, rC, 6 * 4, 0xc040b340, 9)
        MD5STEP(F2, rC, rD, rA, rB, 11 * 4, 0x265e5a51, 14)
        MD5STEP(F2, rB, rC, rD, rA, 0 * 4, 0xe9b6c7aa, 20)
        MD5STEP(F2, rA, rB, rC, rD, 5 * 4, 0xd62f105d, 5)
        MD5STEP(F2, rD, rA, rB, rC, 10 * 4, 0x02441453, 9)
        MD5STEP(F2, rC, rD, rA, rB, 15 * 4, 0xd8a1e681, 14)
        MD5STEP(F2, rB, rC, rD, rA, 4 * 4, 0xe7d3fbc8, 20)
        MD5STEP(F2, rA, rB, rC, rD, 9 * 4, 0x21e1cde6, 5)
        MD5STEP(F2, rD, rA, rB, rC, 14 * 4, 0xc33707d6, 9)
        MD5STEP(F2, rC, rD, rA, rB, 3 * 4, 0xf4d50d87, 14)
        MD5STEP(F2, rB, rC, rD, rA, 8 * 4, 0x455a14ed, 20)
        MD5STEP(F2, rA, rB, rC, rD, 13 * 4, 0xa9e3e905, 5)
        MD5STEP(F2, rD, rA, rB, rC, 2 * 4, 0xfcefa3f8, 9)
        MD5STEP(F2, rC, rD, rA, rB, 7 * 4, 0x676f02d9, 14)
        MD5STEP(F2, rB, rC, rD, rA, 12 * 4, 0x8d2a4c8a, 20)

        MD5STEP(F3, rA, rB, rC, rD, 5 * 4, 0xfffa3942, 4)
        MD5STEP(F3, rD, rA, rB, rC, 8 * 4, 0x8771f681, 11)
        MD5STEP(F3, rC, rD, rA, rB, 11 * 4, 0x6d9d6122, 16)
        MD5STEP(F3, rB, rC, rD, rA, 14 * 4, 0xfde5380c, 23)
        MD5STEP(F3, rA, rB, rC, rD, 1 * 4, 0xa4beea44, 4)
        MD5STEP(F3, rD, rA, rB, rC, 4 * 4, 0x4bdecfa9, 11)
        MD5STEP(F3, rC, rD, rA, rB, 7 * 4, 0xf6bb4b60, 16)
        MD5STEP(F3, rB, rC, rD, rA, 10 * 4, 0xbebfbc70, 23)
        MD5STEP(F3, rA, rB, rC, rD, 13 * 4, 0x289b7ec6, 4)
        MD5STEP(F3, rD, rA, rB, rC, 0 * 4, 0xeaa127fa, 11)
        MD5STEP(F3, rC, rD, rA, rB, 3 * 4, 0xd4ef3085, 16)
        MD5STEP(F3, rB, rC, rD, rA, 6 * 4, 0x04881d05, 23)
        MD5STEP(F3, rA, rB, rC, rD, 9 * 4, 0xd9d4d039, 4)
        MD5STEP(F3, rD, rA, rB, rC, 12 * 4, 0xe6dB99e5, 11)
        MD5STEP(F3, rC, rD, rA, rB, 15 * 4, 0x1fa27cf8, 16)
        MD5STEP(F3, rB, rC, rD, rA, 2 * 4, 0xc4ac5665, 23)

        MD5STEP(F4, rA, rB, rC, rD, 0 * 4, 0xf4292244, 6)
        MD5STEP(F4, rD, rA, rB, rC, 7 * 4, 0x432aff97, 10)
        MD5STEP(F4, rC, rD, rA, rB, 14 * 4, 0xab9423a7, 15)
        MD5STEP(F4, rB, rC, rD, rA, 5 * 4, 0xfc93a039, 21)
        MD5STEP(F4, rA, rB, rC, rD, 12 * 4, 0x655b59c3, 6)
        MD5STEP(F4, rD, rA, rB, rC, 3 * 4, 0x8f0ccc92, 10)
        MD5STEP(F4, rC, rD, rA, rB, 10 * 4, 0xffeff47d, 15)
        MD5STEP(F4, rB, rC, rD, rA, 1 * 4, 0x85845dd1, 21)
        MD5STEP(F4, rA, rB, rC, rD, 8 * 4, 0x6fa87e4f, 6)
        MD5STEP(F4, rD, rA, rB, rC, 15 * 4, 0xfe2ce6e0, 10)
        MD5STEP(F4, rC, rD, rA, rB, 6 * 4, 0xa3014314, 15)
        MD5STEP(F4, rB, rC, rD, rA, 13 * 4, 0x4e0811a1, 21)
        MD5STEP(F4, rA, rB, rC, rD, 4 * 4, 0xf7537e82, 6)
        MD5STEP(F4, rD, rA, rB, rC, 11 * 4, 0xbd3af235, 10)
        MD5STEP(F4, rC, rD, rA, rB, 2 * 4, 0x2ad7d2bb, 15)
        MD5STEP(F4, rB, rC, rD, rA, 9 * 4, 0xeb86d391, 21)
        
        lwz         rATmp, 0x00(rBuf)
        lwz         rBTmp, 0x04(rBuf)
        lwz         rCTmp, 0x08(rBuf)
        lwz         rDTmp, 0x0C(rBuf)
        
        add         rA, rA, rATmp
        add         rB, rB, rBTmp
        add         rC, rC, rCTmp
        add         rD, rD, rDTmp
        
        stw         rA, 0x00(rBuf)
        stw         rB, 0x04(rBuf)
        stw         rC, 0x08(rBuf)
        stw         rD, 0x0C(rBuf)
    }
    
    #undef rA
    #undef rB
    #undef rC
    #undef rD
    #undef rData
    #undef rResult
    #undef rATmp
    #undef rBTmp
    #undef rCTmp
    #undef rDTmp
    
    #undef rBuf
    #undef rIn
}
#endif


/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void MD5Init(MD5Context *ctx) {
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void MD5Update(MD5Context *ctx, const void *data, unsigned len) {
    const unsigned char *buf = data;
    uint32_t t;

    /* Update bitcount */

    t = ctx->bits[0];
    
    if ((ctx->bits[0] = t + ((uint32_t) len << 3)) < t)
	    ctx->bits[1]++; 	/* Carry from low to high */
	    
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
    	unsigned char *p = (unsigned char *) ctx->in + t;

    	t = 64 - t;
    	
    	if (len < t) {
    	    memcpy(p, buf, len);
    	    return;
    	}
    	
    	memcpy(p, buf, t);
    	byteReverse(ctx->in, 16);
    	MD5Transform(ctx->buf, (uint32_t *) ctx->in);
    	buf += t;
    	len -= t;
    }
    /* Process data in 64-byte chunks */

    while (len >= 64) {
    	memcpy(ctx->in, buf, 64);
    	byteReverse(ctx->in, 16);
    	MD5Transform(ctx->buf, (uint32_t *) ctx->in);
    	buf += 64;
    	len -= 64;
    }

    /* Handle any remaining bytes of data. */
    memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
static void MD5Final(unsigned char digest[16], MD5Context *ctx) {
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
    always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
        /* Two lots of padding:  Pad the first block to 64 bytes */
        memset(p, 0, count);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (uint32_t *) ctx->in);

        /* Now fill the next block with 56 bytes */
        memset(ctx->in, 0, 56);
    } else {
        /* Pad block to 56 bytes */
        memset(p, 0, count - 8);
    }
    
    byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((uint32_t *) ctx->in)[14] = ctx->bits[0];
    ((uint32_t *) ctx->in)[15] = ctx->bits[1];

    MD5Transform(ctx->buf, (uint32_t *) ctx->in);
    byteReverse((unsigned char *) ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    //memset(ctx, 0, sizeof(MD5Context));        /* In case it's sensitive */
}

/* This is a different version of the MD5MacFile function that adds the API
   calls to support >2GB file sizes */
short MD5MacFileFork(FSForkIOParamPtr openFSForkPtr, unsigned char *result) {
    FSForkIOParamPtr    currFSForkPtr;
    FSForkIOParamPtr    fsForkPtr[2];
	MD5Context          ctx;
    long                count = 0xffff;
    char                currFSForkIndex = 0;
	
	MD5Init(&ctx);
		
	fsForkPtr[0] = (FSForkIOParamPtr)NewPtrClear(sizeof(FSForkIOParam));
	fsForkPtr[1] = (FSForkIOParamPtr)NewPtrClear(sizeof(FSForkIOParam));
	SetupFSForkIOParam(fsForkPtr[0], openFSForkPtr, count);
	SetupFSForkIOParam(fsForkPtr[1], openFSForkPtr, count);
	PBReadForkAsync(fsForkPtr[0]);
	
	/* Wait for the I/O operation to complete */
	while (fsForkPtr[0]->ioResult > 0) {}
    
	PBReadForkAsync(fsForkPtr[1]);    	
	currFSForkPtr = fsForkPtr[0];
	
    while (true) {
	    /* Wait for the I/O operation to complete */
	    while (currFSForkPtr->ioResult > 0) {}
	    
	    /* Data ready */
	    
	    /* Stop reading if no more data */
	    if (currFSForkPtr->actualCount <= 0) {
	        break;
	    }
	    
	    /* Checksum the data */   
        MD5Update(&ctx, (unsigned char *)currFSForkPtr->buffer, currFSForkPtr->actualCount);

	    /* Usually an eofErr (-43) will trigger this break */
        if (currFSForkPtr->ioResult < 0) {
            break;
        }

    	/* Put the buffer back into the queue */
    	PBReadForkAsync(currFSForkPtr);
    	
    	/* Switch to the other buffer */
    	currFSForkIndex = 1 - currFSForkIndex;
    	currFSForkPtr = fsForkPtr[currFSForkIndex];
	}

    DestroyFSForkIOParam(fsForkPtr[0]);
    DestroyFSForkIOParam(fsForkPtr[1]);

    MD5Final(result, &ctx);

	return 1;
}

static void SetupFSForkIOParam(FSForkIOParamPtr readFSForkPtr, FSForkIOParamPtr openFSForkPtr, unsigned long bufferSize) {
    readFSForkPtr->ioCompletion = NULL;
    
    /* Any positive integer here will do */
    readFSForkPtr->ioResult = ioInProgress;
    readFSForkPtr->ioCompletion = NULL;
    readFSForkPtr->forkRefNum = openFSForkPtr->forkRefNum;
    readFSForkPtr->positionMode = fsAtMark;
    readFSForkPtr->positionOffset = 0LL;
    readFSForkPtr->requestCount = bufferSize;
    readFSForkPtr->buffer = NewPtr(bufferSize);
}

static void DestroyFSForkIOParam(FSForkIOParamPtr fsForkPtr) {
    DisposePtr(fsForkPtr->buffer);
    DisposePtr((Ptr)fsForkPtr);
}

short MD5MacFile(ParmBlkPtr pbOpenBlkPtr, unsigned char *result) {
    ParmBlkPtr  currPBPtr;
    ParmBlkPtr  pbPtr[2];
	MD5Context  ctx;
    long        count = 0xffff;
    char        currPBIndex = 0;
	
	MD5Init(&ctx);
		
	pbPtr[0] = (ParmBlkPtr)NewPtrClear(sizeof(ParamBlockRec));
	pbPtr[1] = (ParmBlkPtr)NewPtrClear(sizeof(ParamBlockRec));
	SetupParamBlk(pbPtr[0], pbOpenBlkPtr, count);
	SetupParamBlk(pbPtr[1], pbOpenBlkPtr, count);
	
	PBReadAsync(pbPtr[0]);
	PBReadAsync(pbPtr[1]);
	currPBPtr = pbPtr[0];
	
	while (true) {
	    /* Wait for the I/O operation to complete */
	    while (currPBPtr->ioParam.ioResult > 0) {};
	    
	    /* Data ready */
	    
	    /* Stop reading if no more data */
	    if (currPBPtr->ioParam.ioActCount == 0) {
	        break;
	    }
	    
	    /* Checksum the data */   
        MD5Update(&ctx, (unsigned char *)currPBPtr->ioParam.ioBuffer, currPBPtr->ioParam.ioActCount);
    	
    	/* Put the buffer back into the queue */
    	PBReadAsync(currPBPtr);
    	
    	/* Switch to the other buffer */
    	currPBIndex = 1 - currPBIndex;
    	currPBPtr = pbPtr[currPBIndex];
	}
	
    DestroyParamBlk(pbPtr[0]);
    DestroyParamBlk(pbPtr[1]);
    MD5Final(result, &ctx);
	
	return 1;
}

static void SetupParamBlk(ParmBlkPtr pbReadBlkPtr, ParmBlkPtr pbOpenBlkPtr, long bufferSize) {
    pbReadBlkPtr->ioParam.ioCompletion = NULL;
    pbReadBlkPtr->ioParam.ioResult = ioInProgress;
    pbReadBlkPtr->ioParam.ioRefNum = pbOpenBlkPtr->ioParam.ioRefNum;
    pbReadBlkPtr->ioParam.ioVRefNum = pbOpenBlkPtr->ioParam.ioVRefNum;
    pbReadBlkPtr->ioParam.ioReqCount = bufferSize;
    pbReadBlkPtr->ioParam.ioBuffer = NewPtr(bufferSize);
    pbReadBlkPtr->ioParam.ioPosMode = fsAtMark;
    pbReadBlkPtr->ioParam.ioPosOffset = 0L;
}

static void DestroyParamBlk(ParmBlkPtr pbPtr) {
    DisposePtr(pbPtr->ioParam.ioBuffer);
    DisposePtr((Ptr)pbPtr);
}