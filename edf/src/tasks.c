#include <assert.h>
#include <stdio.h>

#include "display.h"
#include "tasks.h"

/**
 * @brief Let LED blink for (roughly) delay_ms.
 *
 * @param params    Task parameters
 */
void useless_load_periodic_tasks(struct task_params *params)
{
	assert(params->execution_time_ms % BLINK_PERIOD_MS == 0 && BLINK_PERIOD_MS % 2 == 0);

	// Lock scheduler to avoid time sharing if the deadline of two or multiple tasks is equal
	k_sched_lock();
	for (int32_t d = 0; d < params->execution_time_ms; d += BLINK_PERIOD_MS) {
		printf(" Task %d with deadline %d\n", params->task_id,
		       params->thread->base.prio_deadline);
		gpio_pin_set_dt(params->led, 1);
		k_busy_wait(BLINK_PERIOD_MS * 1000 / 2);
		gpio_pin_set_dt(params->led, 0);
		k_busy_wait(BLINK_PERIOD_MS * 1000 / 2);
	}
	k_sched_unlock();
	/* Clear display after the aperiodic job finishes, so the snowman doesn't stay on always, this fcn is created in display.c*/
    ssd1306_clear_screen();
}
	
/**
 * @brief Show animation on SSD1306 for one second
 */
void useless_load_aperiodic_tasks()
{
	k_sched_lock();
	for (int32_t d = 0; d < MSEC_PER_SEC; d += BLINK_PERIOD_MS) {
		printf(" Aperiodic task\n");
		ssd1306_print_aperiodic_task();
		k_busy_wait(BLINK_PERIOD_MS * 1000);
	}
	k_sched_unlock();

}

/**
 * @brief Simplified version of z_impl_k_thread_deadline_set
 *
 * Unfortunately, z_impl_k_thread_deadline_set relies on deadlines
 * relative to the CPU cycle counter, while casting the result in
 * int32_t. For our dummy tasks, with periods and latencies in the
 * range of multiple seconds, this will result in frequent overflows
 * that breaks EDF as a consequence.
 *
 * @param thread    thread handle
 * @param deadline  thread deadline in milliseconds
 */
void thread_deadline_set(struct k_thread *thread, int deadline)
{
	/* TODO (Part a): Write a simplified version of z_impl_k_thread_deadline_set
	 * that specified the deadline in milliseconds rather than
	 * in CPU cycle counts.
	 * Hint: You can use k_yield() after updating the deadline
	 * to dequeue the thread from the ready queue. */
	// Absolute deadline = i*task_period (CORRECTED, since we pass i * params->period_ms in the fcn anyways)
	thread->base.prio_deadline = deadline;
	k_yield();

}

/**
 * @brief System density test for periodic tasks
 *
 * @params params task specification
 */
bool acceptance_test(struct task_params *params)
{
	static bool acceptance_test_results[CONFIG_NUM_TASKS];

	/* TODO (Part a): The system density test should account for the deferrable server (DS).
	 * That is, we assume the DS is always accepted and the acceptance is solely performed for
	 * periodic tasks. Moreover the acceptance test is called iteratively (see main.c), meaning
	 * that Task i must only account for the previously accepted tasks.
	 *
	 * Your solution should
	 *  - use STRUCT_SECTION_FOREACH for retrieving the task specification of all other tasks
	 *    (see main.c for reference)
	 *  - store the acceptance test result in *acceptance_test_results*
	 * */
	double density = 0.0;
	STRUCT_SECTION_FOREACH(task_params, other) {

		/* Include the deferrable server as it's always accepted */
		if (other->type == DEFERRABLE_SERVER) {
			// For the DS task, density is u_s*(1+ (p_s - e_s)/d_i) and since p_s = d_i; p_s/d_i = 1
			double utilization = (double)other->execution_time_ms /
            (double)other->period_ms;
            density += utilization * (1+ (1 - ((double)other->execution_time_ms /
                       (double)other->period_ms)));
            continue;
		}
		/* Include current task */
		if (other->task_id == params->task_id) {
			density += (double)params->execution_time_ms /
					(double)params->period_ms;
			continue;
		}
       /* Include only previously accepted higher-priority tasks */
        if (other->task_id < params->task_id &&
            acceptance_test_results[other->task_id]) {

            density += (double)other->execution_time_ms /
                       (double)other->period_ms;
		}
	}
	/* Check EDF schedulability, if density <= 1 then this task is scheduleable */
	bool accepted = (density <= 1.0);
	/* store the acceptance test result in *acceptance_test_results* */
	acceptance_test_results[params->task_id] = accepted;
	return accepted;
}

/**
 * @brief Dummy task implementation that toggles LED
 */
