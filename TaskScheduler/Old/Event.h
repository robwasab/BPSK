#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdlib.h>

typedef int (*Runnable)(void * arg);
class Event
{
	public:
		Event();
		Event(Runnable runnable, void * data, size_t len);
		int run();
	private:
		void * data;
		size_t len;
		Runnable runnable;
};
#endif
