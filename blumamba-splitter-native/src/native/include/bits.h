#include <jni.h>

#ifndef _bits
#define _bits
#ifdef __cplusplus
extern "C" {
#endif

#define BYTE_BITS 8
#define LONG_BYTES 8
#define LOG_LONG_BYTES 3

jlong getLong(const jbyte *in, const size_t pos);
void putLong(jbyte *out, const size_t pos, jlong value);

#ifdef __cplusplus
}
#endif
#endif

