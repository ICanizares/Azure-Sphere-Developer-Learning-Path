#pragma once

#include "hw/azure_sphere_learning_path.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "imu_temp_pressure.h"
#include "light_sensor.h"



int lp_readTelemetry(char* msgBuffer, size_t bufferLen);
bool lp_initializeDevKit(void);
bool lp_closeDevKit(void);
