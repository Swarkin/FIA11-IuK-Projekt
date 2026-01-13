#include <stdlib.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "wifi_init.h"
#include "mqtt_init.h"

#include "app_struct.h"
#include "matrix_wrapper.h"
#include "graphics/framebuffer.h"
#include "graphics/rainbow.h"
#include "game/game.h"
#include "input/button.h"
#include "input/mqtt_joystick.h"

#define MATRIX_WIDTH 128
#define MATRIX_HEIGHT 32
#define FRAMEBUFFER_TYPE FRAMEBUFFER_TYPE_RGB
#define BUTTON1_GPIO GPIO_NUM_9
#define BUTTON2_GPIO GPIO_NUM_10
#define FPS 20

//static const char* TAG = "app";

esp_err_t init(struct app_data *app) {
	esp_err_t err;

	matrix_begin(MATRIX_WIDTH, MATRIX_HEIGHT, 1, 17);
	matrix_setBrightness8(8);
	matrix_clearScreen();
	matrix_drawStringRGB888(6, 6, "STARTING...", 255, 255, 255);

	app->wifi_event_group = xEventGroupCreate();
	if (app->wifi_event_group == NULL) { return ESP_FAIL; }
	app->mqtt_event_group = xEventGroupCreate();
	if (app->mqtt_event_group == NULL) { return ESP_FAIL; }
	app->mqtt_pending_data = xQueueCreate(1, sizeof(struct joystick_reading));
	if (app->mqtt_pending_data == NULL) { return ESP_FAIL; }

	err = nvs_flash_init();
	if (err) { return err; }

	err = esp_event_loop_create_default();
	if (err) { return err; }

	matrix_clearScreen();
	matrix_drawStringRGB888(6, 6, "CONNECT\nWIFI", 255, 255, 255);

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_WIFI_SSID,
			.password = CONFIG_WIFI_PASSWORD,
			.threshold = {
				.authmode = WIFI_AUTH_WPA2_PSK,
			},
			// .channel = 11,
		},
	};

	err = wifi_begin(&wifi_config, &app->wifi_event_group);
	if (err) { return err; }

	err = esp_wifi_set_ps(WIFI_PS_NONE);
	if (err) { return err; }

	xEventGroupWaitBits(app->wifi_event_group, WIFI_HAS_IP_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	matrix_clearScreen();
	matrix_drawStringRGB888(6, 6, "CONNECT\nMQTT", 255, 255, 255);

	esp_mqtt_client_config_t mqtt_config = {
		.broker = {
			.address = {
				.hostname = "192.168.101.114",
				.transport = MQTT_TRANSPORT_OVER_TCP,
				.port = 1883,
			}
		},
		.credentials = {
			.client_id = CONFIG_LWIP_LOCAL_HOSTNAME,
		},
		.session = {
			.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
		},
		.network = {
			.reconnect_timeout_ms = 1000,
			.timeout_ms = 10000,
		},
	};
	err = mqtt_begin(&mqtt_config, mqtt_joystick_event_handler, app, &app->mqtt_client);
	if (app->mqtt_client == NULL) { return ESP_FAIL; }

	xEventGroupWaitBits(app->mqtt_event_group, MQTT_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	matrix_clearScreen();
	return ESP_OK;
}

void render(struct Framebuffer* fb, struct Game* game, int i) {
	struct RGBColor player_color = power_hsv(i);
	framebuffer_draw_sprite(fb, &SHIP_SPRITE, (i / 6) % SHIP_SPRITE.frames, &game->player.pos, &player_color);

	if (game->bullet.pos.x != 0) {
		framebuffer_draw_sprite(fb, &LASER_SPRITE, 0, &game->bullet.pos, &RGBCOLOR_WHITE);
	}

	for (int e = 0; e < GAME_MAX_ENEMIES; e++) {
		struct Entity* enemy = &game->enemies[e];
		if (enemy->hp > 0 && enemy->sprite != NULL) {
			framebuffer_draw_sprite(fb, enemy->sprite, (i / 6) % enemy->sprite->frames, &enemy->pos, &enemy->color);
		}
	}
}

void spawn_enemies(struct Game *game) {
	int enemy_i = 0;

	for (int i = 0; i < 3; i++) {
		struct Position pos = { .x = 10, .y = 3 + i * (2 + ALIEN4_SPRITE.width) };
		struct RGBColor color = { .r = 200, .g = 0, .b = 200 };
		game_enemy_set(game, pos, color, &ALIEN4_SPRITE, enemy_i++);
	}

	for (int i = 0; i < 2; i++) {
		struct Position pos = { .x = 21, .y = 4 + i * (9 + ALIEN2_SPRITE.width) };
		struct RGBColor color = { .r = 0, .g = 200, .b = 200 };
		game_enemy_set(game, pos, color, &ALIEN2_SPRITE, enemy_i++);
	}

	for (int i = 0; i < 2; i++) {
		struct Position pos = { .x = 33, .y = 6 + i * (3 + ALIEN1_SPRITE.width) };
		struct RGBColor color = { .r = 200, .g = 200, .b = 0 };
		game_enemy_set(game, pos, color, &ALIEN1_SPRITE, enemy_i++);
	}
}

void app_main() {
	struct app_data app;
	esp_err_t err = init(&app);

	if (err != ESP_OK) {
		ESP_LOGE("app", "init failed: %d", err);
		return;
	}

	app.js_reading.x = 2048;
	app.js_reading.y = 2048;
	app.js_reading.btn = 1;

	struct Game game = game_create();
	game.player.pos.x = MATRIX_WIDTH - SHIP_SPRITE.width - 4;
	game.player.pos.y = MATRIX_HEIGHT / 2 - SHIP_SPRITE.height / 2;

	struct Framebuffer fb = framebuffer_init(MATRIX_WIDTH, MATRIX_HEIGHT, FRAMEBUFFER_TYPE);

	struct Button button1 = button_init(BUTTON1_GPIO);
	struct Button button2 = button_init(BUTTON2_GPIO);

	bool next_enemy_move = false;

	spawn_enemies(&game);

	// Game loop
	for (int i = 0; ; i++) {
		EventBits_t bits = xEventGroupGetBits(app.wifi_event_group);
		if ((bits & WIFI_CONNECTED_BIT) == 0) {
			matrix_clearScreen();
			matrix_drawStringRGB888(6, 6, "WIFI DISCONNECTED", 255, 255, 255);
			vTaskDelay(pdMS_TO_TICKS(1000 / FPS));
			continue;
		} else if ((bits & WIFI_HAS_IP_BIT) == 0) {
			matrix_clearScreen();
			matrix_drawStringRGB888(6, 6, "WIFI NO IP", 255, 255, 255);
			vTaskDelay(pdMS_TO_TICKS(1000 / FPS));
			continue;
		}
		bits = xEventGroupGetBits(app.mqtt_event_group);
		if ((bits & MQTT_CONNECTED_BIT) == 0) {
			matrix_clearScreen();
			matrix_drawStringRGB888(6, 6, "MQTT DISCONNECTED", 255, 255, 255);
			vTaskDelay(pdMS_TO_TICKS(1000 / FPS));
			continue;
		}

		// Update input
		xQueueReceive(app.mqtt_pending_data, &app.js_reading, 0);
		bool input_left = app.js_reading.x > 3000;
		bool input_right = app.js_reading.x < 1000;
		bool input_up = app.js_reading.y < 1000;
		bool input_down = app.js_reading.y > 3000;
		bool fire = app.js_reading.btn == 0;

		bool input1 = button_pressed(&button1);
		bool input2 = button_pressed(&button2);

		if (input1) {
			spawn_enemies(&game);
		}
		if (input2) {
			game.player.pos.x = MATRIX_WIDTH - SHIP_SPRITE.width - 4;
			game.player.pos.y = MATRIX_HEIGHT / 2 - SHIP_SPRITE.height / 2;
			game.bullet.pos.x = 0;
			game.score = 0;
		}

		// Bullet logic
		if (game.bullet.pos.x != 0) { // Existing bullet
			game.bullet.pos.x -= 4;

			for (int enemy_i = 0; enemy_i < GAME_MAX_ENEMIES; enemy_i++) {
				struct Entity* enemy = &game.enemies[enemy_i];
				if (enemy->hp > 0) {
					if (game.bullet.pos.x >= enemy->pos.x
						&& game.bullet.pos.x < enemy->pos.x + enemy->sprite->width
						&& game.bullet.pos.y >= enemy->pos.y
						&& game.bullet.pos.y < enemy->pos.y + enemy->sprite->height
					) {
						game_enemy_damage(&game, enemy_i, game.bullet.hp);
						game.bullet.pos.x = 0;
						game.score += 10;
					}
				}
			}

			if (game.bullet.pos.x > MATRIX_WIDTH) game.bullet.pos.x = 0;

		} else if (fire) {
			struct Position pos = { .x = game.player.pos.x - LASER_SPRITE.width, .y = game.player.pos.y + SHIP_SPRITE.height / 2 };
			game_bullet_fire(&game, pos, 1);
		}

		// Input handling
		if (input_left) game.player.pos.y -= 1;
		if (input_right) game.player.pos.y += 1;
		if (input_up) game.player.pos.x -= 1;
		if (input_down) game.player.pos.x += 1;

		// Enemy anim
		if (i % 6 == 0) {
			for (uint8_t enemy_i = 0; enemy_i < GAME_MAX_ENEMIES; enemy_i++) {
				struct Entity* enemy = &game.enemies[enemy_i];
				if (enemy->hp > 0) {
					if (next_enemy_move) {
						enemy->pos.x++;
					} else {
						enemy->pos.x--;
					}
				}
			}

			next_enemy_move = !next_enemy_move;
		}

		render(&fb, &game, i);

		// Update matrix
		for (int x = 0; x < MATRIX_WIDTH; x++) {
			for (int y = 0; y < MATRIX_HEIGHT; y++) {
				int px = (y * MATRIX_WIDTH + x) * FRAMEBUFFER_TYPE;
				matrix_drawPixelRGB888(x, y, fb.data[px], fb.data[px + 1], fb.data[px + 2]);
			}
		}

		char score_str[10];
		itoa(game.score, score_str, 10);
		matrix_drawStringRGB888_rot90(1, MATRIX_HEIGHT / 2, score_str, 160, 160, 160);

		framebuffer_clear(&fb);

		vTaskDelay(pdMS_TO_TICKS(1000 / FPS));
	}
}
