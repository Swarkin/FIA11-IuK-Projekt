#include "button.h"

struct Button button_init(gpio_num_t pin) {
	struct Button button;
	button.pin = pin;

	ESP_ERROR_CHECK( gpio_reset_pin(pin) );
	ESP_ERROR_CHECK( gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY) );
	ESP_ERROR_CHECK( gpio_set_direction(pin, GPIO_MODE_INPUT) );

	return button;
}

bool button_pressed(struct Button* button) {
	return !gpio_get_level(button->pin);
}

void button_deinit(struct Button* button) {
	ESP_ERROR_CHECK( gpio_reset_pin(button->pin) );
}
