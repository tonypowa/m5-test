// Set your local WiFi username and password. Please use a 2.4GHz access point
#define WIFI_SSID     "MOVISTAR_8930"
#define WIFI_PASSWORD "xxxxx"

// Put you name in below, and the location where you will place the sensor
#define YOUR_NAME "tony"

// Comma-separated Prometheus labels (key=value) to apply to the metrics from the sensor
// Example: site=home,location=study
//
// Adding labels might require you to increase the size of the WriteRequest buffer in M5StickCPlus.ino
#define PROM_LABELS "site=home,location=study"

// For more information on where to get these values see: https://github.com/grafana/diy-iot/blob/main/README.md#sending-metrics
#define GC_URL "prometheus-prod-xxx-prod-us-central-x.grafana.net"
#define GC_PATH "/api/prom/push"
#define GC_PORT 443
#define GC_USER "6xxx3"
#define GC_PASS "glc_xxx="

// Set to 1 to show debug information on the LCD screen, or 0 to not display
// The debug information will show if you are able to connect to the WiFi or not, and whether writing the metrics to Prometheus was successful or not
#define LCD_SHOW_DEBUG_INFO "1"