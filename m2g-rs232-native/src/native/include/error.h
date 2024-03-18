#ifndef _ERROR_H
#define _ERROR_H

//#include <errno.h>  // Include errno for error code definitions

// Function prototypes
void perror(const char *s); // Print an error message to stderr based on the value of errno
//const char *strerror(int errnum); // Return a string describing the given error code

// Error handling macros
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#endif /* _ERROR_H */
