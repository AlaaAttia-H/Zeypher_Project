#include <math.h>
#include <stdio.h>
#include "acceptance_test.h"

void utilization_bound_test(struct task_params **params, unsigned int task_id,
			    AcceptanceTestResult results[])
{
	double utilization = 0;
	bool accepted = false;
	
	/*--------------------------------------------------------------------
	 * TODO 1: Implement the Utilization Bound Test from the lecture
	 ---------------------------------------------------------------------*/

	 // n is the number of tasks in the accepted set of tasks
	 int n = 0; 

	 /*--------------------------------------------------------------------
	 * While loop to calculate utilization:
	 * If utilization > 1 ----> Task is impossible so "Rejected".
	 * If utilization <= U_RM ----> Task scheduling is gurantedd and "Accepted".
	 * Else ----> Task scheduling isn't guranteed so further testing is needed (WCS).
	 ---------------------------------------------------------------------*/

	for (unsigned int k = 0; k <= task_id; k++) {
		// This condition  k == task_id is to make sure that the task itself is included for testing intitally
		if (k == task_id || results[k].accepted) {
			double e_k = params[k]->execution_time_ms;
			double p_k = params[k]->period_ms;
			utilization += e_k / p_k;
			n++;
		}
	}
	
	double U_RM = n * (pow(2.0, 1.0 / n) - 1.0);

	if (utilization > 1.0) {
		accepted = false;       
	} else if (utilization <= U_RM) {
		accepted = true;       
	} else {
		accepted = false;     
	}
	results[task_id].accepted = accepted;
	results[task_id].info.util = utilization;

	printf("Utiliization Test (Task %d): %f (%s)\n", task_id + 1, utilization,
	       accepted ? "accepted" : "rejected");
}

void worst_case_simulation(struct task_params **params, unsigned int task_id,
			   AcceptanceTestResult results[])
{
	int32_t completion_time = 0;
	bool accepted = false;
	
	/*--------------------------------------------------------------------
	 * TODO 2: Implement the Worst Case Simulation from the lecture

	 * WCS is a sufficient test:
	 *   - if it accepts ----> definitely schedulable.
	 *   - if it rejects ----> not final, TDA must still be checked.
	 ---------------------------------------------------------------------*/
	completion_time += params[task_id]->execution_time_ms; // C_WCS is initially e_i
	
	int32_t p_i = params[task_id]->period_ms;

	for (unsigned int k = 0; k < task_id; k++) {

		// Skip rejected tasks
		if (!results[k].accepted) {
			continue;
		}

		int32_t p_k = params[k]->period_ms;
		int32_t e_k = params[k]->execution_time_ms;
		//Calculate ceil([p_i/k_k]) which is how many jobs of each higher priority task T_k can delay T_i in the worst case.
		int32_t jobs = (p_i + p_k - 1) / p_k;
		completion_time += jobs * e_k;     //C_WCS = e_i + (p_i/p_k)*e_k
	}

		// For this assignment the deadline is equal to the period, so if C_WCS <= d_i, then the task set is schedulable.
		if (completion_time <= p_i) {
		accepted = true;
	

	}
	// ___________________________ END _____________________________________
	results[task_id].accepted = accepted;
	results[task_id].info.wcs_result = completion_time;

	printf("Worst Case Simulation (Task %d): %d ms (%s)\n", task_id + 1, completion_time,
	       accepted ? "accepted" : "rejected");
}

void time_demand_analysis(struct task_params **params, unsigned int task_id,
			  AcceptanceTestResult results[])
{
	int32_t t_next = 0;
	bool accepted = false;

	/*--------------------------------------------------------------------
	 * TODO 3: Implement the Time Demand Analysis from the lecture
	 ---------------------------------------------------------------------*/
	int32_t deadline = params[task_id]->period_ms;
	int32_t e_i = params[task_id]->execution_time_ms;

	int32_t t = 0;
	for (unsigned int k = 0; k <= task_id; k++) {
		if (k == task_id || results[k].accepted) {
			t += params[k]->execution_time_ms;
		}
	}

	while (1) {
		int32_t w = e_i;

		for (unsigned int k = 0; k < task_id; k++) {
			if (!results[k].accepted) {
				continue;
			}

			int32_t p_k = params[k]->period_ms;
			int32_t e_k = params[k]->execution_time_ms;

			int32_t jobs = (t + p_k - 1) / p_k;
			w += jobs * e_k;
		}

		t_next = w;

		if (t_next == t) {
			accepted = (t_next <= deadline);
			break;
		}

		if (t_next > deadline) {
			accepted = false;
			break;
		}

		t = t_next;
	}

	results[task_id].accepted = accepted;
	results[task_id].info.tda_result = t_next;

	printf("Time Demand Analysis (Task %d): %d ms (%s)\n", task_id + 1, t_next,
	       accepted ? "accepted" : "rejected");
}

/* Determine if params[task_id] can be scheduled.
 * - params: array of all task parameters (e.g., needed to perform TDA)
 * - task_id: index such that params[task_id] is the task under consideration
 * - results: output parameter yielding the acceptance test result */
void acceptance_test(struct task_params **params, unsigned int task_id,
		     AcceptanceTestResult results[])
{
	/*--------------------------------------------------------------------
	 * TODO 4: Call the above acceptance tests in a suitable order.
	 *  In particular, recall which of these tests are necessary,
	 *  sufficient, or both.
	 *  Ensure that the final value of result->accepted is true if and
	 *  only if the task encoded by params[task_id] can be scheduled.
	 ---------------------------------------------------------------------*/

	 // The tests are organised from least complex to most complex

	 /*Utilization Bound Test*/

	utilization_bound_test(params, task_id, results);

	double utilization = results[task_id].info.util;

	/*Case 1: utilization > 1 ; this is impossible so rejected with no further testing needed*/
	if (utilization > 1.0) {
		return;
	}

	/* Case 2: utilization <= U_RM ; Task is definitely accepted and so no further testing needs to be done*/
	if (results[task_id].accepted) {
		return;
	}

	/* Case 3: U_RM < utilization <= 1; Test is inconclusive so we have to perform the other tests */



	/* Worst Case Simulation*/

	worst_case_simulation(params, task_id, results);

	/* Case 1: WCS accepts the task ;  Task is definitely accepted and so no further testing needs to be done */
	if (results[task_id].accepted) {
		return;
	}

	/* Case 2: WCS rejects the task ; WCS rejection is NOT final so we must do futher testing (TDA) */

	/*Time Demand Analysis (Final conlusive test) */
	time_demand_analysis(params, task_id, results);


}
