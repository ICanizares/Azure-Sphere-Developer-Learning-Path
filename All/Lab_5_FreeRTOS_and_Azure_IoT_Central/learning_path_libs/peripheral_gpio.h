#pragma once

//#include "epoll_timerfd_utilities.h"
#include "parson.h"
#include <applibs/gpio.h>
#include <applibs/log.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "terminate.h"

typedef enum {
	LP_DIRECTION_UNKNOWN,
	LP_INPUT,
	LP_OUTPUT
} Direction;

struct _peripheralGpio {
	int fd;
	int pin;
	GPIO_Value initialState;
	bool invertPin;
	bool (*initialise)(struct _peripheralGpio* peripheralGpio);
	char* name;
	Direction direction;
	bool opened;
};

typedef struct _peripheralGpio LP_PeripheralGpio;


bool lp_openPeripheralGpio(LP_PeripheralGpio* peripheral);
void lp_openPeripheralGpioSet(LP_PeripheralGpio** peripheralSet, size_t peripheralSetCount);
void lp_closePeripheralGpioSet(void);
void lp_closePeripheralGpio(LP_PeripheralGpio* peripheral);
void lp_gpioOn(LP_PeripheralGpio* peripheral);
void lp_gpioOff(LP_PeripheralGpio* peripheral);
