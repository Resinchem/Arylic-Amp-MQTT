/*
 * Interface to Arylic Amp v4
 * Version: 0.28 - Add disable IR settings option to disable IR command processing due to rogue signals
 *               - Also added this as an MQTT command so it can be toggled from front end.
 * Last Update: 1/29/2023
 */
#include <WiFi.h>               
#include <ESPmDNS.h>              // https://github.com/espressif/arduino-esp32
#include <WebServer.h>           
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <FS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>         // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>     // https://github.com/adafruit/Adafruit_SSD1306
#include <IRremote.h>             // https://github.com/Arduino-IRremote/Arduino-IRremote
#include "Credentials.h"          // File must exist in same folder as .ino.  Edit as needed for project
#include "Settings.h"             // File must exist in same folder as .ino.  Edit as needed for project

//Display modes
#define SOURCE 1
#define VOLUME 2
#define TITLE 3
#define TRACK 4
#define MUTE 5
#define BLANK 6                  //BLANK should always be last in list so that button click 'wraps' from last item to first

//IR Commands 
struct IRCommands {
  uint16_t irCode;
  String uartCommand;
};

/* UPDATE COMMAND FOR YOUR REMOTE HERE:
   first value is the code returned, second value is the UART command to issue
   UART commands that only accept a numeric value (e.g treble, bass and balance), use ++ for increase and -- for decrease and 
   value will be increased or decreased by 1 within the permissible min/max values.
   Other commands may be added... just assure the UART command is valid.
*/
//==============================
IRCommands remoteCommands[] = {
  {16, "VOL:-;"},
  {17, "TRE:--;"},
  {18, "BAS:--;"},
  {19, "BAL:--;"},
  {20, "VOL:+;"},
  {21, "TRE:++;"},
  {22, "BAS:++;"},
  {23, "BAL:++;"},
  {65, "POP;"},  //toggles pause/play
  {64, "SYS:STANDBY;"},
  {68, "SRC:NET;"},
  {69, "SRC:BT;"},
  {88, "SRC:USB;"},
  {89, "SRC:USBDAC;"},
  {92, "MUT:T;"},  //toggles mute
  {93, "DISPMODE"}  //This is not a UART command, but local to change display home mode
};
//=============================

//GLOBAL VARIABLES
bool mqttConnected = false;       //Will be enabled if defined and successful connnection made.  This var should be checked upon any MQTT actin.
long lastReconnectAttempt = 0;    //If MQTT connected lost, attempt reconnenct
uint16_t ota_time = ota_boot_time_window;
bool initBoot = true;            // Used to hide OTA Update on display on initial boot
unsigned long currentMillis = 0;
unsigned long standbyTimer = 0;

//DISPLAY RELATED GLOBALS
byte dispMode = bootDispMode;    
byte prevdispMode = bootDispMode;
bool dispModeTempSource = true;
bool standbyMode = false;
unsigned long dispModeTemp_timer = 0;
String dispSource = "NET";
byte dispVolume = 50;
String dispMute = "OFF";
//Needed for IR remote command processing
int dispBass = 0;
int dispMidrange = 0;
int dispTreble = 0;
int dispBalance = 0;
String dispTitle = "N/A";
String dispTrack = "N/A";
String dispArtist = "N/A";
String dispAlbum = "N/A";

//BUTTON DEBOUNCE (Rotary knob click)
int dispBtnState;
int lastBtnState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;


//Setup Local Access point if enabled via WIFI Mode
#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2)
  const char* APssid = AP_SSID;        
  const char* APpassword = AP_PWD;  
#endif
//Setup Wifi if enabled via WIFI Mode
#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2)
  #include "Credentials.h"                    // Edit this file in the same directory as the .ino file and add your own credentials
  const char *ssid = SID;
  const char *password = PW;
#endif
//Setup MQTT if enabled - only available when WiFi is also enabled
#if (WIFIMODE == 1 || WIFIMODE == 2) && (MQTTMODE == 1)    // MQTT only available when on local wifi
  const char *mqttUser = MQTTUSERNAME;
  const char *mqttPW = MQTTPWD;
  const char *mqttClient = MQTTCLIENT;
  const char *mqttTopicSub = MQTT_TOPIC_SUB;
  //const char *mqttTopicPub = MQTT_TOPIC_PUB;
  //String strTopicPub = String(*mqttTopicPub);
#endif

WiFiClient espClient;
PubSubClient client(espClient);    
WebServer server;
//Define Display;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//Setup IR Receiver
IRrecv irrecv(IR_RECV_PIN);
decode_results irresults;
  
