#include "tasks.h"
#include <assert.h>
#include <stdio.h>

/**
 * @brief TODO: Let LED blink for (roughly) delay_ms.
 *
 * @param led	    LED device.
 * @param delay_ms  Delay in milliseconds. Must be a multiple of
 *		    BLINK_PERIOD_MS.
 */
void useless_load(int task_id, const struct gpio_dt_spec *led, int32_t delay_ms)
{
	assert(delay_ms % BLINK_PERIOD_MS == 0 && BLINK_PERIOD_MS % 2 == 0);

	// Let LED blink for (roughly) delay_ms
	for (int32_t d = 0; d < delay_ms; d += BLINK_PERIOD_MS) {
		printf(" Task %d\n", task_id);

		/** Modify this method so that *led* is blinking while this loop is
		 * executed. Afterwards, the LED should be in the OFF state. */
		
		// Toggle LED ON
		gpio_pin_set_dt(led, 1);
		k_busy_wait(BLINK_PERIOD_MS * 1000 / 2);  // Half period ON (125ms)
		// Toggle LED OFF
		gpio_pin_set_dt(led, 0);
		k_busy_wait(BLINK_PERIOD_MS * 1000 / 2);  // Half period OFF (125ms)
	}
	
	// Ensure LED is OFF after execution
	gpio_pin_set_dt(led, 0);
}

/**
 * @brief TODO: Implementation of the chatterbox task
 */
void chatterbox_task(struct task_params *params)
{
	/* Implement the chatterbox task for generic *task_params.
	 * This consists of:
	 *  - Initializing the GPIO pin for params->led
	 *  - Periodically executing the task by calling useless_load
	 *  - Suspending the task until the next cycle
	 */
	gpio_pin_configure_dt(params->led, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set_dt(params->led, 0); /* start OFF */

	int64_t next_release = k_uptime_get();  /* first activation time (absolute) */

	while (1) {
		/* Do the execution window (blinking handled inside) */
		useless_load(params->task_id, params->led, params->execution_time_ms);

		/* Compute absolute next release: release + n*period */
		next_release += params->period_ms;

		/* Wait until that absolute instant using a one-shot timer (no k_msleep) */
		int64_t now = k_uptime_get();
		int32_t wait_ms = (int32_t)(next_release - now);
		if (wait_ms > 0) {
			k_timer_start(params->timer, K_MSEC(wait_ms), K_NO_WAIT);
			k_timer_status_sync(params->timer);  /* precise wakeup at period boundary */
			k_timer_stop(params->timer);
		}
		/* If wait_ms <= 0: we're late/overrun; immediately start next cycle. */
	}
}

/** TODO: After correctly setting the GPIO assignments for
 * Task 2 and 3 in boards/esp_wrover_kit_procpu.overlay,
 * uncomment the following lines.
 */
INITIALIZE_TASK(1);
INITIALIZE_TASK(2);
INITIALIZE_TASK(3);
