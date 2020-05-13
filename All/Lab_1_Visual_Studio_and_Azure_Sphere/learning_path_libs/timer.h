#pragma once

#include "eventloop_timer_utilities.h"
#include "stdbool.h"
#include <applibs/eventloop.h>

typedef struct {
	void (*handler)(EventLoopTimer* timer);
	struct timespec period;
	EventLoopTimer* eventLoopTimer;
	const char* name;
} Timer;

void lp_startTimerSet(Timer* timerSet[], size_t timerCount);
void lp_stopTimerSet(void);
bool lp_startTimer(Timer* timer);
void lp_stopTimer(Timer* timer);
EventLoop* lp_getTimerEventLoop(void);
void lp_stopTimerEventLoop(void);
bool lp_changeTimer(Timer* timer, const struct timespec* period);
bool lp_setOneShotTimer(Timer* timer, const struct timespec* delay);