// ============================================
//   MAIN SETUP
// ============================================
void setup() {
  // Serial monitor
  Serial.begin(115200);
  Serial.println("Booting...");
  //Serial communication to amp
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial2.setTimeout(1000);

  //----------------
  //WiFi Connection
  //----------------
  WiFi.setSleep(false);  //Disable WiFi Sleep
  delay(200);
  // WiFi - AP Mode or both
#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2) 
  WiFi.hostname(WIFIHOSTNAME);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(APssid, APpassword);    // IP is usually 192.168.4.1
#endif
  // WiFi - Local network Mode or both
#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2) 
  byte count = 0;
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // Stop if cannot connect
    if (count >= 60) {
      // Could not connect to local WiFi 
      Serial.println();
      Serial.println("Could not connect to WiFi.");     
      return;
    }
    delay(500);
    count++;
  }
  Serial.println();
  Serial.println("Successfully connected to Wifi");
  IPAddress ip = WiFi.localIP();
#endif   
  //-----------------------------------------
  // Setup MQTT - only if enabled and on WiFi  
  //-----------------------------------------
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
  byte mcount = 0;
  //char topicPub[32];
  client.setServer(MQTTSERVER, MQTTPORT);
  client.setCallback(callback);
  Serial.print("Connecting to MQTT broker.");
  while (!client.connected( )) {
    Serial.print(".");
    client.connect(mqttClient, mqttUser, mqttPW);
    if (mcount >= 60) {
      Serial.println();
      Serial.println("Could not connect to MQTT broker. MQTT disabled.");
      // Could not connect to MQTT broker
      return;
    }
    delay(500);
    mcount++;
  }
  mqttConnected = true;
  Serial.println();
  Serial.println("Successfully connected to MQTT broker.");
  client.subscribe(MQTT_TOPIC_SUB"/#");
  String outTopic = mqttTopicPub + "/mqtt";
  client.publish(outTopic.c_str(), "connected", true);
  //v0.28 - output initial IRMode status from settings since this isn't a UART command
  outTopic = mqttTopicPub + "/irmode";
  if (enableIR) {
    client.publish(outTopic.c_str(), "1");
  } else {
    client.publish(outTopic.c_str(), "0");
  }
#endif
  //-----------------------------
  // Setup OTA Updates
  //-----------------------------
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
  });
  ArduinoOTA.begin();
  // Setup handlers for web calls for OTAUpdate and Restart
  server.on("/restart",[](){
    server.send(200, "text/html", "<h1>Restarting...</h1>");
    delay(1000);
    ESP.restart();
  });
  server.on("/otaupdate",[]() {
    server.send(200, "text/html", "<h1>Ready for upload...<h1><h3>Start upload from IDE now</h3>");
    ota_flag = true;
    ota_time = ota_time_window;
    ota_time_elapsed = 0;
  });
  server.begin();

// ------------------------------------------------------
// Initialize Display and show splash screen for 2 seconds
// ------------------------------------------------------
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR);
  display.display();
  delay(2000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  // Set button (rotary know) for switching display mode
  pinMode(DISP_PIN, INPUT_PULLUP);
  // Set button for wake (wake from Standby - simulate button press)
  //pinMode(WAKE_PIN, OUTPUT);
  //digitalWrite(WAKE_PIN, LOW);  //Set to low
// Initialize IR Receiver
  irrecv.enableIRIn();  
  
  Serial.println("Staring main loop...");
  //Force status update
  Serial2.flush();
  Serial2.write("STA;");

 }

