#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "graphics/rgbcolor.h"
#include "graphics/position.h"

#define GAME_MAX_ENEMIES 12
#define GAME_MAX_PLAYER_HP 3
#define GAME_MAX_ENEMY_HP 2

struct Entity {
	int hp;
	struct Position pos;
	struct RGBColor color;
	const struct Sprite* sprite;
};

struct Game {
	int score;
	struct Entity player;
	struct Entity bullet;
	struct Entity enemies[GAME_MAX_ENEMIES];
};

struct Game game_create();

void game_enemy_set(struct Game* game, struct Position pos, struct RGBColor color, const struct Sprite* sprite, uint8_t i);
void game_enemy_damage(struct Game* game, uint8_t i, int damage);

void game_bullet_fire(struct Game* game, struct Position pos, int hp);

#endif // GAME_H
