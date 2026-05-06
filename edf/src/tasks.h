#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define STACKSIZE       1024
#define BLINK_PERIOD_MS 250

typedef void task_func(void *p0, void *p1, void *p2);

enum task_type {
	PERIODIC_TASK,
	DEFERRABLE_SERVER,
};

struct task_params {
	// task specification
	int task_id;
	enum task_type type;
	int priority;
	int32_t execution_time_ms;
	int32_t period_ms;
	// optional LED (only specified for periodic tasks)
	const struct gpio_dt_spec *led;
	// helper field
	task_func *func;
	struct k_timer *timer;
	struct k_thread *thread;
};

/**
 * @brief System density test for periodic tasks
 *
 * TODO: The system density test should account for the deferrable server (DS). That is, we assume
 * the DS is always accepted and the acceptance is solely performed for periodic tasks. Moreover the
 * acceptance test is called iteratively (see main.c), meaning that Task i must only account for the
 * previously accepted tasks.
 *
 * @params params task specification
 */
bool acceptance_test(struct task_params *params);

/**
 * @brief Dummy task implementation that toggles LED
 */
void periodic_task_implementation(void *params, void *p1, void *p2);

/**
 * @brief Deferrable server implementation
 */
void deferrable_server_implementation(void *params, void *p1, void *p2);

/**
 * @brief Initializes periodic timer interrupt to print time in seconds
 */
void start_time_keeper(void);
int init_button_gpio(void);
void ssd1306_clear_screen(void);

/**
 * @brief Macro for initializing tasks.
 */
#define PERIODIC_TASK_SETUP(n)                                                                     \
	struct k_thread task##n##_thread;                                                          \
	static const struct gpio_dt_spec task##n##_led =                                           \
		GPIO_DT_SPEC_GET_OR(DT_CHOSEN(task##n##_led), gpios, {0});                         \
	K_TIMER_DEFINE(task##n##_timer, NULL, NULL);                                               \
	static const STRUCT_SECTION_ITERABLE(task_params, task##n##_params) = {                    \
		.task_id = n,                                                                      \
		.type = PERIODIC_TASK,                                                             \
		.priority = CONFIG_MAIN_THREAD_PRIORITY,                                           \
		.execution_time_ms = CONFIG_TASK##n##_EXECUTION_TIME_MS,                           \
		.period_ms = CONFIG_TASK##n##_PERIOD_MS,                                           \
		.led = &task##n##_led,                                                             \
		.func = periodic_task_implementation,                                              \
		.timer = &task##n##_timer,                                                         \
		.thread = &task##n##_thread,                                                       \
	};

#define DEFERRABLE_SERVER_SETUP(n)                                                                 \
	struct k_thread task##n##_thread;                                                          \
	static const struct gpio_dt_spec task##n##_led =                                           \
		GPIO_DT_SPEC_GET_OR(DT_CHOSEN(task##n##_led), gpios, {0});                         \
	K_TIMER_DEFINE(task##n##_timer, NULL, NULL);                                               \
	static const STRUCT_SECTION_ITERABLE(task_params, task##n##_params) = {                    \
		.task_id = n,                                                                      \
		.type = DEFERRABLE_SERVER,                                                         \
		.priority = CONFIG_MAIN_THREAD_PRIORITY,                                           \
		.execution_time_ms = CONFIG_TASK##n##_EXECUTION_TIME_MS,                           \
		.period_ms = CONFIG_TASK##n##_PERIOD_MS,                                           \
		.led = &task##n##_led,                                                             \
		.func = deferrable_server_implementation,                                          \
		.timer = &task##n##_timer,                                                         \
		.thread = &task##n##_thread,                                                       \
	};
