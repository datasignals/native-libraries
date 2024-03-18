#include <stdlib.h>
#include <pthread.h>

#include "debug.h"
#include "datasignals_transform_matrix_jni_JNativeMatrixSplitter.h"
#include "bits.h"
#include "mult.h"

struct split_info {
   jbyte * const *output;
   const jint *outPosition;
   const jlong *matrix;
   jint dimension;
   const jbyte *input;
   size_t inputPosition;
};

struct thread_info {
   pthread_t thread_id;
   const struct split_info *sInfo;
   size_t rowStart;
   size_t rowEnd;
};

void rangeSplit(jbyte * const * const output, const jint * const outPosition, const jlong * const matrix, const jint dimension,
                const jbyte * const input, const size_t inputPosition,
                const size_t rowStart, const size_t rowEnd) {
    size_t offset, col;
    for(size_t row = rowStart; row < rowEnd; row++) {
        offset = row << LOG_LONG_BYTES;
        for(col = 0; col < dimension; col++) {

            #ifdef TRACE
            printf("\trow: %lu, col: %lu, abs offset: %lu\n", row, col, outPosition[col] + offset);
            #endif

            putLong(output[col], outPosition[col] + offset,
                mult(dimension, matrix, col, input, inputPosition + offset * dimension));

        }
    }
}

void *threadRangeSplit(void *info) {
    const struct thread_info * const inf = info;
    #ifdef DEBUG
    printf("\tthreadRangeSplit(%p){\n", inf);
    #endif

    const struct split_info * const sInfo = inf -> sInfo;

    rangeSplit(sInfo -> output, sInfo -> outPosition,
        sInfo -> matrix, sInfo -> dimension,
        sInfo -> input, sInfo -> inputPosition,
        inf -> rowStart, inf -> rowEnd);

    #ifdef DEBUG
    printf("\t}threadRangeSplit(%p)\n", inf);
    #endif
}

void csplit(jbyte * const * const output, const jint * const outPosition, const jlong * const matrix, const jint dimension,
        const jbyte * const input, const size_t inputPosition, const size_t inputLength, jint nThreads) {
    const size_t nRows = (inputLength / dimension) >> LOG_LONG_BYTES;
    if(nRows <= nThreads) nThreads = nRows;
    size_t rowStep = nRows / nThreads;
    if(rowStep * nThreads < nRows) rowStep++;

    #ifdef DEBUG
    printf("Number of rows: %lu, number of threads: %d\n", nRows, nThreads);
    #endif

    pthread_t threads[nThreads];

    struct split_info * const sInfo = (struct split_info * const)malloc(sizeof(struct split_info));
    sInfo -> output = output;
    sInfo -> outPosition = outPosition;
    sInfo -> matrix = matrix;
    sInfo -> dimension = dimension;
    sInfo -> input = input;
    sInfo -> inputPosition = inputPosition;

    struct thread_info * const info = (struct thread_info * const)malloc(nThreads * sizeof(struct thread_info));
    size_t rowStart, rowEnd = 0;
    struct thread_info *inf;
    jint i;

    for(i = 0; i < nThreads; i++) {
        rowStart = rowEnd;
        if(i == nThreads - 1)
            rowEnd = nRows;
        else
            rowEnd += rowStep;

        inf = &info[i];
        inf -> sInfo = sInfo;
        inf -> rowStart = rowStart;
        inf -> rowEnd = rowEnd;

        #ifdef DEBUG
        int r =
        #endif

        pthread_create(&threads[i], NULL, threadRangeSplit, (void *)inf);

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
    free(sInfo);
 }

/*
 * Class:      datasignals_transform_matrix_jni_JNativeMatrixSplitter
 * Method:     concurrentSplit
 * Signature:  ([[B[I[JI[BIII)V
 */
JNIEXPORT void JNICALL Java_datasignals_transform_matrix_jni_JNativeMatrixSplitter_concurrentSplit
  (JNIEnv *env, jclass instance, jobjectArray outArrays, jintArray outPositions,
  jlongArray matrixArray, jint dimension, jbyteArray inputArray, jint inPosition, jint inLength, jint nThreads) {

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

    csplit(outputs, outputPositions, matrix, dimension, input, inPosition, inLength, nThreads);

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
