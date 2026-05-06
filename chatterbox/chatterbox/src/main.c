#include <stdio.h>
#include <zephyr/kernel.h>

/**
 * @brief Print time every second.
 */
void print_time()
{
	static int time_sec = 0;
	printf("T = %ds:\n", time_sec);
	time_sec++;
}
K_TIMER_DEFINE(time_keeper, print_time, NULL);

int main()
{
	k_timer_start(&time_keeper, K_NO_WAIT, K_MSEC(1000));

	return 0;
}
