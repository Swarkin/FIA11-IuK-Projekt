idf_component_register(
	SRCS "main.c" "graphics/framebuffer.c" "game/game.c" "input/button.c"
	INCLUDE_DIRS "."
	REQUIRES matrix-wrapper esp_driver_gpio nvs_flash wifi_init mqtt_init joystick
)
