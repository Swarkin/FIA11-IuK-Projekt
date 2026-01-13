#pragma once

#include "freertos/FreeRTOS.h"
#include "mqtt_client.h"
#include "joystick.h"

struct app_data {
	EventGroupHandle_t wifi_event_group;
	EventGroupHandle_t mqtt_event_group;
	QueueHandle_t mqtt_pending_data;
	esp_mqtt_client_handle_t mqtt_client;
	struct joystick_reading js_reading;
};
