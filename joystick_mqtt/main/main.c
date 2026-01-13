#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "wifi_init.h"
#include "mqtt_init.h"
#include "driver/gpio.h"

#include "mqtt_event_handler.h"
#include "joystick.h"

static const char *TAG = "app";

#define LED_PIN GPIO_NUM_2

struct app_data {
	EventGroupHandle_t wifi_event_group;
	EventGroupHandle_t mqtt_event_group;
	esp_mqtt_client_handle_t mqtt_client;
	QueueHandle_t mqtt_pending_data;
	struct joystick js;
};

esp_err_t init(struct app_data *app) {
	esp_err_t err;

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
	err = mqtt_begin(&mqtt_config, mqtt_joystick_event_handler, &app->mqtt_event_group, &app->mqtt_client);
	if (app->mqtt_client == NULL) { return ESP_FAIL; }

	xEventGroupWaitBits(app->mqtt_event_group, MQTT_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	struct joystick_config js_config = {
		.pin_btn = GPIO_NUM_25,
		.pin_x = GPIO_NUM_33,
		.pin_y = GPIO_NUM_32,
		.adc_unit = ADC_UNIT_1,
		.adc_channel_x = ADC_CHANNEL_5,
		.adc_channel_y = ADC_CHANNEL_4,
	};
	app->js = joystick_init(js_config);

	// Configure LED pin
	gpio_config_t led_config = {
		.pin_bit_mask = (1ULL << LED_PIN),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
	};
	err = gpio_config(&led_config);
	if (err) { return err; }
	err = gpio_set_level(LED_PIN, 0);
	if (err) { return err; }

	return ESP_OK;
}

void app_main() {
	struct app_data app;
	ESP_ERROR_CHECK( init(&app) );
	ESP_LOGI(TAG, "Initialized");

	while (1) {
		struct joystick_reading reading;
		joystick_read(&app.js, &reading);

		// Control LED based on button press
		gpio_set_level(LED_PIN, reading.btn ? 0 : 1);

		esp_mqtt_client_publish(app.mqtt_client, "/js", (const char*)&reading, sizeof(reading), 0, 0);
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}
