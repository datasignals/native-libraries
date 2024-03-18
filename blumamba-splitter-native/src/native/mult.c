#include "mult.h"
#include "bits.h"
#include "debug.h"


inline jlong mult(const size_t dimension, const jlong *matrix, const size_t column, const jbyte *input, const size_t inPosition) {
    jlong sum = 0l;
    for(size_t j = 0; j < dimension; j++) {

        #ifdef TRACE
        printf("sum = %ld, matrix[%lu, %lu] = %ld, val at %ld = %ld\n", sum, j, column, matrix[column * dimension + j], inPosition + (j << LOG_LONG_BYTES), getLong(input, inPosition + (j << LOG_LONG_BYTES)));
        #endif

        sum += matrix[column * dimension + j] * getLong(input, inPosition + (j << LOG_LONG_BYTES));
    }

    #ifdef TRACE
    printf("sum = %ld\n", sum);
    #endif
    return sum;
}

inline jlong mmult(const size_t dimension, const jlong *matrix, const size_t column, jbyte * const * const inputs, const jint * const inputPositions, const size_t inPosition) {
    jlong sum = 0l;
    for(size_t j = 0; j < dimension; j++) {

        #ifdef TRACE
        printf("sum = %ld, matrix[%lu, %lu] = %ld, val at %ld = %ld\n", sum, j, column, matrix[column * dimension + j], inputPositions[j] + inPosition, getLong(inputs[j], inputPositions[j] + inPosition));
        #endif

        sum += matrix[column * dimension + j] * getLong(inputs[j], inputPositions[j] + inPosition);
    }

    #ifdef TRACE
    printf("sum = %ld\n", sum);
    #endif
    return sum;
}