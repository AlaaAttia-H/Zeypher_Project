#include <assert.h>
#include <stdio.h>

#include "display.h"
#include "tasks.h"

/**
 * @brief Let LED blink for (roughly) delay_ms.
 *
 * @param params    Task parameters
 * @param info	    Task info of acceptance tests
 */
void useless_load(struct task_params *params, struct task_info *info)
{
	assert(params->execution_time_ms % BLINK_PERIOD_MS == 0 && BLINK_PERIOD_MS % 2 == 0);

	// Let LED blink for (roughly) delay_ms
	for (int32_t d = 0; d < params->execution_time_ms; d += BLINK_PERIOD_MS) {
		printf(" Task %d\n", params->task_id);
		gpio_pin_set_dt(params->led, 1);
		k_busy_wait(BLINK_PERIOD_MS * 1000 / 2);
		gpio_pin_set_dt(params->led, 0);
		k_busy_wait(BLINK_PERIOD_MS * 1000 / 2);
	}
}

/**
 * @brief Dummy task implementation that toggles LED
 */
void task_implementation(void *p0, void *p1, void *p2)
{
	struct task_params *params = (struct task_params *)p0;
	struct task_info *info = (struct task_info *)p1;

	if (!gpio_is_ready_dt(params->led)) {
		return;
	}
	int ret = gpio_pin_configure_dt(params->led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	k_timer_start(params->timer, K_NO_WAIT, K_MSEC(params->period_ms));
	while (true) {
		k_timer_status_sync(params->timer);
		useless_load(params, info);
	}
}

/**
 * @brief Prints current time in seconds
 */
void print_time()
{
	static int time_sec = 0;
	printf("T = %ds:\n", time_sec);
	time_sec++;
}
K_TIMER_DEFINE(time_keeper, print_time, NULL);

/**
 * @brief Initializes periodic timer interrupt to print time in seconds
 */
void start_time_keeper(void)
{
	k_timer_start(&time_keeper, K_NO_WAIT, K_MSEC(1000));
}
