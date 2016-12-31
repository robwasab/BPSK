#include <stdio.h>
#include <string.h>
#include "Event.h"

Event::Event(Runnable runnable, void * data, size_t len)
{
	this->runnable = runnable;

	if (len > 0 && data != NULL) {
		this->data = (void *) malloc(len);
		this->len = len;
		memcpy(this->data, data, len);
	}
	else {
		this->data = NULL;
		this->len = 0;
	}
}

int Event::run()
{
	return runnable(this->data);
}
