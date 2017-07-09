#ifndef __LOG_H__
#define __LOG_H__

#include <typeinfo>
#include "../main.h"
#include "../Colors/Colors.h"

#define LOG(...) pthread_mutex_lock(&log_mutex); BLUE; printf("%s %s(): ", __FILE__, __func__); ENDC; printf(__VA_ARGS__); pthread_mutex_unlock(&log_mutex)
#define Log(...) LOG(__VA_ARGS__)
#define WARNING(...) BLINK; YELLOW; printf("%s %s(): ", __FILE__, __func__); ENDC; printf(__VA_ARGS__)
#define ERROR(...) BLINK; UNDERLINE; BOLD; RED; printf("%s %s(): ", __FILE__, __func__); ENDC; printf(__VA_ARGS__)

#endif
