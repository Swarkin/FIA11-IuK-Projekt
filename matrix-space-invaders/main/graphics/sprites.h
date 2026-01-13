#ifndef SPRITES_H
#define SPRITES_H

#include <stdint.h>

struct Sprite {
	uint8_t width;
	uint8_t height;
	uint8_t frames;
	const uint8_t* pixels;
};


static const uint8_t ALIEN1_PIXELS[] = {
	0b00111000,
	0b01101110,
	0b01101001,
	0b11111101,
	0b11111101,
	0b01101001,
	0b01101110,
	0b00111000,

	0b00111001,
	0b01101010,
	0b01101100,
	0b11111010,
	0b11111010,
	0b01101100,
	0b01101010,
	0b00111001,
};

static const struct Sprite ALIEN1_SPRITE = {
	.width = 8,
	.height = 8,
	.frames = 2,
	.pixels = ALIEN1_PIXELS,
};


static const uint8_t ALIEN2_PIXELS[] = {
	0b00011100,
	0b00111011,
	0b11101101,
	0b01111100,
	0b11101101,
	0b00111011,
	0b00011100,

	0b11110001,
	0b00111110,
	0b11101100,
	0b01111100,
	0b11101100,
	0b00111110,
	0b11110001,
};

static const struct Sprite ALIEN2_SPRITE = {
	.width = 8,
	.height = 7,
	.frames = 2,
	.pixels = ALIEN2_PIXELS,
};


static const uint8_t ALIEN3_PIXELS[] = {
	0b00011010,
	0b00101101,
	0b01111000,
	0b01111000,
	0b00101101,
	0b00011010,

	0b00011001,
	0b00101010,
	0b01111101,
	0b01111101,
	0b00101010,
	0b00011001,
};

static const struct Sprite ALIEN3_SPRITE = {
	.width = 8,
	.height = 6,
	.frames = 2,
	.pixels = ALIEN3_PIXELS,
};


static const uint8_t ALIEN4_PIXELS[] = {
	0b00111000,
	0b01101110,
	0b01110100,
	0b01101110,
	0b00111000,

	0b00111010,
	0b01101100,
	0b01110100,
	0b01101100,
	0b00111010,
};

static const struct Sprite ALIEN4_SPRITE = {
	.width = 8,
	.height = 5,
	.frames = 2,
	.pixels = ALIEN4_PIXELS,
};


static const uint8_t EXPLOSION_PIXELS[] = {
	0b01001001,
	0b00101010,
	0b00000000,
	0b01100011,
	0b00000000,
	0b00101010,
	0b01001001,
};

static const struct Sprite EXPLOSION_SPRITE = {
	.width = 8,
	.height = 7,
	.frames = 1,
	.pixels = EXPLOSION_PIXELS,
};


static const uint8_t SHIP_PIXELS[] = {
	0b00001111,
	0b00000111,
	0b00111110,
	0b11101111,
	0b00111110,
	0b00000111,
	0b00001111,
};

static const struct Sprite SHIP_SPRITE = {
	.width = 8,
	.height = 8,
	.frames = 1,
	.pixels = SHIP_PIXELS,
};


static const uint8_t LASER_PIXELS[] = {
	0b11111111,
};

static const struct Sprite LASER_SPRITE = {
	.width = 8,
	.height = 1,
	.frames = 1,
	.pixels = LASER_PIXELS,
};

#endif // SPRITES_H
