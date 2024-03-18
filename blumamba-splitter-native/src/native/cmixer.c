#include <stdlib.h>
#include <pthread.h>

#include "debug.h"
#include "datasignals_transform_matrix_jni_JNativeMatrixMixer.h"
#include "bits.h"
#include "mult.h"

struct mix_info {
   jbyte * output;
   const jlong *matrix;
   jint dimension;
   jbyte * const * inputs;
   const jint * inputPositions;
};

struct thread_mix_info {
   pthread_t thread_id;
   const struct mix_info *mInfo;
   size_t outPosition;
   size_t rowStart;
   size_t rowEnd;
};

void rangeMix(jbyte * const output, size_t outPosition, const jlong * const matrix, const jint dimension,
                     jbyte * const * const inputs, const jint * const inputPositions, const size_t rowStart, const size_t rowEnd) {
    for(size_t row = rowStart; row < rowEnd; row++) {
        #ifdef TRACE
        printf("row: %ld\n", row);
        #endif
        for(size_t col = 0; col < dimension; col ++) {
            #ifdef TRACE
            printf("row: %ld, inputPositions[%ld]: %ld\n", row, col, inputPositions[col]);
            #endif
            putLong(output, outPosition, mmult(dimension, matrix, col, inputs, inputPositions, row << LOG_LONG_BYTES));
            outPosition += LONG_BYTES;
        }
    }
}

void *threadRangeMix(void *info) {
    const struct thread_mix_info * const inf = info;
    #ifdef DEBUG
    printf("\tthreadRangeMix(%p){\n", inf);
    #endif

    const struct mix_info * const mInfo = inf -> mInfo;

    rangeMix(mInfo -> output, inf -> outPosition,
        mInfo -> matrix, mInfo -> dimension,
        mInfo -> inputs, mInfo -> inputPositions,
        inf -> rowStart, inf -> rowEnd);

    #ifdef DEBUG
    printf("\t}threadRangeMix(%p)\n", inf);
    #endif
}

void cmix(jbyte * const output, size_t outPosition, const jlong * const matrix, const jint dimension,
                 jbyte * const * const inputs, jint * const inputPositions, const size_t inputLength, jint nThreads) {

    #ifdef DEBUG
    printf("inputLength: %ld\n", inputLength);
    #endif

    const size_t nRows = inputLength >> LOG_LONG_BYTES;
    if(nRows <= nThreads) nThreads = nRows;
    size_t rowStep = nRows / nThreads;
    if(rowStep * nThreads < nRows) rowStep++;

    #ifdef DEBUG
    printf("Number of rows: %lu, number of threads: %d\n", nRows, nThreads);
    #endif

    pthread_t threads[nThreads];

    struct mix_info * const mInfo = (struct mix_info * const)malloc(sizeof(struct mix_info));
    mInfo -> output = output;
    mInfo -> matrix = matrix;
    mInfo -> dimension = dimension;
    mInfo -> inputs = inputs;
    mInfo -> inputPositions = inputPositions;

    struct thread_mix_info * const info = (struct thread_mix_info * const)malloc(nThreads * sizeof(struct thread_mix_info));
    struct thread_mix_info *inf;

    size_t rowStart, rowEnd = 0;
    jint i;
    for(i = 0; i < nThreads; i++) {
        rowStart = rowEnd;
        if(i == nThreads - 1)
            rowEnd = nRows;
        else
            rowEnd += rowStep;

        inf = &info[i];
        inf -> mInfo = mInfo;
        inf -> outPosition = outPosition + (dimension * rowStart << LOG_LONG_BYTES);
        inf -> rowStart = rowStart;
        inf -> rowEnd = rowEnd;

        #ifdef DEBUG
        int r =
        #endif

        pthread_create(&threads[i], NULL, threadRangeMix, (void *)inf);

        #ifdef DEBUG
        if(r != 0) printf("Error thread create %d: %d", i, r);
        #endif
    }

    for(i = 0; i < nThreads; i++) {
        #ifdef DEBUG
        int r =
        #endif

        pthread_join(threads[i], NULL);

        #ifdef DEBUG
        if(r != 0) printf("Error thread join %d: %d", i, r);
        #endif
    }
    free(info);
    free(mInfo);
}

/*
 * Class:      datasignals_transform_matrix_jni_JNativeMatrixMixer
 * Method:     concurrentMix
 * Signature:  ([BI[JI[[B[III)V
 */
JNIEXPORT void JNICALL Java_datasignals_transform_matrix_jni_JNativeMatrixMixer_concurrentMix
  (JNIEnv * env, jclass instance, jbyteArray outArray, jint outPosition, jlongArray matrixArray, jint dimension,
    jobjectArray inArrays, jintArray inPositions, jint inLength, jint nThreads) {

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

    cmix(output, outPosition, matrix, dimension, inputs, inputPositions, inLength, nThreads);

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