// =============================================================
// *************** MQTT Message Processing *********************
// =============================================================
void callback(char* topic, byte* payload, unsigned int length) {
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))

  payload[length] = '\0';
  String message = (char*)payload;
  String uartMsg;
  bool hasPayload = false;
  /*
   * Add any commands submitted here
   * Example:
   * if (strcmp(topic, MQTT_TOPIC_SUB"/mode")==0) {
   *   MyVal = message;
   *   Do something with MyVal
   *   return;
   * };
   */
  message.trim();
  if (message.length() > 0) {
    hasPayload = true;
  }
  //Amp control and commands (UART) 
  // These commands should be universal, regardless of current source
  if (strcmp(topic, MQTT_TOPIC_SUB"/source") == 0) {
    if (hasPayload) {
      uartMsg = "SRC:" + message + ";";
    } else {
      uartMsg = "SRC;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/system") == 0) {
    //Use these with caution!  Reset and recover could require reinitialization of your amp
    uartMsg = "SYS:" + message + ";";
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/wifireset") == 0) {
    Serial2.flush();
    Serial2.write("WRS;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/volume") == 0) {
    if (hasPayload) {
      uartMsg = "VOL:" + message + ";";
    } else {
      uartMsg = "VOL;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/mute")==0) {
    if (hasPayload) {
      uartMsg = "MUT:" + message + ";";
    } else {
      uartMsg = "MUT;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/bass") == 0) {
    if (hasPayload) {
      uartMsg = "BAS:" + message + ";";
    } else {
      uartMsg = "BAS;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/midrange") == 0) {
    if (hasPayload) {
      uartMsg = "MID:" + message + ";";
    } else {
      uartMsg = "MID:";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/treble") == 0) {
    if (hasPayload) {
      uartMsg = "TRE:" + message + ";";
    } else {
      uartMsg = "TRE;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/led") == 0) {
    if (hasPayload) {
      uartMsg = "LED:" + message + ";";
    } else {
      uartMsg = "LED;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/beep") == 0) {
    if (hasPayload) {
      uartMsg = "BEP:" + message + ";";
    } else {
      uartMsg = "BEP;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/voice") == 0) {
    if (hasPayload) {
      uartMsg = "PMT:" + message + ";";
    } else {
      uartMsg = "PMT;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/virtbass") == 0) {
    if (hasPayload) {
      uartMsg = "VBS:" + message + ";";
    } else {
      uartMsg = "VBS;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/pregain") == 0) {
    if (hasPayload) {
      uartMsg = "PRG:" + message + ";";
    } else {
      uartMsg = "PRG;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/balance") == 0) {
    if (hasPayload) {
      uartMsg = "BAL:" + message + ";";
    } else {
      uartMsg = "BAL;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/maxvol") == 0) {
    if (hasPayload) {
      uartMsg = "MXV:" + message + ";";
    } else {
      uartMsg = "MXV;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/poweronsource") == 0) {
    if (hasPayload) {
      uartMsg = "POM:" + message + ";";
    } else {
      uartMsg = "POM;";
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/preset") == 0) { 
    if (hasPayload) {
      uartMsg = "PST:" + message + ";";
    } else {
      uartMsg = "PST;";
    }
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/delay") == 0) {
    if (hasPayload) {
      uartMsg = "DLY:" + message + ";";
    } else {
      uartMsg = "DLY;";
    }
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/autoswitch") == 0) {
    if (hasPayload) {
      uartMsg = "ASW:" + message + ";";
    } else {
      uartMsg = "ASW;";
    }
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/syncvol") == 0) {
    if (hasPayload) {
      uartMsg = "VOS:" + message + ";";
    } else {
      uartMsg = "VOS;";
    }
    
  // These commands only work with certain sources (see documentation)
  } else if ((strcmp(topic, MQTT_TOPIC_SUB"/pause") == 0) || (strcmp(topic, MQTT_TOPIC_SUB"/play") == 0)) {
    Serial2.flush();
    Serial2.write("POP;");
    showTextParam("Command", "Pause/Play", 2);
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/stop") == 0) {
    Serial2.flush();
    Serial2.write("STP;");
    showTextParam("Command", "STOP", 3);
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/next") == 0) {
    Serial2.flush();
    Serial2.write("NXT;");
    showTextParam("Command", "NEXT", 3);
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/previous") == 0) {
    Serial2.flush();
    Serial2.write("PRE;");
    showTextParam("Command", "PREV", 3);
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/bluetooth") == 0) {
    if (hasPayload) {
      uartMsg = "BTC:" + message + ";";
    } else {
      uartMsg = "BTC;";  
    }
    Serial2.flush();
    Serial2.write(uartMsg.c_str());
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/loopmode") == 0) {
    uartMsg = "LPM:" + message + ";";
    Serial2.flush();
    Serial2.write(uartMsg.c_str());


  //Amp Status updates requests - these commands only request a state update, which will be published under related /stat topics
  //Any payloads are ignored
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/status") == 0) {
    Serial2.flush();
    Serial2.write("STA;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/version") == 0) {
    Serial2.flush();
    Serial2.write("VER;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/ethernet") == 0) {
    Serial2.flush();
    Serial2.write("ETH;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/wifi") == 0) {
    Serial2.flush();
    Serial2.write("WIF;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/internet") == 0) {
    Serial2.flush();
    Serial2.write("WWW;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/elapsed") == 0) {  //force elspsed time update (elapsed/total in msec)
    Serial2.flush();
    Serial2.write("ELP;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/playing") == 0) {
    Serial2.flush();
    Serial2.write("PLA;"); 
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/channel") == 0) {
    Serial2.flush();
    Serial2.write("CHN;");
  } else if (strcmp(topic,MQTT_TOPIC_SUB"/multiroom") == 0) {
    Serial2.flush();
    Serial2.write("MRM;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/audio") == 0) {
    Serial2.flush();
    Serial2.write("AUD;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/fixedvol") == 0) {
    Serial2.flush();
    Serial2.write("VOF;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/tracknum") == 0) {
    Serial2.flush();
    Serial2.write("PLI;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/devicename") == 0) {
    Serial2.flush();
    Serial2.write("NAM;");
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/network") == 0) {
    Serial2.flush();
    Serial2.write("NET;");
  //This final command just passed the payload directly as a UART message - payload must formatted properly, including trailing semi-colon
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/uart") == 0) {
    uartMsg = message;
    Serial2.flush();
    Serial2.write(uartMsg.c_str());

  //These are not UART commands, but other commands to enable/disable certain feature that are firmware only
  } else if (strcmp(topic, MQTT_TOPIC_SUB"/irmode") == 0) {
    if (hasPayload) {
      setIRMode(message.toInt());
    } else {
      setIRMode(99);  //just update status
    }
  }
#endif
};
// Reconnect to broker if connection lost (usually a result of a Home Assistant/broker/server reboot)
boolean reconnect() {
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
  if (client.connect(mqttClient, mqttUser, mqttPW)) {
    // Once connected, publish an announcement...
    String outTopic = mqttTopicPub + "/mqtt";
    client.publish(outTopic.c_str(), "connected", true);
    // ... and resubscribe
    client.subscribe(MQTT_TOPIC_SUB"/#");
  }
  return client.connected();
#endif
}

// ===============================================================
//   MAIN LOOP
// ===============================================================
void loop() {
  //Handle OTA updates when OTA flag set via HTML call to http://ip_address/otaupdate

  if (ota_flag) {
    if (initBoot) {
      initBoot = false;
    } else {
      showOTAUpdate();
    }
    uint16_t ota_time_start = millis();
    while (ota_time_elapsed < ota_time) {
      ArduinoOTA.handle();  
      ota_time_elapsed = millis()-ota_time_start;   
      delay(10); 
    }
    ota_flag = false;
  }
  //Handle any web calls
  server.handleClient();

  // Reconnect to MQTT if enabled and not connected
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
    if (!client.connected()) {
      long now = millis();
      if (now - lastReconnectAttempt > 60000) {   //attempt reconnect once per minute. Drop usually a result of Home Assistant/broker server restart
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (reconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      // Client connected
      client.loop();
    }
#endif
  // Process IR if enabled
  if (enableIR) {
  //Look for IR Code Received if enabled
    if (irrecv.decode()) {
    
      uint16_t ir_code = irrecv.decodedIRData.command;
      processIRCommand(ir_code);
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
    client.publish(MQTT_TOPIC_PUB"/ircode", String(ir_code).c_str(), true);
#endif
      delay(250);

      irrecv.resume();
    }
  }
  //Rotary Knob Click Process
  currentMillis = millis();
  int curBtnState = digitalRead(DISP_PIN);
  //Debounce
  if (curBtnState != lastBtnState) {
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (curBtnState != dispBtnState) {
      dispBtnState = curBtnState;
    
      if (dispBtnState == LOW) {
        if (dispMode == BLANK) {
          dispMode = 1;
        } else {
         dispMode ++;
        }
        prevdispMode = dispMode;
      }  
    }
  }
  lastBtnState = curBtnState;
  
  if (standbyMode && (currentMillis < standbyTimer)) {  //Don't process incoming data for 5 seconds after putting in standby (keeps display off)
    if (Serial2.available() > 0) {
      String ignoreData = Serial2.readStringUntil('\n');
    }
  } else {
    if (Serial2.available() > 0) {
      standbyMode = false;
      String ampData = Serial2.readStringUntil('\n');
      ampData.trim();
      //Handle power down of display when amp reports SYS:STANDBY
      if (ampData.indexOf("STANDBY") > 0) {
        while (Serial2.available() > 0) {
          ampData  = Serial2.readStringUntil('\n');
          delay(500);
        }
        display.clearDisplay();
        display.display(); 
        standbyMode = true;
        standbyTimer = currentMillis + 5000;
        //Publish message with source of STANDBY
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
        client.publish(MQTT_TOPIC_PUB"/source", "STANDBY", true);
#endif
      } else { 
        standbyMode = false;
        updateMQTT(ampData);
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
        client.publish(MQTT_TOPIC_PUB"/uart", ampData.c_str(), true);
#endif
      }
    }
  }
  //Update display
  if (!standbyMode) {
    if (currentMillis > (dispModeTemp_timer + dispTempDisplay_duration)) {
      switch (dispMode) {
        case SOURCE:
          showSource();
          break;
        case VOLUME:
          //showVolume();
          showNumberParam("Volume", dispVolume);
          break;
        case MUTE:
          showMute();
           break;
        case TITLE:
          //showTitle();
          showTextParam("Title", dispTitle, 2);
          break;
        case TRACK:
          //showTrack();
          showTextParam("Track Num", dispTrack, 3);
          break;
        default:
          display.clearDisplay();
          display.display(); 
          break;
      }
      dispModeTemp_timer = 0;
      dispModeTempSource = false;
    }
  }  
}

// ====================================
//  Misc Functions
// ====================================
// This is needed to UART Hex-returned values (NAM, TIT, ART, ALB)
String hexToAscii( String hex )
{
  uint16_t len = hex.length();
  String ascii = "";

  for ( uint16_t i = 0; i < len; i += 2 )
    ascii += (char)strtol( hex.substring( i, i+2 ).c_str(), NULL, 16 );
    
  return ascii;
}

// --------------------------------
//  Handle enable/disable custom IR
//    (since this is not a UART command)
//  Updates both MQTT and display
// ---------------------------------
void setIRMode(byte statcode) {
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
  String topic = mqttTopicPub + "/irmode";
  String payload = "0";
  //If stat code > 0, enable IR otherwise disable
  if (statcode > 0) {
    enableIR = 1;
    payload = "1";
  } else if (statcode == 0) {
    enableIR = 0;
    payload = "0";    
    //Clear last retained IR code (if any)
    client.publish((mqttTopicPub + "/ircode").c_str(), "0", true);
  }
  client.publish(topic.c_str(), payload.c_str(), true);
  showBooleanParam("IR Recvr", enableIR);

#endif
}

// -------------------------
// Process IR codes received
// -------------------------
void processIRCommand(uint16_t ircode) {
  for (int i=0; i<sizeof remoteCommands/sizeof remoteCommands[0]; i++) {

    if (remoteCommands[i].irCode == ircode) {
      String tempCmd = remoteCommands[i].uartCommand;

      //Must handle commands that expect a numeric value and won't accept "+" or "-".. and validate range
      //Treble
      if (tempCmd.startsWith("TRE:--")) {
        if (dispTreble > -10) {
          tempCmd.replace("--", String(dispTreble - 1));
        } else {
          tempCmd = "TRE:-10;";
        }
      } else if (tempCmd.startsWith("TRE:++")) {
        if (dispTreble < 10) {
           tempCmd.replace("++", String(dispTreble + 1));   
        } else {
          tempCmd = "TRE:10;";
        }
     //Bass
     } else if (tempCmd.startsWith("BAS:--")) {
        if (dispBass > -10) {
          tempCmd.replace("--", String(dispBass - 1));
        } else {
          tempCmd = "BAS:-10;";
        }
     } else if (tempCmd.startsWith("BAS:++")) {
        if (dispBass <  10) {
          tempCmd.replace("++", String(dispBass + 1));
        }
     //Balance (-100 to 100)   
     } else if (tempCmd.startsWith("BAL:--")) {
       if (dispBalance > -100 ) {
        tempCmd.replace("--", String(dispBalance - 1));
       } else {
        tempCmd = "BAL:-100;";
       }
     } else if (tempCmd.startsWith("BAL:++")) {
       if (dispBalance < 100) {
         tempCmd.replace("++", String(dispBalance + 1));
       } else {
        tempCmd = "BAL:100";
       }
     }

     if (tempCmd == "DISPMODE") {
       //this is a local command, not a UART command. Increase display mode by 1
        if (dispMode == BLANK) {
          dispMode = 1;
        } else {
          dispMode ++;
        }
        prevdispMode = dispMode;
       
       break; 
     } else {
        Serial2.flush();
        //Serial2.write((remoteCommands[i].uartCommand).c_str());
        Serial2.write(tempCmd.c_str());
        //Update display for those commands that do not return a UART value
        if (tempCmd == "POP;") {
          showTextParam("Command", "Pause/Play", 2);
        } else if (tempCmd == "STP;") {
          showTextParam("Command", "STOP", 3);
        } else if (tempCmd == "NXT;") {
          showTextParam("Command", "NEXT", 3);
        } else if (tempCmd == "PRE;") {
          showTextParam("Command", "PREV", 3);
        }
        break; 
     }
    }
  }
}

// ====================================
//  MQTT Message Processing
// ===================================
void updateMQTT(String uartData) {
  //Incoming data may be a single value or a list of semi-colon delimited values
  //All data returned consists of a three character topic, followed by a colon and the value
  //This proc also calls the process to update the display, so it should remain enabled even if MQTT disabled
  int dataLen = uartData.length();
  int intSemiPos = uartData.indexOf(';');
  int intLastPos = 0;
  String strTopic;
  String strMessage;
  String uartTopic;
  if (dataLen > 0) {
    uartTopic = uartData.substring(0,3);
    if (uartTopic == "STA") {
      //Must handle differently to split multiple values
      updateAmpStatus(uartData);
    } else {  
      while (intSemiPos > 0) {
        if ((uartTopic == "NAM")) {  //hexed value - must convert to ASCII
          strMessage = hexToAscii(uartData.substring(4, intSemiPos));
         } else if ((uartTopic == "CHN") || (uartTopic == "MRM")) {
          strMessage = decodeUartValue(uartTopic, uartData.substring(4, intSemiPos));
        } else {
          strMessage = uartData.substring(4, intSemiPos);           //UART value/response 
        }
        strTopic = mqttTopicPub + "/" + getFriendlyTopic(uartTopic, strMessage);  //Get friendly name from UART message (topic)
        
       // strMessage = uartData.substring(4, intSemiPos);           //UART value/response
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))
        client.publish(strTopic.c_str(), strMessage.c_str(), true);
#endif
        intLastPos = intSemiPos + 1;
        intSemiPos = uartData.indexOf(';', intLastPos);
      }  
    }
  }
}

String getFriendlyTopic(String uartTopic, String uartValue) {
  //Gets friendly/topic name for MQTT Publish
  //Also updates display where applicable
  String outVal="unknown";
  String dispVal = "?";
  if (uartTopic == "VER") {
    outVal = "version";
    showTextParam("Version", uartValue, 2);
  } else if (uartTopic == "WWW") {
    outVal = "internet";
    //dispWWWconn = uartValue.toInt();
    showBooleanParam("Internet", uartValue.toInt());
  } else if (uartTopic == "AUD") {
    outVal = "audio";
    //dispAudio = uartValue.toInt();
    showBooleanParam("Audio", uartValue.toInt());
  } else if (uartTopic == "SRC") {
    outVal = "source";
    dispSource = uartValue;
    showSource();
  } else if (uartTopic == "VOL") {
    outVal = "volume";
    dispVolume = uartValue.toInt();
    showNumberParam("Volume", dispVolume);
    //showVolume();
  } else if (uartTopic == "MUT") {
    outVal = "mute";
    if (uartValue == "1") {
      dispMute = "ON";
      dispMode = MUTE;
    } else {
      dispMute = "OFF";
      dispMode = prevdispMode;
    }
    showMute();
  } else if (uartTopic == "BAS") {
    outVal = "bass";
    dispBass = uartValue.toInt();
    showNumberParam("Bass", dispBass);
  } else if (uartTopic == "MID") {
    outVal = "midrange";
    dispMidrange = uartValue.toInt();
    showNumberParam("Midrange", dispMidrange);
  } else if (uartTopic == "TRE") {
    outVal = "treble";
    dispTreble = uartValue.toInt();
    showNumberParam("Treble", dispTreble);
  } else if (uartTopic == "BTC") {
    outVal = "btconnect";
    //dispBTconn = uartValue.toInt();
    showBooleanParam("Bluetooth", uartValue.toInt());
  } else if (uartTopic == "PLA") {
    outVal = "playing";
    showBooleanParam("Playing", uartValue.toInt());
  } else if (uartTopic == "CHN") {
    outVal = "channel";
    showTextParam("Channel", uartValue, 2);
  } else if (uartTopic == "MRM") {
    outVal = "multiroom";
    showTextParam("Multi-room", uartValue, 2);
  } else if (uartTopic == "LED") {
    outVal = "led";
    //dispLED = uartValue.toInt();
    showBooleanParam("LED", uartValue.toInt());
  } else if (uartTopic == "BEP") {
    outVal = "beep";
    //dispBeep = uartValue.toInt();
    showBooleanParam("Key Beep", uartValue.toInt());
  } else if (uartTopic == "VBS") {
    outVal = "virtbass";
    //dispVirtbass = uartValue.toInt();
    showBooleanParam("Virt. Bass", uartValue.toInt());
  } else if (uartTopic == "LPM") {
    outVal = "loopmode";
    //dispLoopmode = uartValue;
    showTextParam("Loop mode", uartValue, 2);
  } else if (uartTopic == "NET") {
    outVal = "network";
    //dispNetwork = uartValue.toInt();
    showBooleanParam("Network", uartValue.toInt());
  } else if (uartTopic == "ETH") {
    outVal = "ethernet";
    //dispETHconn = uartValue.toInt();
    showBooleanParam("Ethernet", uartValue.toInt());
  } else if (uartTopic == "WIF") {
    outVal = "wifi";
    //dispWIFIconn = uartValue.toInt();
    showBooleanParam("WIFI", uartValue.toInt());
  } else if (uartTopic == "PMT") {
    outVal = "voice";
    //dispVoice = uartValue.toInt();
    showBooleanParam("Voice Pmpt", uartValue.toInt());
  } else if (uartTopic == "PRG") {
    outVal = "pregain";
    //dispPregain = uartValue.toInt();
    showBooleanParam("Pregain", uartValue.toInt());
  } else if (uartTopic == "DLY") {
    outVal = "delay";
    showNumberParam("Delay", uartValue.toInt());
  } else if (uartTopic == "MXV") {
    outVal = "maxvol";
    //dispMaxvol = uartValue.toInt();
    showNumberParam("Max. Vol", uartValue.toInt());
  } else if (uartTopic == "ASW") {
    outVal = "autoswitch";
    showBooleanParam("Auto-switch", uartValue.toInt());
  } else if (uartTopic == "POM") {
    outVal = "poweronsource";
    //dispPOM = uartValue;
    showTextParam("PowerOn Scr", uartValue, 2);
  } else if (uartTopic == "VND") {
    outVal = "vendor";
    showTextParam("Vendor", uartValue, 2);
  } else if (uartTopic == "ELP") {
    outVal = "elasped";
    showTextParam("Elaspsed", uartValue, 2);
  } else if (uartTopic == "VOS") {
    outVal = "syncvol";
    showBooleanParam("Vol Sync", uartValue.toInt());
  } else if (uartTopic == "BAL") {
    outVal = "balance";
    dispBalance = uartValue.toInt();
    showNumberParam("Balance", dispBalance);
    //showBalance();
  } else if (uartTopic == "VOF") {
    outVal = "fixedvol";
    showBooleanParam("Fixed Vol", uartValue.toInt());
  } else if (uartTopic == "PLI") {
    outVal = "tracknum";
    dispTrack = uartValue;
    showTextParam("Track Num", dispTrack, 2);
    //showTrack();
  } else if (uartTopic == "PST") {
    outVal = "preset";
    showNumberParam("Preset", uartValue.toInt()); 

  } else if (uartTopic == "NAM") {
    outVal = "devicename";
    showTextParam("Device name", uartValue, 2);
  } else if (uartTopic == "TIT") {
    outVal = "title";
    dispTitle = uartValue;
    showTextParam("Title", dispTitle, 2);
  } else if (uartTopic == "ART") {
    outVal = "artist";
    showTextParam("Artist", uartValue, 2);
  } else if (uartTopic == "ALB") {
    outVal = "album";
    showTextParam("Album", uartValue, 2);
  } else {
    outVal = uartTopic;
  }

  return outVal;
}

String decodeUartValue(String uartTopic, String uartCode) {
  String retVal;
  if (uartTopic == "CHN") {
    if (uartCode == "S") {
      retVal = "Stereo";
    } else if (uartCode = "L") {
      retVal = "Left";
    } else if (uartCode = "R") {
      retVal = "Right";
    } else {
      retVal = "Unknown";
    }
  } else if (uartTopic == "MRM") {
    if (uartCode == "S") {
      retVal = "Slave";
    } else if (uartCode == "M") {
      retVal = "Master";
    } else {
      retVal = "None";
    }
    
  } else {
    retVal = "Unknown";
  }
  return retVal;
}

void updateAmpStatus(String statData) {
  int delimPos = statData.indexOf(',');
  int lastPos = 0;
  String parmVal = "";
#if defined(MQTTMODE) && (MQTTMODE == 1 && (WIFIMODE == 1 || WIFIMODE == 2))

  if (delimPos > 0) {
    //Source
    parmVal = statData.substring(4, delimPos);
    client.publish(MQTT_TOPIC_PUB"/source", parmVal.c_str(), true);
    dispSource = parmVal;
    lastPos = delimPos;
    delimPos = statData.indexOf(',', lastPos + 1);
  }

  if ((delimPos > 0) && (lastPos > 0)) {
    //Mute
    parmVal = statData.substring(lastPos+1, delimPos);
    client.publish(MQTT_TOPIC_PUB"/mute", parmVal.c_str(), true);
    lastPos = delimPos;
    delimPos = statData.indexOf(',', lastPos + 1);
  }

  if ((delimPos > 0) && (lastPos > 0)) {
    //Volume
    parmVal = statData.substring(lastPos+1, delimPos);
    client.publish(MQTT_TOPIC_PUB"/volume", parmVal.c_str(), true);
    lastPos = delimPos;
    delimPos = statData.indexOf(',', lastPos + 1);
  }

  if ((delimPos > 0) && (lastPos > 0)) {
    //Treble
    parmVal = statData.substring(lastPos+1, delimPos);
    client.publish(MQTT_TOPIC_PUB"/treble", parmVal.c_str(), true);
    lastPos = delimPos;
    delimPos = statData.indexOf(',', lastPos + 1);
  }
  if ((delimPos > 0) && (lastPos > 0)) {
    //Bass
    parmVal = statData.substring(lastPos+1, delimPos);
    client.publish(MQTT_TOPIC_PUB"/bass", parmVal.c_str(), true);
    lastPos = delimPos;
    delimPos = statData.indexOf(',', lastPos + 1);
  }

  if ((delimPos > 0) && (lastPos > 0)) {
    //Network
    parmVal = statData.substring(lastPos+1, delimPos);
    client.publish(MQTT_TOPIC_PUB"/network", parmVal.c_str(), true);
    lastPos = delimPos;
    delimPos = statData.indexOf(',', lastPos + 1);
  }
  
  if ((delimPos > 0) && (lastPos > 0)) {
    //Internet
    parmVal = statData.substring(lastPos+1, delimPos);
    client.publish(MQTT_TOPIC_PUB"/internet", parmVal.c_str(), true);
    lastPos = delimPos;
    delimPos = statData.indexOf(',', lastPos + 1);
  }

  if ((delimPos > 0) && (lastPos > 0)) {
    //Playing
    parmVal = statData.substring(lastPos+1, delimPos);
    client.publish(MQTT_TOPIC_PUB"/playing", parmVal.c_str(), true);
    lastPos = delimPos;
    delimPos = statData.indexOf(',', lastPos + 1);
  }

  if ((delimPos > 0) && (lastPos > 0)) {
    //LED
    parmVal = statData.substring(lastPos+1, delimPos);
    client.publish(MQTT_TOPIC_PUB"/led", parmVal.c_str(), true);
    lastPos = delimPos;
    delimPos = statData.indexOf(',', lastPos + 1);
  }
  //Upgrading flag not included here
#endif
}

// ===================================
//  Display related functions
// ===================================
void showOTAUpdate() {
    display.clearDisplay();
    display.setTextSize(2); 
    display.setTextColor(WHITE);
    display.invertDisplay(false);
    display.setCursor(0, 0);
    display.println("OTA Update");
    display.display(); 
}

void showTextParam(String parmName, String parmValue, byte textSize) {
    if (!dispModeTempSource) {
      display.clearDisplay();
      display.setTextSize(2); 
      display.setTextColor(WHITE);
      display.invertDisplay(false);
      display.setCursor(0, 0);
      display.println(parmName);
      display.setTextSize(textSize); 
      display.setTextColor(WHITE);
      display.setCursor(0,25);
      display.println(parmValue);
      display.display();
      dispModeTemp_timer = millis();
    }
  
}

void showBooleanParam(String parmName, byte parmValue) {
    if (!dispModeTempSource) {
      display.clearDisplay();
      display.setTextSize(2); 
      display.setTextColor(WHITE);
      display.invertDisplay(false);
      display.setCursor(0, 0);
      display.println(parmName);
      display.setTextSize(3); 
      display.setTextColor(WHITE);
      display.setCursor(40,25);
      if (parmValue) {
        display.println("ON");
      } else {
        display.println("OFF");
      }
      display.display();
      dispModeTemp_timer = millis();
    }
}

void showNumberParam(String parmName, int parmValue) {
    if (!dispModeTempSource) {
      display.clearDisplay();
      display.setTextSize(2); 
      display.setTextColor(WHITE);
      display.invertDisplay(false);
      display.setCursor(0, 0);
      display.println(parmName);
      display.setTextSize(3); 
      display.setTextColor(WHITE);
      display.setCursor(40,25);
      display.println(parmValue);
      display.display();
      dispModeTemp_timer = millis();
    }
  
}

void showSource() {
    display.clearDisplay();
    display.setTextSize(2); 
    display.setTextColor(WHITE);
    display.invertDisplay(false);
    display.setCursor(0, 0);
    display.println("Source");
    display.setTextSize(3); 
    display.setTextColor(WHITE);
    display.setCursor(0,25);
    display.println(dispSource);
    display.display();
    dispModeTemp_timer = millis();
    dispModeTempSource = true;
}

void showMute() {
    if (!dispModeTempSource) {
      display.clearDisplay();
      display.setTextSize(2); 
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("Mute");
      display.setTextSize(3); 
      display.setTextColor(WHITE);
      display.setCursor(40,25);
      display.println(dispMute);
      if (dispMute == "ON") {
        display.invertDisplay(true);
      } else {
        display.invertDisplay(false);
      }
      display.display();
      dispModeTemp_timer = millis();
     }
}
