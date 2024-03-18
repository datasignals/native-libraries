#include <stdlib.h>

#include "debug.h"
#include "datasignals_transform_matrix_jni_JNativeMatrixMixer.h"
#include "bits.h"
#include "mult.h"

void mix(jbyte * const output, size_t outPosition, const jlong * const matrix, const jint dimension,
                jbyte * const * const inputs, jint * const inputPositions, const size_t inputLength) {

    #ifdef DEBUG
    printf("inputLength: %ld\n", inputLength);
    #endif

   for(size_t pos = 0; pos < inputLength; pos += LONG_BYTES) {
        #ifdef TRACE
        printf("pos: %ld\n", pos);
        #endif
        for(size_t col = 0; col < dimension; col ++) {
            #ifdef TRACE
            printf("pos: %ld, inputPositions[%ld]: %ld\n", pos, col, inputPositions[col]);
            #endif
            putLong(output, outPosition, mmult(dimension, matrix, col, inputs, inputPositions, pos));
            outPosition += LONG_BYTES;
        }
   }
}

/*
 * Class:      datasignals_transform_matrix_jni_JNativeMatrixMixer
 * Method:     mix
 * Signature:  ([BI[JI[[B[I[I)V
 */
JNIEXPORT void JNICALL Java_datasignals_transform_matrix_jni_JNativeMatrixMixer_mix
  (JNIEnv * env, jclass instance, jbyteArray outArray, jint outPosition, jlongArray matrixArray, jint dimension,
    jobjectArray inArrays, jintArray inPositions, jint inLength) {

    #ifdef DEBUG
    printf("Dimension: %d, outPosition: %d, inLength: %d\n", dimension, outPosition, inLength);
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

    jobject * const inputArrays = (jobject * const)malloc(dimension * sizeof(jobject));
    jbyte ** const inputs = (jbyte ** const)malloc(dimension * sizeof(jbyte *));
    jint * const inputPositions = (jint * const)(*env) -> GetPrimitiveArrayCritical(env, inPositions, 0);

    #ifdef DEBUG
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    #endif

    for(size_t i = 0; i < dimension; i++) {
        inputArrays[i] = (*env) -> GetObjectArrayElement(env, inArrays, i);
        inputs[i] = (*env) -> GetPrimitiveArrayCritical(env, inputArrays[i], 0);

        #ifdef DEBUG
            #ifdef TRACE
            for(size_t j = 0; j < inLength; j++) printf("inputs[%ld][%ld] = %d\n", i, j, inputs[i][j]);
            #endif
        if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
        #endif
    }

    jbyte * const output = (jbyte * const)(*env) -> GetPrimitiveArrayCritical(env, outArray, 0);

    #ifdef DEBUG
    printf("Got output array %p\n", output);
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    #endif

    #ifdef DEBUG
    printf("Mixing...\n");
    #endif

    mix(output, outPosition, matrix, dimension, inputs, inputPositions, inLength);

    #ifdef DEBUG
    printf("Mixing is finished.\n");
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    #endif

    (*env) -> ReleasePrimitiveArrayCritical(env, outArray, output, 0);

    #ifdef DEBUG
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    printf("Output array is released\n");
    #endif

    for(size_t i = 0; i < dimension; i++) {
        (*env) -> ReleasePrimitiveArrayCritical(env, inputArrays[i], inputs[i], JNI_ABORT);

        #ifdef DEBUG
        if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
        #endif
    }

    #ifdef DEBUG
    printf("Input arrays are released\n");
    #endif

    (*env) -> ReleasePrimitiveArrayCritical(env, inPositions, inputPositions, JNI_ABORT);
    free(inputs);
    free(inputArrays);

    #ifdef DEBUG
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    printf("Input positions are released, input arrays are freed.\n");
    #endif

    (*env) -> ReleasePrimitiveArrayCritical(env, matrixArray, matrix, JNI_ABORT);

    #ifdef DEBUG
    if((*env) -> ExceptionOccurred(env)) (*env) -> ExceptionDescribe(env);
    printf("Matrix array is released\n");
    #endif
  }
