#pragma once

#include "esp_log.h"
#include "mqtt_init.h"
#include "app_struct.h"

static const char* TAG = "mqtt_joystick";

void mqtt_joystick_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data) {
	ESP_LOGD(TAG, "event | base %s, id %d", base, id);

	esp_mqtt_event_handle_t event = data;
	struct app_data *app = (struct app_data *)arg;

	switch ((esp_mqtt_event_id_t)id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(app->mqtt_event_group, MQTT_CONNECTED_BIT);
			esp_mqtt_client_subscribe_single(app->mqtt_client, "/js", 0);
			break;

		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			xEventGroupClearBits(app->mqtt_event_group, MQTT_CONNECTED_BIT);
			break;

		case MQTT_EVENT_SUBSCRIBED:
			ESP_LOGV(TAG, "MQTT_EVENT_SUBSCRIBED");
			break;

		case MQTT_EVENT_UNSUBSCRIBED:
			ESP_LOGV(TAG, "MQTT_EVENT_UNSUBSCRIBED");
			break;

		case MQTT_EVENT_PUBLISHED:
			ESP_LOGV(TAG, "MQTT_EVENT_PUBLISHED");
			break;

		case MQTT_EVENT_DATA:
			if(event->topic_len == 3 && strncmp(event->topic, "/js", 3) == 0
				&& event->data_len == sizeof(struct joystick_reading))
			{
				xQueueOverwrite(app->mqtt_pending_data, event->data);
			}

			break;

		case MQTT_EVENT_ERROR:
			ESP_LOGE(TAG, "MQTT_EVENT_ERROR, %d", event->error_handle->error_type);
			if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
				if (event->error_handle->esp_tls_last_esp_err) {
					ESP_LOGE(TAG, "reported from esp-tls: %d", event->error_handle->esp_tls_last_esp_err);
				}
				if (event->error_handle->esp_tls_stack_err) {
					ESP_LOGE(TAG, "reported from tls stack: %d", event->error_handle->esp_tls_stack_err);
				}
				if (event->error_handle->esp_transport_sock_errno) {
					ESP_LOGE(TAG, "reported from transport's socket errno: %d", event->error_handle->esp_transport_sock_errno);
				}
			}

			xEventGroupSetBits(app->mqtt_event_group, MQTT_FAILED_BIT);
			break;

		default: break;
	}
}
