#include <jni.h>
#include <unistd.h>

#include "datasignals_m2g_M2G.h"
#include "m2g.h"
#include "debug.h"

/*
This C code is designed to interface Java code with a native library for managing RS-232 (serial) communication, specifically for a module or device referred to as "M2G".
It uses the Java Native Interface (JNI) to allow Java applications to call these native methods. Here's a breakdown of its functionality:

1. Connect (Java_datasignals_m2g_M2G_connect): This function establishes a connection to a serial port specified by the port parameter. The async parameter determines
whether the connection should be asynchronous. It uses the m2g_connect function (presumably from the m2g.h library) to perform the actual connection and returns a descriptor
(an integer) that represents the connection.

2. Disconnect (Java_datasignals_m2g_M2G_disconnect): This function disconnects a previously established connection, identified by the descriptor desc. It calls m2g_disconnect
to perform the disconnection.

3. Write (Java_datasignals_m2g_M2G_write): This function writes data to the serial port. The data to be written is passed as a byte array (buf), with pos indicating the start
position in the array and length specifying how many bytes to write. It uses the write system call to send the data and returns the number of bytes actually written.

4. Read (Java_datasignals_m2g_M2G_read): This function reads data from the serial port into a byte array (buf). The parameters pos and length determine where in the array to store
the read data and how much data to attempt to read, respectively. It uses the read system call to receive the data and returns the number of bytes actually read.

5. Byte Count (Java_datasignals_m2g_M2G_byteCount): This function checks how many bytes are available to be read from the serial port without blocking. It calls m2g_bytes_available
to get this information and returns the count.

Throughout the code, there are conditional debug print statements (#ifdef DEBUG) that output diagnostic information to the console if the DEBUG macro is defined. This can be useful
for development and troubleshooting.

The JNI functions (GetStringUTFChars, ReleaseStringUTFChars, GetPrimitiveArrayCritical, ReleasePrimitiveArrayCritical) are used to handle conversion and memory management
between Java objects and native types.
*/

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
  
