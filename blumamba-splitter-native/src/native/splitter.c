#include <stdlib.h>

#include "debug.h"
#include "datasignals_transform_matrix_jni_JNativeMatrixSplitter.h"
#include "bits.h"
#include "mult.h"

void split(jbyte ** const output, jint * const outPosition, const jlong * const matrix, const jint dimension,
        const jbyte * const input, const size_t inputPosition, const size_t inputLength) {
    const size_t nRows = (inputLength / dimension) >> LOG_LONG_BYTES;

    #ifdef DEBUG
    printf("Number of rows: %lu\n", nRows);
    #endif

    for(size_t row = 0; row < nRows; row++) {
        size_t offset = row << LOG_LONG_BYTES;
        for(size_t col = 0; col < dimension; col++) {

            #ifdef TRACE
            printf("\trow: %lu, col: %lu, abs offset: %lu\n", row, col, outPosition[col] + offset);
            #endif

            putLong(output[col], outPosition[col] + offset,
                mult(dimension, matrix, col, input, inputPosition + offset * dimension));

        }
    }
}

/*
 * Class:      datasignals_transform_matrix_jni_JNativeMatrixSplitter
 * Method:     split
 * Signature:  ([[B[I[JI[BII)V
 */
JNIEXPORT void JNICALL Java_datasignals_transform_matrix_jni_JNativeMatrixSplitter_split
  (JNIEnv *env, jclass instance, jobjectArray outArrays, jintArray outPositions,
  jlongArray matrixArray, jint dimension, jbyteArray inputArray, jint inPosition, jint inLength) {

    #ifdef DEBUG
    printf("Dimension: %d, inPosition: %d, inLength: %d\n", dimension, inPosition, inLength);
    #endif

    jlong * const matrix = (jlong * const)(*env) -> GetPrimitiveArrayCritical(env, matrixArray, 0);

    #ifdef DEBUG
    printf("Got matrix pointer: %p\n", matrix);
        #ifdef TRACE
        for(size_t i = 0; i < dimension; i++) {
            for(size_t j = 0; j < dimension; j++)
                printf("matrix[%lu, %lu]=%ld ", i, j, matrix[i * dimension + j]);
            printf("\n");
        }
        #endif
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    #endif

    jbyte * const input = (jbyte * const)(*env) -> GetPrimitiveArrayCritical(env, inputArray, 0);

    #ifdef DEBUG
    printf("Got input array %p\n", input);
        #ifdef TRACE
        for(size_t i = 0; i < inPosition; i++)
            printf("input[%lu] = %x\n", i, input[i]);
        #endif
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    #endif

    jobject * const outputArrays = (jobject * const)malloc(dimension * sizeof(jobject));
    jbyte ** const outputs = (jbyte ** const)malloc(dimension * sizeof(jbyte *));
    jint * const outputPositions = (jint * const)(*env) -> GetPrimitiveArrayCritical(env, outPositions, 0);

    #ifdef DEBUG
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    #endif

    for(size_t i = 0; i < dimension; i++) {
        outputArrays[i] = (*env) -> GetObjectArrayElement(env, outArrays, i);
        outputs[i] = (*env) -> GetPrimitiveArrayCritical(env, outputArrays[i], 0);

        #ifdef DEBUG
        if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
        #endif
    }

    #ifdef DEBUG
    printf("Splitting...\n");
    #endif

    split(outputs, outputPositions, matrix, dimension, input, inPosition, inLength);

    #ifdef DEBUG
    printf("Splitting is finished.\n");
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    #endif

    for(size_t i = 0; i < dimension; i++) {
        (*env) -> ReleasePrimitiveArrayCritical(env, outputArrays[i], outputs[i], 0);

        #ifdef DEBUG
        if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
        #endif
    }

    #ifdef DEBUG
    printf("Output arrays are released\n");
    #endif

    (*env) -> ReleasePrimitiveArrayCritical(env, outPositions, outputPositions, JNI_ABORT);
    free(outputs);
    free(outputArrays);

    #ifdef DEBUG
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    printf("Output positions are released, output arrays are freed.\n");
    #endif

    (*env) -> ReleasePrimitiveArrayCritical(env, inputArray, input, JNI_ABORT);

    #ifdef DEBUG
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    printf("Input array is released\n");
    #endif

    (*env) -> ReleasePrimitiveArrayCritical(env, matrixArray, matrix, JNI_ABORT);

    #ifdef DEBUG
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    printf("Matrix array is released\n");
    #endif

    #ifdef DEBUG
    jbyte* va = (jbyte*) malloc(8 * sizeof(jbyte));
    va[0] = 0x08;
    va[1] = 0x19;
    va[2] = 0x2a;
    va[3] = 0x3b;
    va[4] = 0x4c;
    va[5] = 0x5d;
    va[6] = 0x6e;
    va[7] = 0x7f;
    jlong v = getLong(va, 0);
    printf("getLong: 0x%lx\n", v);
    free(va);

    jbyte* vb = (jbyte*) malloc(8 * sizeof(jbyte));
    putLong(vb, 0, v);
    for(size_t i = 0; i < 8; i++) {
        printf("putLong[%lu]: %x\n", i, vb[i]);
    }
    free(vb);
    #endif

  }
