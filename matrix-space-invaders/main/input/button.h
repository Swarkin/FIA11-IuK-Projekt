#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include "driver/gpio.h"

struct Button {
	gpio_num_t pin;
};

struct Button button_init(gpio_num_t pin);
bool button_pressed(struct Button* button);
void button_deinit(struct Button* button);

#endif // BUTTON_H
