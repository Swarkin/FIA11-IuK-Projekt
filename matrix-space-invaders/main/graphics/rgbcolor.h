#ifndef RGBCOLOR_H
#define RGBCOLOR_H

#include <stdint.h>

struct RGBColor {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

static const struct RGBColor RGBCOLOR_WHITE = { 255, 255, 255 };
static const struct RGBColor RGBCOLOR_BLACK = { 0, 0, 0 };

static const struct RGBColor RGBCOLOR_RED   = { 255, 0, 0 };
static const struct RGBColor RGBCOLOR_GREEN = { 0, 255, 0 };
static const struct RGBColor RGBCOLOR_BLUE  = { 0, 0, 255 };

#endif // RGBCOLOR_H
