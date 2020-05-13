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
	DIRECTION_UNKNOWN,
	INPUT,
	OUTPUT
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

typedef struct _peripheralGpio PeripheralGpio;


bool OpenPeripheralGpio(PeripheralGpio* peripheral);
void OpenPeripheralGpioSet(PeripheralGpio** peripheralSet, size_t peripheralSetCount);
void ClosePeripheralGpioSet(void);
void ClosePeripheralGpio(PeripheralGpio* peripheral);
void Gpio_On(PeripheralGpio* peripheral);
void Gpio_Off(PeripheralGpio* peripheral);
