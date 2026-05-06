#include "display.h"
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

#include <zephyr/display/cfb.h>
#include <zephyr/drivers/display.h>
#include <stdlib.h>

#define SSD1306 DT_CHOSEN(display)
static const struct device *dev = DEVICE_DT_GET(SSD1306);
static const uint32_t display_width = DT_PROP(SSD1306, width);
static const uint32_t display_height = DT_PROP(SSD1306, height);

int ssd1306_setup(void)
{
	if (!device_is_ready(dev)) {
		printf("ERROR: Display device not ready");
		return -EIO;
	}

	if (display_set_pixel_format(dev, PIXEL_FORMAT_MONO01)) {
		printf("ERROR: Failed to set required pixel format");
		return -EIO;
	}

	if (display_blanking_off(dev)) {
		printf("ERROR: Failed to turn off display blanking");
		return -EIO;
	}

	if (cfb_framebuffer_init(dev)) {
		printf("ERROR: Failed to initialize character framebuffer");
		return -EIO;
	}
	cfb_framebuffer_invert(dev);
	cfb_framebuffer_clear(dev, true);
	return 0;
}
SYS_INIT(ssd1306_setup, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);

/* @brief Draw bitmap
 *
 * @param dev	  pointer to device structure for driver instance
 * @param bitmap  bitmap of size 128x64
 *
 * @return 0 on success, negative value otherwise
 */
int cfb_draw_bitmap(const struct device *dev, const unsigned char *bitmap)
{
	struct cfb_position pos;
	uint16_t byteWidth = display_width / 8;
	for (pos.y = 0; pos.y < display_height; pos.y++) {
		for (pos.x = 0; pos.x < display_width; pos.x++) {
			if (bitmap[pos.y * byteWidth + pos.x / 8] & (128 >> (pos.x & 7))) {
				cfb_draw_point(dev, &pos);
			}
		}
	}

	return 0;
}

/* @brief Display snowman bitmap and CONFIG_SNOWFLAKES randomly generated snowflakes
 *
 * For some reason, the SSD1306 display does not always respond, which is often
 * because it was not correctly initialized. While unsatisfactory, the simplest
 * approach seems to be to restart the ESP or to reconnect the USB cable.
 */
void ssd1306_print_aperiodic_task()
{
	if (!device_is_ready(dev)) {
		printf("ERROR: Display device not ready");
		printf("Check your wiring or try reconnecting your USB cable");
		return;
	}

	cfb_framebuffer_clear(dev, false);
	cfb_draw_bitmap(dev, snowman_bitmap);

	struct cfb_position pos;
	for (int i = 0; i < CONFIG_SNOWFLAKES; i++) {
		pos.x = rand() % display_width;
		pos.y = rand() % display_height;
		cfb_draw_point(dev, &pos);
	}
	cfb_framebuffer_finalize(dev);
}
/* Added this fcn since the snowman always stays on the screen after the first time the aperiodic task is implemented 
And now the screen gets deleted after the task is completed*/ 
void ssd1306_clear_screen(void)
{
    if (!device_is_ready(dev)) {
        return;
    }

    cfb_framebuffer_clear(dev, true);
    cfb_framebuffer_finalize(dev);
}