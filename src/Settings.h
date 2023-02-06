// ===============================================================================
// Update/Add any #define values to match your build and board type if not using D1 Mini
// ================================================================================
// Change these if you use different pins.
#define RX_PIN 16                               // Serial2 RX to Amp TX
#define TX_PIN 17                               // Serial2 TX to Amp RX
#define SDA_PIN 21                              // Data pin for SSD1306 display
#define SCL_PIN 22                              // Clock pin for SSD1306 display
#define DISP_PIN 19                             // Rotary knob click for controlling display
#define IR_RECV_PIN 18                          // IR Receiver Pin

#define WIFIMODE 2                              // 0 = Only Soft Access Point, 1 = Only connect to local WiFi network with UN/PW, 2 = Both
#define MQTTMODE 1                              // 0 = Disable MQTT (not recommended), 1 = Enable (will only be enabled if WiFi mode = 1 or 2 - broker must be on same network)

// Change to desired MQTT topics
#define MQTTCLIENT "DeskAmp"                    // MQTT Client Name
#define WIFIHOSTNAME "DeskAmp"                  // Wifi Host Name
#define MQTT_TOPIC_SUB "cmnd/deskamp"           // MQTT subscribe topic (should be separate from pub - these values are NOT retained)
#define MQTT_TOPIC_PUB "stat/deskamp"           // MQTT publish topic (should be separate from sub - these values ARE retained)
#define OTA_HOSTNAME "DeskAmpOTA"               // Hostname to broadcast as port in the IDE of OTA Updates

//Display Settings
#define SCREEN_WIDTH 128                        // OLED display width, in pixels
#define SCREEN_HEIGHT 64                        // OLED display height, in pixels
#define SCREEN_ADDR 0x3C                        //I2C 7-bit Address 
#define OLED_RESET     -1                       // Reset pin # (or -1 if display does not have reset pin)

// OTA Updates - do not change these unless needed and you are sure you know what you are doing
bool ota_flag = true;                           // Must leave this as true for board to broadcast port to IDE upon boot
uint16_t ota_time_elapsed = 0;                  // Counter when OTA active
uint16_t ota_boot_time_window = 2500;           // minimum time on boot for IP address to show in IDE ports, in millisecs
uint16_t ota_time_window = 20000;               // time to start file upload when ota_flag set to true (after initial boot), in millsecs

// ---------------------------------------------------------------------------------------------------------------
// Options - Defaults upon boot-up or any other custom ssttings
// ---------------------------------------------------------------------------------------------------------------
String mqttTopicPub = "stat/deskamp";

// Custom settings applied/reapplied at boot
unsigned long dispTempDisplay_duration = 4000;  // How long to display changed value on display before returning to selected mode (in milliseconds)
byte bootDispMode = 1;                          // Default display mode at boot (1=Source, 2=Vol, 3=Title, 4=Track, 5=Mute, 6=blank)
bool enableIR = 1;                              // 0 = disabled. Can toggled off/on via MQTT after boot.  This is just default boot setting
