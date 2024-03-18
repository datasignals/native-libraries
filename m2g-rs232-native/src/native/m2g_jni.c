#include <jni.h>
#include <unistd.h>

#include "datasignals_m2g_M2G.h"
#include "m2g.h"
#include "debug.h"

/*
 * Class:      datasignals_m2g_M2G
 * Method:     connect
 * Signature:  (Ljava/lang/String;Z)I
 */
JNIEXPORT jint JNICALL Java_datasignals_m2g_M2G_connect
  (JNIEnv * env, jclass instance, jstring port, jboolean async) {

    const char * port_name = (*env) -> GetStringUTFChars(env, port, 0);

    #ifdef DEBUG
    printf("Port name: '%s'\n", port_name);
    #endif

    jint desc = m2g_connect(port_name, async);

    (*env) -> ReleaseStringUTFChars(env, port, port_name);

    return desc;
  }

/*
 * Class:      datasignals_m2g_M2G
 * Method:     disconnect
 * Signature:  (I)I
 */
JNIEXPORT jint JNICALL Java_datasignals_m2g_M2G_disconnect
  (JNIEnv *env, jclass instance, jint desc) {
    return (jint)m2g_disconnect(desc);
  }

/*
 * Class:      datasignals_m2g_M2G
 * Method:     write
 * Signature:  (I[B)I
 */
JNIEXPORT jint JNICALL Java_datasignals_m2g_M2G_write
  (JNIEnv *env, jclass instance, jint desc, jbyteArray buf, jint pos, jint length) {

    jbyte * const array = (jbyte * const)(*env) -> GetPrimitiveArrayCritical(env, buf, 0);

    #ifdef DEBUG
    printf("Writing byte array:");
    for(int i = 0; i < length; i++) {
        printf(" %x", (array + pos)[i]);
    }
    printf("\n");
    #endif

    jint bytes = write(desc, array + pos, length);

    #ifdef DEBUG
    printf("Bytes written: %d\n", bytes);
    #endif

    (*env) -> ReleasePrimitiveArrayCritical(env, buf, array, JNI_ABORT);

    return bytes;
  }

  /*
   * Class:      datasignals_m2g_M2G
   * Method:     read
   * Signature:  (I[BII)I
   */
  JNIEXPORT jint JNICALL Java_datasignals_m2g_M2G_read
    (JNIEnv * env, jclass instance, jint desc, jbyteArray buf, jint pos, jint length) {

    jbyte * const array = (jbyte * const)(*env) -> GetPrimitiveArrayCritical(env, buf, 0);

    jint bytes = read(desc, array + pos, length);

    #ifdef DEBUG
    printf("Received bytes: %d\n", bytes);
    printf("Received byte array:");
    for(int i = 0; i < length; i++) {
        printf(" %x", array[pos + i]);
    }
    printf("\n");
    #endif

    (*env) -> ReleasePrimitiveArrayCritical(env, buf, array, 0);

    return bytes;

  }

  /*
   * Class:      datasignals_m2g_M2G
   * Method:     byteCount
   * Signature:  (I)I
   */
  JNIEXPORT jint JNICALL Java_datasignals_m2g_M2G_byteCount
    (JNIEnv * env, jclass instance, jint desc) {
    return m2g_bytes_available(desc);
  }
  
