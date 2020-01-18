// Wifi username and password
const char *WIFI_SSID = "your ssid";
const char *WIFI_PASSWORD = "your password";

// MQTT connection details
const char *MQTT_SERVER = "x.x.x.x";
const char *MQTT_USER = "username"; // NULL for no authentication
const char *MQTT_PASSWORD = "password"; // NULL for no authentication

#define MQTT_TOPIC_TEMPERATURE "home/unique_sensor_id/temperature"
#define MQTT_TOPIC_STATE "home/unique_sensor_id/status"
#define MQTT_PUBLISH_DELAY 15000
#define MQTT_CLIENT_ID "unique_sensor_id"
