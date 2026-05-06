#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define STACKSIZE       1024
#define BLINK_PERIOD_MS 250

struct task_params {
	int task_id;
	const struct gpio_dt_spec *led;
	int32_t execution_time_ms;
	int32_t period_ms;
	struct k_timer *timer;
};

/**
 * @brief TODO: Implementation of the chatterbox task, see tasks.c
 */
void chatterbox_task(struct task_params *params);

/**
 * @brief Macro for initializing Task 1 to Task 3.
 *
 * This assumes that the device tree overlay and Kconfig are already
 * set up correctly. In particular, *INITIALIZE_TASK(1);* requires
 * boards/esp_wrover_kit_procpu.overlay:
 *  - DT_CHOSEN(task1_led)
 * Kconfig:
 *  - CONFIG_TASK1_PERIOD_MS
 *  - CONFIG_TASK1_EXECUTION_TIME_MS
 *  - CONFIG_TASK1_PRIORITY
 *  - CONFIG_TASK1_RELEASE_MS
 */
#define INITIALIZE_TASK(n)                                                                         \
	static const struct gpio_dt_spec task##n##_led =                                           \
		GPIO_DT_SPEC_GET(DT_CHOSEN(task##n##_led), gpios);                                 \
	K_TIMER_DEFINE(task##n##_timer, NULL, NULL);                                               \
	static const struct task_params task##n##_params = {                                       \
		.task_id = n,                                                                      \
		.led = &task##n##_led,                                                             \
		.period_ms = CONFIG_TASK##n##_PERIOD_MS,                                           \
		.execution_time_ms = CONFIG_TASK##n##_EXECUTION_TIME_MS,                           \
		.timer = &task##n##_timer,                                                         \
	};                                                                                         \
	K_THREAD_DEFINE(task##n##_id, STACKSIZE, chatterbox_task, &task##n##_params, NULL, NULL,   \
			CONFIG_TASK##n##_PRIORITY, 0, CONFIG_TASK##n##_RELEASE_MS);
