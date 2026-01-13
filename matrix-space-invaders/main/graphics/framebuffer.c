#include "framebuffer.h"

struct Framebuffer framebuffer_init(int width, int height, enum FramebufferType type) {
	struct Framebuffer fb;
	fb.width = width;
	fb.height = height;
	fb.type = type;
	fb.data = (uint8_t*)calloc(width * height * type, sizeof(uint8_t));
	return fb;
}

void framebuffer_uninit(struct Framebuffer* fb) {
	if (fb == NULL || fb->data == NULL) { return; }

	free(fb->data);
	fb->data = NULL;
}

void framebuffer_clear(struct Framebuffer* fb) {
	if (fb == NULL || fb->data == NULL) { return; }

	int size = fb->width * fb->height * fb->type;
	for (int i = 0; i < size; i++) {
		fb->data[i] = 0;
	}
}

void framebuffer_draw_sprite(struct Framebuffer* fb, const struct Sprite* sprite, int frame, struct Position* pos, const struct RGBColor* color) {
	if (fb == NULL || fb->data == NULL
		|| sprite == NULL || sprite->pixels == NULL || frame >= sprite->frames
		|| pos == NULL
		|| color == NULL
	) { return; }

	int frame_offset = (sprite->height * ((sprite->width + 7) / 8)) * frame;

	for (int sy = 0; sy < sprite->height; sy++) {
		for (int sx = 0; sx < sprite->width; sx++) {
			int byte_index = frame_offset + sy * ((sprite->width + 7) / 8) + (sx / 8);
			int bit_index = 7 - (sx % 8);

			int fb_x = pos->x + sx;
			int fb_y = pos->y + sy;

			if (fb_x >= 0 && fb_x < fb->width && fb_y >= 0 && fb_y < fb->height) {
				int index = (fb_y * fb->width + fb_x) * fb->type;

				if (sprite->pixels[byte_index] & (1 << bit_index)) {
					fb->data[index] = color->r;
					fb->data[index + 1] = color->g;
					fb->data[index + 2] = color->b;
				}/* else {
					fb->data[index] = 0;
					fb->data[index + 1] = 0;
					fb->data[index + 2] = 0;
				}*/

				if (fb->type == FRAMEBUFFER_TYPE_RGBA) {
					fb->data[index + 3] = 255;
				}
			}
		}
	}
}
