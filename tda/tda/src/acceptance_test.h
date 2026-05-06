#pragma once

#include <zephyr/kernel.h>

#include "display.h"

typedef struct {
	bool accepted;
	struct task_info info;
} AcceptanceTestResult;

/* Determine if params[task_id] can be scheduled.
 * - params: array of all task parameters (e.g., needed to perform TDA)
 * - task_id: index such that params[task_id] is the task under consideration
 * - results: output parameter yielding the acceptance test result */
void acceptance_test(struct task_params **params, unsigned int task_id,
		     AcceptanceTestResult result[]);
