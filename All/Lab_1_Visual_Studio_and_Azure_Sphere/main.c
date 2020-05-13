/*********************************************************************************************************************

Disclaimer: Please read!

The learning_path_libs C Functions provided in the learning_path_libs folder:

	1) are NOT supported Azure Sphere APIs.
	2) are built from the Azure Sphere SDK Samples provided at "https://github.com/Azure/azure-sphere-samples".
	3) are not intended as a substitute for understanding the Azure Sphere SDK Samples.
	4) aim to follow best practices as demonstrated by the Azure Sphere SDK Samples.
	5) are provided as is and as a convenience to aid the Azure Sphere Developer Learning experience. 

**********************************************************************************************************************/


/*********************************************************************************************************************

Support developer boards for the Azure Sphere Developer Learning Path:

	1) The AVNET Azure Sphere Starter Kit.
	2) Seeed Studio Azure Sphere MT3620 Development Kit (aka Reference Design Board or rdb).
	3) Seeed Studio Seeed Studio MT3620 Mini Dev Board.

How to select your developer board

	1) Open CMakeLists.txt.
	2) Uncomment the set command that matches your developer board.
	3) File -> Save (ctrl+s) to save the file and Generate the CMake Cache.

**********************************************************************************************************************/

#include "hw/azure_sphere_learning_path.h"

#include "learning_path_libs/azure_iot.h"
#include "learning_path_libs/globals.h"
#include "learning_path_libs/peripheral_gpio.h"
#include "learning_path_libs/terminate.h"
#include "learning_path_libs/timer.h"

#include "applibs_versions.h"
#include "exit_codes.h"
#include <applibs/gpio.h>
#include <applibs/log.h>
#include <applibs/powermanagement.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>


// Hardware specific

#ifdef OEM_AVNET
#include "learning_path_libs/AVNET/board.h"
#include "learning_path_libs/AVNET/imu_temp_pressure.h"
#include "learning_path_libs/AVNET/light_sensor.h"
#endif // OEM_AVNET

#ifdef OEM_SEEED_STUDIO
#include "learning_path_libs/SEEED_STUDIO/board.h"
#endif // SEEED_STUDIO


#define JSON_MESSAGE_BYTES 128  // Number of bytes to allocate for the JSON telemetry message for IoT Central

// Forward signatures
static void InitPeripheralsAndHandlers(void);
static void ClosePeripheralsAndHandlers(void);
static void Led1BlinkHandler(EventLoopTimer* eventLoopTimer);
static void Led2OffHandler(EventLoopTimer* eventLoopTimer);
static void MeasureSensorHandler(EventLoopTimer* eventLoopTimer);
static void ButtonPressCheckHandler(EventLoopTimer* eventLoopTimer);
static void NetworkConnectionStatusHandler(EventLoopTimer* eventLoopTimer);

static char msgBuffer[JSON_MESSAGE_BYTES] = { 0 };

static const struct timespec led2BlinkPeriod = { 0, 300 * 1000 * 1000 };

static int led1BlinkIntervalIndex = 0;
static const struct timespec led1BlinkIntervals[] = { {0, 125000000}, {0, 250000000}, {0, 500000000}, {0, 750000000}, {1, 0} };
static const int led1BlinkIntervalsCount = NELEMS(led1BlinkIntervals);

// GPIO Input Peripherals
static PeripheralGpio buttonA = { .pin = BUTTON_A, .direction = INPUT, .initialise = OpenPeripheralGpio, .name = "buttonA" };
static PeripheralGpio buttonB = { .pin = BUTTON_B, .direction = INPUT, .initialise = OpenPeripheralGpio, .name = "buttonB" };

// GPIO Output Peripherals
static PeripheralGpio led1 = {
	.pin = LED1, .direction = OUTPUT, .initialState = GPIO_Value_Low, .invertPin = true,
	.initialise = OpenPeripheralGpio, .name = "led1"
};
static PeripheralGpio led2 = {
	.pin = LED2, .direction = OUTPUT, .initialState = GPIO_Value_Low, .invertPin = true,
	.initialise = OpenPeripheralGpio, .name = "led2"
};
static PeripheralGpio networkConnectedLed = {
	.pin = NETWORK_CONNECTED_LED, .direction = OUTPUT, .initialState = GPIO_Value_Low, .invertPin = true,
	.initialise = OpenPeripheralGpio, .name = "networkConnectedLed"
};

// Timers
static Timer led1BlinkTimer = { .period = { 0, 125000000 }, .name = "led1BlinkTimer", .handler = Led1BlinkHandler };
static Timer led2BlinkOffOneShotTimer = { 
	.period = { 0, 0 }, 
	.name = "led2BlinkOffOneShotTimer", 
	.handler = Led2OffHandler 
};
static Timer buttonPressCheckTimer = { .period = { 0, 1000000 }, .name = "buttonPressCheckTimer", .handler = ButtonPressCheckHandler };
static Timer networkConnectionStatusTimer = { .period = { 5, 0 }, .name = "networkConnectionStatusTimer", .handler = NetworkConnectionStatusHandler };
static Timer measureSensorTimer = { 
	.period = { 10, 0 }, 
	.name = "measureSensorTimer", 
	.handler = MeasureSensorHandler 
};

