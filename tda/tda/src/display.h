#pragma once

#include "tasks.h"
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

struct task_info {
	double util;
	int32_t wcs_result;
	int32_t tda_result;
};

void ssd1306_print_task_info(struct task_params *params, struct task_info *info);