void periodic_task_implementation(void *p0, void *p1, void *p2)
{
	struct task_params *params = (struct task_params *)p0;

	if (!gpio_is_ready_dt(params->led)) {
		return;
	}
	int ret = gpio_pin_configure_dt(params->led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	k_timer_start(params->timer, K_NO_WAIT, K_MSEC(params->period_ms));
	for (int32_t i = 1;; i++) {
		k_timer_status_sync(params->timer);
		thread_deadline_set(params->thread, i * params->period_ms);
		useless_load_periodic_tasks(params);
	}
}

/* TODO (Part b): Configure the button in the DTS overlay and initialize the GPIO driver here.
 * The goal is to release an aperiodic job whenever the button is pressed and to use the deferrable
 * server (as introduced in the lecture) to schedule them. To this end, you have to complete
 * the implementation of
 *  - button_press_callback
 *  - init_button_gpio
 *  - deferrable_server_implementation
 * */

/* Semaphore used to signal that an aperiodic job exists.
* When the semaphore value is 0, the deferrable server thread cannot be run and is removed from the scheduler.
* When the semaphore value becomes 1, the deferrable server can run */

K_SEM_DEFINE(button_sem, 0, 1);

void button_press_callback()
{
	/* Utilize a suitable synchronization primitive to let the DS know that
	 * the button was pressed. Note that
	 *  - Interrupt service routines (ISRs) are very different from normal threads and many
	 *    kernel APIs behave differently when called from an ISR or from a thread. The
	 *    implementation of this ISR should therefore be as lightweight as possible. For more
	 *    information, see https://docs.zephyrproject.org/latest/kernel/services/interrupts.html
	 *  - Deferrable servers are supposed to yield their execution to the scheduler if there
	 *    are no asynchronous jobs available. Thus, your synchronization primitive must not
	 *    result in busy waiting that may start periodic tasks.
	 * */
	/*Once this fcn is called due to the button being pressed, the semaphore value increased to one, 
	and the deferrable server can run now*/
	k_sem_give(&button_sem);
}

int init_button_gpio()
{
	/* TODO: Initialize the GPIO pin to trigger a callback to *button_press_callback* whenever
	 *the button is pressed. For reference, you may want to have a look at:
	 *https://docs.zephyrproject.org/latest/samples/basic/button/README.html */
	static struct gpio_callback button_cb;
	// To find the button in the overlay file
    const struct gpio_dt_spec button =
        GPIO_DT_SPEC_GET(DT_CHOSEN(sw0), gpios);
	// Check that button is ready
    if (!gpio_is_ready_dt(&button)) {
        return -ENODEV;
    }
	// Configure the button as input
    int ret = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        return ret;
    }
	// Activate interrupt
    ret = gpio_pin_interrupt_configure_dt(
        &button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        return ret;
    }
	// Initializes button_press_callback() fcn
    gpio_init_callback(
        &button_cb,
        (void (*)(const struct device *,
                  struct gpio_callback *,
                  uint32_t))button_press_callback,
        BIT(button.pin));
	//Button press --> Interrupt triggered and call button_press_callback()
    gpio_add_callback(button.port, &button_cb);

    return 0;
}

void deferrable_server_implementation(void *p0, void *p1, void *p2)
{
	struct task_params *params = (struct task_params *)p0;

	/* TODO: Implement a deferrable server that replenishes periodically, but
	 * that does not accumulate its budget over multiple cycles. For simplicity,
	 * you may assume the following:
	 *  - Every aperiodic job is identical and wants to run *useless_load_aperiodic_tasks()*
	 *  - The budget of the DS is equal to the budget of one aperiodic job per period
	 * As a starting point, you can likely reuse parts of *periodic_task_implementation*.
	 * */
	
	int period_count = 1;
    int budget_ms;
	// Deferrable server period is defined as 7 seconds (budget replenishment points)
	k_timer_start(params->timer, K_NO_WAIT, K_MSEC(params->period_ms));
	for (;;) {
		k_timer_status_sync(params->timer);
		/* Set the budget */
		budget_ms = params->execution_time_ms;

		/* Sets EDF deadline*/
        thread_deadline_set(params->thread,
                            period_count * params->period_ms);
        period_count++;
        /* Removes any old button presses from previous periods */
        while (k_sem_take(&button_sem, K_NO_WAIT) == 0) {
            /* do nothing */
        }
        /* Now wait for a button press for this period*/
		/* Serve aperiodic jobs while budget allows */
        while (budget_ms > 0) {
        	if (k_sem_take(&button_sem, K_MSEC(params->period_ms)) == 0) {
				/* Do the task */
				useless_load_aperiodic_tasks();
				/* Use the budget */
				budget_ms -= params->execution_time_ms;
			}
		}
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
