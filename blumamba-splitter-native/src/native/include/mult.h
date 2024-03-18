#include <jni.h>

#ifndef _mult
#define _mult
#ifdef __cplusplus
extern "C" {
#endif

jlong mult(const size_t dimension, const jlong *matrix, const size_t column, const jbyte *input, const size_t inPosition);
jlong mmult(const size_t dimension, const jlong *matrix, const size_t column, jbyte * const * const inputs, const jint * const inputPositions, const size_t inPosition);

#ifdef __cplusplus
}
#endif
#endif

