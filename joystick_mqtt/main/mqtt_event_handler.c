#include "mqtt_event_handler.h"

static const char *TAG = "mqtt_event_handler";

void mqtt_joystick_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data) {
	ESP_LOGI(TAG, "base %s, id %d", base, id);

	esp_mqtt_event_handle_t event = data;
	EventGroupHandle_t mqtt_event_group = *(EventGroupHandle_t*)arg;

	switch ((esp_mqtt_event_id_t)id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
			break;

		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
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
			ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			// printf("%.*s | ", event->topic_len, event->topic);
			// printf("%.*s\n", event->data_len, event->data);
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

			xEventGroupSetBits(mqtt_event_group, MQTT_FAILED_BIT);
			break;

		default: break;
	}
}
