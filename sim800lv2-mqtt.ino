#include <WiFi.h>
#include "time.h"
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Your GPRS credentials (leave empty, if not needed)
const char apn[]        = "AXIS";   // APN
const char gprsUser[]   = "axis";   // GPRS User
const char gprsPass[]   = "123456"; // GPRS Password

// Server details
// The server variable can be just a domain name or it can have a subdomain. It depends on the service you are using
const char mqttServer[] = "hairdresser.cloudmqtt.com";        // MQTT broker
const char mqttUser[]   = "nglettrq";
const char mqttPwd[]    = "RVPcR2AQJEV1";
const char ntpServer[]  = "pool.ntp.org";                     // server to synchronize time
String deviceId         = "ADLDev-1";
String pubTopic         = String(deviceId + "/relayStatus");
String subTopic         = String(deviceId + "/relayControl");
String deviceMac        = "2286179853734245";
const int  mqttPort     = 18848;                              // server port number

// SIM800L Modem Pins
#define MODEM_RST       5
#define MODEM_TX        17
#define MODEM_RX        16

#define PERIOD          50000

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT  Serial2

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <TinyGsmClient.h>
#include <PubSubClient.h>

TinyGsm modem(SerialAT);

// TinyGSM Client for Internet connection
TinyGsmClient client(modem);
PubSubClient mqtt(client);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const long utcOffsetInSeconds = 0;
unsigned long relayStatus = 0;
unsigned long epochTime = 0;
unsigned long lastRequest = 0;
String request = "relay on\r\n";
String relayString = "";
String inputString = "";
bool stringComplete = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds);

void setup() {
  // Set serial monitor debugging window baud rate to 115200
  SerialMon.begin(115200);

  // Set modem reset
  pinMode(MODEM_RST, OUTPUT);
  digitalWrite(MODEM_RST, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(38400);
  delay(3000);

  // Restart SIM800 module, it takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();

  SerialMon.print("Connecting to APN: ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
  }
  else {
    SerialMon.println(" OK");
  }

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(mqttCallback);

  timeClient.begin();
}

void loop() {
  if (millis() - lastRequest > 50000) {
    SerialMon.print("Connecting to ");
    SerialMon.print(mqttServer);

    // Connect to MQTT Broker
    boolean status = mqtt.connect(deviceId.c_str(), mqttUser, mqttPwd);

    if (status == false) {
      SerialMon.println(" fail");
    } else {
      SerialMon.println(" OK");

      print_local_time();
      send_event();
    }
    
    lastRequest = millis();
  }

  mqtt.loop();
}

void print_local_time()
{
  timeClient.update();

  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());

  epochTime = timeClient.getEpochTime();
}

void publish_message(const char* message){
  mqtt.publish(pubTopic.c_str(), message);
  Serial.println("Event published...");  
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();
}

//====================================================================================================================================================================  
void send_event(){
  char msgToSend[1024] = {0};
  char relay[2];
  char epochMil[32];
  char deviceSerial[32];

  String sepoch = String(epochTime);
  sepoch.toCharArray(epochMil, sizeof(epochMil));
  deviceMac.toCharArray(deviceSerial, sizeof(deviceSerial));
  dtostrf(relayStatus, 1, 0, relay);

  strcat(msgToSend, "{\"eventName\":\"relayStatus\",\"status\":\"none\"");
  strcat(msgToSend, ",\"relay\":");
  strcat(msgToSend, relay);
  strcat(msgToSend, ",\"time\":");
  strcat(msgToSend, epochMil);
  strcat(msgToSend, ",\"mac\":\"");
  strcat(msgToSend, deviceSerial);
  strcat(msgToSend, "\"}");
  publish_message(msgToSend);  //send the event to backend
  memset(msgToSend, 0, sizeof msgToSend);
}
//====================================================================================================================================================================
//==================================================================================================================================================================== 
void do_actions(const char* message){
  Serial.println(message);
  // Check if received command whether 'relay_on'or 'relay_off
  if (strcmp(message, "relay_on") == 0) {
    //Send command to Arduino to turn on relay
  } else if (strcmp(message, "relay_off") == 0) {
    //Send command to Arduino to turn off relay
  } else {
    Serial.println("Command is not recognized");
  }
}
