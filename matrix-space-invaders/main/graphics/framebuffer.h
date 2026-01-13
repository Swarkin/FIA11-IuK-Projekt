#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdlib.h>
#include <stdint.h>

#include "sprites.h"
#include "rgbcolor.h"
#include "position.h"

enum FramebufferType {
	FRAMEBUFFER_TYPE_RGB = 3,
	FRAMEBUFFER_TYPE_RGBA = 4,
};

// RGBA framebuffer
struct Framebuffer {
	int width;
	int height;
	enum FramebufferType type;
	uint8_t* data;
};

struct Framebuffer framebuffer_init(int width, int height, enum FramebufferType type);
void framebuffer_uninit(struct Framebuffer* fb);

void framebuffer_clear(struct Framebuffer* fb);
void framebuffer_draw_sprite(struct Framebuffer* fb, const struct Sprite* sprite, int frame, struct Position* pos, const struct RGBColor* color);

#endif // FRAMEBUFFER_H
