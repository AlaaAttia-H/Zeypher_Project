#include <zephyr/kernel.h>

#include "acceptance_test.h"
#include "display.h"

K_THREAD_STACK_ARRAY_DEFINE(task_stacks, CONFIG_NUM_TASKS, STACKSIZE);

TASK_SETUP(1);
TASK_SETUP(2);
TASK_SETUP(3);

struct task_params *task_set[] = {
	&task1_params,
	&task2_params,
	&task3_params,
};

struct k_thread task_threads[CONFIG_NUM_TASKS];

int main()
{
	AcceptanceTestResult default_result = {false, {0, 0, 0}};
	AcceptanceTestResult results[] = {default_result, default_result, default_result};

	// check acceptance test and store decision in results array
	for (unsigned int i = 0; i < CONFIG_NUM_TASKS; i++) {
		acceptance_test(task_set, i, results);
		if (!results[i].accepted) {
			continue;
		}

		// if accepted, create the task
		k_thread_create(&task_threads[i], task_stacks[i], STACKSIZE, task_implementation,
				task_set[i], &results[i].info, NULL, task_set[i]->priority, 0,
				K_NO_WAIT);
	}

	start_time_keeper();

	while (1) {
		for (unsigned int i = 0; i < CONFIG_NUM_TASKS; i++) {
			ssd1306_print_task_info(task_set[i], &results[i].info);
			k_sleep(K_MSEC(1000));
		}
	}

	return 0;
}
