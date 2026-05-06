#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define STACKSIZE       1024
#define BLINK_PERIOD_MS 250

struct task_params {
	int task_id;
	int priority;
	const struct gpio_dt_spec *led;
	int32_t execution_time_ms;
	int32_t period_ms;
	struct k_timer *timer;
};

/**
 * @brief Dummy task implementation that toggles LED
 */
void task_implementation(void *params, void *p1, void *p2);

/**
 * @brief Initializes periodic timer interrupt to print time in seconds
 */
void start_time_keeper(void);

/**
 * @brief Macro for initializing tasks.
 */
#define TASK_SETUP(n)                                                                              \
	static const struct gpio_dt_spec task##n##_led =                                           \
		GPIO_DT_SPEC_GET(DT_CHOSEN(task##n##_led), gpios);                                 \
	K_TIMER_DEFINE(task##n##_timer, NULL, NULL);                                               \
	static struct task_params task##n##_params = {                                             \
		.task_id = n,                                                                      \
		.priority = CONFIG_TASK##n##_PRIORITY,                                             \
		.led = &task##n##_led,                                                             \
		.period_ms = CONFIG_TASK##n##_PERIOD_MS,                                           \
		.execution_time_ms = CONFIG_TASK##n##_EXECUTION_TIME_MS,                           \
		.timer = &task##n##_timer,                                                         \
	};
