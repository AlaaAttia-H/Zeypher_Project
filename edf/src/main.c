#include <zephyr/kernel.h>

#include "tasks.h"

K_THREAD_STACK_ARRAY_DEFINE(task_stacks, CONFIG_NUM_TASKS, STACKSIZE);

PERIODIC_TASK_SETUP(1);
PERIODIC_TASK_SETUP(2);
PERIODIC_TASK_SETUP(3);
DEFERRABLE_SERVER_SETUP(4);

int main()
{
	/* Compared to the previous exercises, we now utilize slightly more advanced iterable
	 * sections that simplify task initialization. For more information, please refer to
	 *  - https://docs.zephyrproject.org/latest/kernel/iterable_sections/index.html
	 *  - https://docs.zephyrproject.org/latest/doxygen/html/group__iterable__section__apis.html
	 */
	// Initialize the button fcn
	init_button_gpio();
	STRUCT_SECTION_FOREACH(task_params, params) {
		if (params->type == DEFERRABLE_SERVER || acceptance_test(params)) {
			k_thread_create(params->thread, task_stacks[params->task_id], STACKSIZE,
					params->func, params, NULL, NULL, params->priority, 0,
					K_NO_WAIT);
		}
	}

	start_time_keeper();

	return 0;
}
