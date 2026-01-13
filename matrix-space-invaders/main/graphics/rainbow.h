#ifndef RAINBOW_H
#define RAINBOW_H

#include <stdint.h>
#include "rgbcolor.h"

const uint8_t hsv_power_lookup[121] = {
	0, 2, 4, 6, 8, 11, 13, 15, 17, 19, 21, 23, 25, 28, 30, 32, 34, 36, 38, 40,
	42, 45, 47, 49, 51, 53, 55, 57, 59, 62, 64, 66, 68, 70, 72, 74, 76, 79, 81,
	83, 85, 87, 89, 91, 93, 96, 98, 100, 102, 104, 106, 108, 110, 113, 115, 117,
	119, 121, 123, 125, 127, 130, 132, 134, 136, 138, 140, 142, 144, 147, 149,
	151, 153, 155, 157, 159, 161, 164, 166, 168, 170, 172, 174, 176, 178, 181,
	183, 185, 187, 189, 191, 193, 195, 198, 200, 202, 204, 206, 208, 210, 212,
	215, 217, 219, 221, 223, 225, 227, 229, 232, 234, 236, 238, 240, 242, 244,
	246, 249, 251, 253, 255
};

struct RGBColor power_hsv(int angle) {
	angle = angle % 360;
	if (angle < 0) angle += 360;

	struct RGBColor color;

	if (angle < 120) {
		color.r = hsv_power_lookup[120-angle];
		color.g = hsv_power_lookup[angle];
		color.b = 0;
	} else if (angle < 240) {
		color.r = 0;
		color.g = hsv_power_lookup[240-angle];
		color.b = hsv_power_lookup[angle-120];
	} else {
		color.r = hsv_power_lookup[angle-240];
		color.g = 0;
		color.b = hsv_power_lookup[360-angle];
	}

	return color;
}

#endif // RAINBOW_H
