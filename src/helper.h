#ifndef __HELPER_H
#define __HELPER_H

#include "common.h"

#ifdef DEBUG
  #define DOUT(...) fprintf(stderr, __VA_ARGS__)
#else
  #define DOUT(...) if (0) {}
#endif

#endif
