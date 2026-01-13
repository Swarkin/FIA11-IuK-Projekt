idf_component_register(
	SRCS "main.c" "mqtt_event_handler.c"
	INCLUDE_DIRS "."
	REQUIRES nvs_flash wifi_init mqtt_init joystick
)
