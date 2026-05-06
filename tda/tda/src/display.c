#include "display.h"
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

#include <zephyr/display/cfb.h>
#include <zephyr/drivers/display.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display);

#define SSD1306 DT_CHOSEN(display)
static const struct device *dev = DEVICE_DT_GET(SSD1306);

#define BUFFER_SIZE 10
static char buf[BUFFER_SIZE];

int ssd1306_setup(void)
{
	if (!device_is_ready(dev)) {
		LOG_ERR("Display device not ready");
		return -EIO;
	}

	if (display_set_pixel_format(dev, PIXEL_FORMAT_MONO01)) {
		LOG_ERR("Failed to set required pixel format");
		return -EIO;
	}
	if (display_blanking_off(dev)) {
		LOG_ERR("Failed to turn off display blanking");
		return -EIO;
	}
	if (cfb_framebuffer_init(dev)) {
		LOG_ERR("Failed to initialize character framebuffer");
		return -EIO;
	}
	cfb_framebuffer_invert(dev);
	return 0;
}
SYS_INIT(ssd1306_setup, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);

void ssd1306_print_task_info(struct task_params *params, struct task_info *info)
{
	// Clear the display first since values werelingering from previous screens
	cfb_framebuffer_clear(dev, false);
	snprintf(buf, sizeof(buf), "Task %d", params->task_id);
	cfb_print(dev, buf, 0, 0);
	snprintf(buf, sizeof(buf), "Utl %.3f", info->util);
	cfb_print(dev, buf, 0, 16);
	snprintf(buf, sizeof(buf), "WCS %d", info->wcs_result);
	cfb_print(dev, buf, 0, 32);
	snprintf(buf, sizeof(buf), "TDA %d", info->tda_result);
	cfb_print(dev, buf, 0, 46);

	cfb_framebuffer_finalize(dev);
}
