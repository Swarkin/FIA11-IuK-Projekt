#include "game.h"

struct Game game_create() {
	struct Game game = {0};

	game.player.hp = GAME_MAX_PLAYER_HP;

	return game;
}

void game_enemy_set(struct Game* game, struct Position pos, struct RGBColor color, const struct Sprite* sprite, uint8_t i) {
	if (i >= GAME_MAX_ENEMIES) { return; }

	struct Entity* enemy = &game->enemies[i];
	enemy->pos = pos;
	enemy->hp = GAME_MAX_ENEMY_HP;
	enemy->color = color;
	enemy->sprite = sprite;
}

void game_enemy_damage(struct Game* game, uint8_t i, int damage) {
	if (i >= GAME_MAX_ENEMIES) { return; }

	struct Entity* enemy = &game->enemies[i];
	if (enemy->hp > damage) {
		enemy->hp -= damage;
	} else {
		enemy->hp = 0;
	}
}

void game_bullet_fire(struct Game* game, struct Position pos, int hp) {
	game->bullet.pos = pos;
	game->bullet.hp = hp;
}