// Initialize Sets
PeripheralGpio* peripheralSet[] = { &buttonA, &buttonB, &led1, &led2, &networkConnectedLed };
Timer* timerSet[] = { &led1BlinkTimer, &led2BlinkOffOneShotTimer, &buttonPressCheckTimer, &networkConnectionStatusTimer, &measureSensorTimer };


int main(int argc, char* argv[]) {
	RegisterTerminationHandler();

	InitPeripheralsAndHandlers();

	// Main loop
	while (!IsTerminationRequired()) {
		int result = EventLoop_Run(GetTimerEventLoop(), -1, true);
		// Continue if interrupted by signal, e.g. due to breakpoint being set.
		if (result == -1 && errno != EINTR) {
			Terminate();
		}
	}

	ClosePeripheralsAndHandlers();

	Log_Debug("Application exiting.\n");
	return GetTerminationExitCode();
}

/// <summary>
/// Check status of connection to Azure IoT
/// </summary>
static void NetworkConnectionStatusHandler(EventLoopTimer* eventLoopTimer) {
	if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
		Terminate();
		return;
	}

	if (IsNetworkReady()) {
		Gpio_On(&networkConnectedLed);
	}
	else {
		Gpio_Off(&networkConnectedLed);
	}
}

/// <summary>
/// Turn on LED2 and set a one shot timer to turn LED2 off
/// </summary>
static void Led2On(void) {
	Gpio_On(&led2);
	SetOneShotTimer(&led2BlinkOffOneShotTimer, &led2BlinkPeriod);
}

/// <summary>
/// One shot timer to turn LED2 off
/// </summary>
static void Led2OffHandler(EventLoopTimer* eventLoopTimer) {
	if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
		Terminate();
		return;
	}
	Gpio_Off(&led2);
}

/// <summary>
/// Read sensor and send to Azure IoT
/// </summary>
static void MeasureSensorHandler(EventLoopTimer* eventLoopTimer) {
	if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
		Terminate();
		return;
	}
	if (ReadTelemetry(msgBuffer, JSON_MESSAGE_BYTES) > 0) {
		Log_Debug("%s\n", msgBuffer);
		Led2On();
	}
}

/// <summary>
/// Read Button PeripheralGpio returns pressed state
/// </summary>
static bool IsButtonPressed(PeripheralGpio button, GPIO_Value_Type* oldState) {
	bool isButtonPressed = false;
	GPIO_Value_Type newState;

	if (GPIO_GetValue(button.fd, &newState) != 0) {
		Terminate();
	}
	else {
		// Button is pressed if it is low and different than last known state.
		isButtonPressed = (newState != *oldState) && (newState == GPIO_Value_Low);
		*oldState = newState;
	}
	return isButtonPressed;
}

/// <summary>
/// Handler to check for Button Presses
/// </summary>
static void ButtonPressCheckHandler(EventLoopTimer* eventLoopTimer) {
	static GPIO_Value_Type buttonAState;
	static GPIO_Value_Type buttonBState;

	if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
		Terminate();
		return;
	}

	if (IsButtonPressed(buttonA, &buttonAState)) {
		led1BlinkIntervalIndex = (led1BlinkIntervalIndex + 1) % led1BlinkIntervalsCount;
		ChangeTimer(&led1BlinkTimer, &led1BlinkIntervals[led1BlinkIntervalIndex]);

		if (ReadTelemetry(msgBuffer, JSON_MESSAGE_BYTES) > 0) {
			Log_Debug("%s\n", msgBuffer);
			Led2On();
		}
	}

	if (IsButtonPressed(buttonB, &buttonBState)) {
		if (ReadTelemetry(msgBuffer, JSON_MESSAGE_BYTES) > 0) {
			Log_Debug("%s\n", msgBuffer);
			Led2On();
		}
	}
}

/// <summary>
/// Blink Led1 Handler
/// </summary>
static void Led1BlinkHandler(EventLoopTimer* eventLoopTimer) {
	static bool blinkingLedState = false;

	if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
		Terminate();
		return;
	}

	blinkingLedState = !blinkingLedState;

	if (blinkingLedState) { Gpio_Off(&led1); }
	else { Gpio_On(&led1); }
}

/// <summary>
///  Initialize peripherals, device twins, direct methods, timers.
/// </summary>
/// <returns>0 on success, or -1 on failure</returns>
static void InitPeripheralsAndHandlers(void) {
	InitializeDevKit();

	OpenPeripheralGpioSet(peripheralSet, NELEMS(peripheralSet));
	StartTimerSet(timerSet, NELEMS(timerSet));
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void) {
	Log_Debug("Closing file descriptors\n");

	StopTimerSet();
	ClosePeripheralGpioSet();
	CloseDevKit();

	StopTimerEventLoop();
}