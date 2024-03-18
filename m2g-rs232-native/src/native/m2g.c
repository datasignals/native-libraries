#include <stdlib.h>
#include <error.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "m2g.h"
#include "debug.h"

int set_connection_param(int fd, const bool async) {

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    options.c_cflag |= (CLOCAL | CREAD);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS; //Hardware flow off

    //options.c_iflag &= ~IGNCR;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    if(async)
        options.c_cc[VMIN] = 0;
    else
        options.c_cc[VMIN] = 1; // read blocks until at least one character is received
    options.c_cc[VTIME] = 0; // no timeout for read: it blocks indefinitely

    const int r = tcsetattr(fd, TCSANOW, &options);
    #ifdef DEBUG
    if(r != 0) {
        perror("Error setting connection attributes");
    }
    #endif
    return r;
}

int m2g_connect(char const * const port, const bool async) {
  #ifdef DEBUG
  printf("Connecting %s...\n", port);
  #endif

  int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);

  if (fd == -1) {
    //Error reporting
    #ifdef DEBUG
    perror("Error opening port");
    #endif
  } else {
    #ifdef DEBUG
    printf("Descriptor: %d\n", fd);
    printf("Descriptor flags: %x\n", fcntl(fd, F_GETFL));
    #endif

    fcntl(fd, F_SETFL, 0);

    #ifdef DEBUG
    printf("Descriptor flags: %x\n", fcntl(fd, F_GETFL));
    printf("O_NONBLOCK=%x\n", O_NONBLOCK);
    printf("O_NDELAY=%x\n", O_NDELAY);
    printf("O_RWDR=%x\n", O_RDWR);
    printf("O_NOCTTY=%x\n", O_NOCTTY);
    #endif

    if(set_connection_param(fd, async) != 0) {
        m2g_disconnect(fd);
        return -1;
    };
  }
  return fd;
}

int m2g_disconnect(int fd) {
  int r = close(fd);
  #ifdef DEBUG
  if(r != 0) {
      perror("Error closing descriptor");
  }
  #endif
  return r;
}

int m2g_bytes_available(int fd) {
  int bytes;
  int r = ioctl(fd, FIONREAD, &bytes);
  if(r == -1) {
    #ifdef DEBUG
    perror("Error in counting bytes available");
    #endif
    return -1;
  }
  #ifdef DEBUG
  printf("Available bytes: %d\n", bytes);
  #endif
  return bytes;
}
