// Light controller in the Livingroom. v. 2
//
// RF Codes для люсты и звонка
//
// 1. Звонок: 8385797, Protocol 1, PulseLength: 144-145 ms.
// 2. Лампы A : 7996049, Protocol 1, PulseLength : 250 ms.
// 3. Лампы B : 7996050, Protocol 1, PulseLength : 251 ms.
// 4. Лампы OnOff : 7996052, Protocol 1, PulseLength : 251 ms.
// 5. Кнопка проветривания в туалете: 11111944
//
// 
//

// Uncomment this to disable it 
//#define DEBUG_DISABLE_DEBUGGER true
//#define DEBUG_INITIAL_LEVEL DEBUG_LEVEL_VERBOSE
//#define DEBUG_USE_FLASH_F true

#define DEBUG_ENABLE   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.

#include <Arduino.h>
#include <pgmspace.h>
//#include <ESP8266WiFi.h>

#include "ESPSmart.h"

#include "debugStr.h"
#define BUILTIN_LED_PIN 2





const char *wifi_ssid  = "WdLink";
const char *wifi_password  = "aeroglass";

const char *mqtt_id = "Hallight2_cl";
const char *mqtt_server = "m23.cloudmqtt.com";
const int mqtt_server_port = 16247;
const char *mqtt_login = "rwkscoyb";
const char *mqtt_password = "TpLdgiXdjcUo";

const char *updater_host_name = "halllight2";

const char *TOPIC_TEST_OUT = "home/halllight2/out";
const char *TOPIC_TEST_IN = "home/halllight2/in";



//WiFiClient cl;
ESPSmart esps;


// ASYNC MQTT
void onWifiConnect(const WiFiEventStationModeGotIP& event) 
{
  DPRINTLN("Connected to Wi-Fi.");
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) 
{
  DPRINTLN("Disconnected from Wi-Fi.");
}

void onMqttConnect(bool sessionPresent) 
{
  DPRINTLN("Connected to MQTT.");
  DPRINT("Session present: ");
  DPRINTLN(sessionPresent);
  uint16_t packetIdSub = esps.subscribe(TOPIC_TEST_IN, 2);
  DPRINT("Subscribing at QoS 2, packetId: ");
  DPRINTLN(packetIdSub);
  
  // mqttClient.publish("test/lol", 0, true, "test 1");
  // Serial.println("Publishing at QoS 0");
  // uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
  // Serial.print("Publishing at QoS 1, packetId: ");
  // Serial.println(packetIdPub1);
  // uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  // Serial.print("Publishing at QoS 2, packetId: ");
  // Serial.println(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
{
  DPRINTLN("Disconnected from MQTT.");
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) 
{
  DPRINTLN("Subscribe acknowledged.");
  DPRINT("  packetId: ");
  DPRINTLN(packetId);
  DPRINT("  qos: ");
  DPRINTLN(qos);
}


void onMqttUnsubscribe(uint16_t packetId) 
{
  DPRINTLN("Unsubscribe acknowledged.");
  DPRINT("  packetId: ");
  DPRINTLN(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
  DPRINTLN("Publish received.");
  DPRINT("  topic: ");
  DPRINTLN(topic);
  DPRINT("  qos: ");
  DPRINTLN(properties.qos);
  DPRINT("  dup: ");
  DPRINTLN(properties.dup);
  DPRINT("  retain: ");
  DPRINTLN(properties.retain);
  DPRINT("  len: ");
  DPRINTLN(len);
  DPRINT("  index: ");
  DPRINTLN(index);
  DPRINT("  total: ");
  DPRINTLN(total);

  char* p = (char*)malloc(len + 1);
	memset(p, '\0', len + 1);
	// Copy the payload to the new buffer
	memcpy(p, payload, len);
  String val = String(p);
  DPRINT("     Data: ");
  DPRINTLN(val);

  // Free the memory
	free(p);
}

void onMqttPublish(uint16_t packetId) 
{
  DPRINTLN("Publish acknowledged.");
  DPRINT("  packetId: ");
  DPRINTLN(packetId);
}

// ASYNC MQTT

void setup() 
{
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  Serial.println();
  delay(100);
  DPRINTLN("[setup]");
  WiFi.onStationModeGotIP(onWifiConnect);
  WiFi.onStationModeDisconnected(onWifiDisconnect);

  esps.onConnect(onMqttConnect);
  esps.onDisconnect(onMqttDisconnect);
  esps.onSubscribe(onMqttSubscribe);
  esps.onUnsubscribe(onMqttUnsubscribe);
  esps.onMessage(onMqttMessage);
  esps.onPublish(onMqttPublish);
  //esps.setServer(MQTT_HOST, MQTT_PORT);

  esps.setLed(BUILTIN_LED_PIN, true);
  esps.setWiFi(wifi_ssid, wifi_password);
  esps.setMQTT(mqtt_id, mqtt_server, mqtt_server_port, NULL, NULL, false, mqtt_login, mqtt_password);
 
  esps.setWebUpdaterHostName(updater_host_name);

  // Autoconnect!!!
  esps.setAutoConnect(true);

 /*  DPRINT("Connecting to WiFi");
  esps.wifiConnect();
  uint8_t cnt = 10;
  while(cnt)
  {
    if (esps.getWiFiStatus() != WL_CONNECTED) 
    {
      DPRINT(".");
    }
    else break;
    cnt--;
    delay(1000);
  }
  Serial.println();
  
  if (esps.getWiFiStatus() != WL_CONNECTED)
  {
     DPRINTLN("Error connected to WiFi. Status: ");
     Serial.println((uint8_t)esps.getWiFiStatus());
     exit(0);
  }
  else
  {
    DPRINTLN("WiFi Connected sucessfull.");
  }


  // MQTT
  cnt = 5;
  DPRINT("Connecting to MQTT");
  //esps.mqttConnect();

   while(cnt)
  {
    if (!esps.mqttConnect())
    {
       DPRINT(".");
    }
    else
    {
       break;
    }
    cnt--;
    delay(2000);
  }
  Serial.println();
  
  if (!esps.mqttIsConnect())
  {
     DPRINTLN("Error connected to MQTT. Status: ");
     DPRINT((int8_t)esps.getMQTTState());
     exit(1);
  }
  else
  {
    DPRINTLN("MQTT Connected sucessfull.");
    
    //subscribe
    esps.subscribe(TOPIC_TEST_IN);
  } */
  

}


uint32_t counter = 0;
void loop() 
{
   esps.loop();
   if (!(millis() % 5000))
   {
     counter++;
     char buf[10];
     itoa(counter, buf, 10);
     uint16_t packetIdPub1 = esps.publish(TOPIC_TEST_OUT, 1, true, buf);
     DPRINT("Publishing at QoS 1, packetId: ");
     DPRINTLN(packetIdPub1);
   }
}