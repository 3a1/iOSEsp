#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <arpa/inet.h>
#include <sys/sysctl.h>

//
//  Errors output toggle.
//  May be useful to enable if something is not working.
//
#define ERROR_OUTPUT 1

#if ERROR_OUTPUT
  #define ErrorPrint(format, ...) printf(format "\n", ##__VA_ARGS__)
#else
  #define ErrorPrint(format, ...) ((void)0)
#endif

#include "offsets.h"
#include "socket.h"
#include "memory.h"
#include "utils.h"
#include "game.h"
#include "main.h"

//       |\      _,,,---,,_
// Zzz   /,`.-'`'    -.  ;-;;,_
//      |,4-  ) )-,_. ,\ (  `'-'
//     '---''(_/--'  `-\_)