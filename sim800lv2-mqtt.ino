#include <WiFi.h>

// Your GPRS credentials (leave empty, if not needed)
const char apn[]      = "AXIS"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = "axis"; // GPRS User
const char gprsPass[] = "123456"; // GPRS Password

// Server details
// The server variable can be just a domain name or it can have a subdomain. It depends on the service you are using
const char server[] = "hairdresser.cloudmqtt.com";    // domain name: example.com, maker.ifttt.com, etc
const int  port = 18848;                              // server port number

// SIM800L Modem Pins
#define MODEM_RST            5
#define MODEM_TX             17
#define MODEM_RX             16

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT  Serial2

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

// Define the serial console for debug prints, if needed
//#define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

// TinyGSM Client for Internet connection
TinyGsmClient client(modem);
PubSubClient mqtt(client);

unsigned int last_request = 0;

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

  mqtt.setServer(server, port);
  mqtt.setCallback(mqttCallback);
}

void loop() {
  if (millis() - last_request > 50000) {
    SerialMon.print("Connecting to ");
    SerialMon.print(server);

    // Connect to MQTT Broker
    boolean status = mqtt.connect("ADLDev-1", "nglettrq", "RVPcR2AQJEV1");

    if (status == false) {
      SerialMon.println(" fail");
    } else {
      SerialMon.println(" OK");
    }
    
    last_request = millis();
  }

  mqtt.loop();
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();

  // Only proceed if incoming message's topic matches
//  if (String(topic) == topicLed) {
//    ledStatus = !ledStatus;
//    digitalWrite(LED_PIN, ledStatus);
//    mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
//  }
}
