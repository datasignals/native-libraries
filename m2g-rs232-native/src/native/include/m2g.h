#include <jni.h>
#include <bits/types.h>

#ifndef _bits
#define _bits
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char bool;

int m2g_connect(char const * const port, const bool async);
int m2g_disconnect(int fd);
int m2g_bytes_available(int fd);

#ifdef __cplusplus
}
#endif
#endif

