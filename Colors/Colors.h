#ifndef __COLORS_H__
#define __COLORS_H__

#include <stdio.h>

#define BOLD printf("\e[1m")
#define DIM printf("\e[2m")
#define UNDERLINE printf("\e[4m")
#define BLINK printf("\e[5m")
#define INVERT printf("\e[8m")
#define ENDC printf("\e[0m"); fflush(stdout)
#define RED printf("\e[31m")
#define GREEN printf("\e[32m")
#define YELLOW printf("\e[33m")
#define BLUE printf("\e[34m")
#define MAGENTA printf("\e[35m")
#define CYAN printf("\e[36m")

#endif
