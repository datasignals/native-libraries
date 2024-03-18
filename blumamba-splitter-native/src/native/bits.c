#include <stdlib.h>

#include "bits.h"
#include "debug.h"


// a) As Mac OS X does not have byteswap.h
// needed this for a c util I had used over the years on linux. 
// did not find a solution to stopgap via macports, sadly, but this did the trick

#if HAVE_BYTESWAP_H
#include <byteswap.h>
#else
#define bswap_16(value) \
((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
(((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
(uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define bswap_64(value) \
(((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) \
<< 32) | \
(uint64_t)bswap_32((uint32_t)((value) >> 32)))
#endif

jlong getLong(const jbyte *in, const size_t pos) {

    #ifdef TRACE
    printf("getLong at %ld\n", pos);
    for(int i = 0; i < LONG_BYTES; i++) printf("in[%ld] = %d\n", pos + i, in[pos + i]);
    #endif

    return bswap_64(*(jlong *)(&(in[pos])));
}

void putLong(jbyte *out, const size_t pos, jlong value) {

    #ifdef TRACE
    printf("putLong(...,%ld, %ld){\n", pos, value);
    #endif

    size_t max = pos + LONG_BYTES - 1;
    for(size_t i = 0; i < LONG_BYTES; i++) {
        out[max - i] = (jbyte)(value & 0xffl);
        value >>= BYTE_BITS;
    }

    #ifdef TRACE
    printf("}putLong(..., %ld, %ld)\n", pos, value);
    #endif
}