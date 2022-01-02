#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
 


int main(int argc, char** argv){

//  int ctty = 0, cstdin = 0;
  struct termios tio;
  struct termios stdio;
  struct termios old_stdio;
  int tty_fd;

  fd_set readfds; // The set of file descriptors probed for 'ready for
		  // reading', that is read() system call will not
		  // block/sleep.

  // select() timout value in seconds and microseconds.
  // struct timeval timeout;

  int retval;

  unsigned char charRead = 'D';


  tcgetattr(STDOUT_FILENO, &old_stdio);
 
  // Input serial port to communicate with as command line argument
  printf("Please start with %s /dev/ttyS1 (for example)\n", argv[0]);

  // Configure stdin and stout, as part make them non-blocking streams.
  memset(&stdio ,0, sizeof(stdio));
  stdio.c_iflag=0;
  stdio.c_oflag=0;
  stdio.c_cflag=0;
  stdio.c_lflag=0;
  stdio.c_cc[VMIN]=1;
  stdio.c_cc[VTIME]=0;
  tcsetattr(STDOUT_FILENO, TCSANOW, &stdio);
  tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio);
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // make the reads non-blocking
 
  // Configure the terminal/serial port.
  memset(&tio, 0, sizeof(tio));
  tio.c_iflag=0;
  tio.c_oflag=0;
  tio.c_cflag=CS8|CREAD|CLOCAL; // 8n1, see termios.h for more information
  tio.c_lflag=0;
  tio.c_cc[VMIN]=1;
  tio.c_cc[VTIME]=5;

  // system calls ie open/read/write() need error handling

  // Open the terminal/serial port stream: as read, write and non-blocking.
  tty_fd = open(argv[1], O_RDWR | O_NONBLOCK);      
  cfsetospeed(&tio, B9600); // as per arduino sketch
  cfsetispeed(&tio, B9600); // as per arduino sketch
  tcsetattr(tty_fd, TCSANOW, &tio);


  // Loop till stdin/console input is character 'q'.
  while (charRead != 'q'){

    // Clear and reset fds ready for next select() call.
    //
    // This is as per examples in select_tut(2)) and Stevens and also,
    // ~proj/atnf/cabb/src/callbacks.c.
    //
    FD_ZERO(&readfds); // clear/reset read file descriptors
    FD_SET(tty_fd, &readfds); // watch for chars on tty_fd/serial port.
    FD_SET(STDIN_FILENO, &readfds); // watch for chars on stdin=fd=0
    
    // Timeout from the select() call after timeout.sec/usec of non-activity.
    //
    // select() should really return way before so this is just a safety catch.
    // Set timeout = NULL for indefinate blocking/sleeping, and = 0 for polling.
    //
    //timeout.tv_sec = 60; // 1 minute timeout for select().
    //timeout.tv_usec = 0;

    // select() allows you to monitor more than one file descriptor at
    // a time.  It blocks(ie sleeps) till one or more file descriptors
    // become 'ready' for some io operation.  A file descriptor is
    // 'ready' when a corresponding type of system call(eg read()) for
    // readfds)will not block/sleep.
    if ( (retval = select((FD_SETSIZE+1), &readfds,
			  (fd_set *) NULL,(fd_set *) NULL, NULL))  == -1 ){
      perror("readSerialPort");
      fprintf(stderr, "readSerialPort: select returned %d\n", charRead);
      return 1;
    }

    // NEED ERROR HANDLING HERE: ****************************************

    // More than one fd can be ready at a time.  That is, character
    // present on the serial port and at stdin simultaneously.
    if (FD_ISSET(tty_fd, &readfds)){
      // Data is available on the serial port, read it from port and
      // print it out to stdout.
      read(tty_fd, &charRead, 1);
      write(STDOUT_FILENO, &charRead, 1);
    }
    
    if (FD_ISSET(STDIN_FILENO, &readfds)){
      // if data is available on the console/stdin, store input into charRead.
      read(STDIN_FILENO, &charRead, 1);
    }

  } // end: while (charRead != 'q')


  close(tty_fd);
  tcsetattr(STDOUT_FILENO, TCSANOW, &old_stdio);
	
  return 0; // success

}
