// ESPSmart is library for ESP8266 and ESP32 Arduino boards.
// This library is an extension for simple  MQTT automation.
//
// Copyright (c) 2019 ALab.
//
#define DEBUG_ENABLE  

#include <Arduino.h>
#include <PubSubClient.h>
#include <Client.h>
#include <string.h>
#include "Led.h"
#include "debug.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#else
#error Error. This library only ESP8266 or ESP32!
#endif

// MQTT String size
#define MQTT_SERVER_STR_SIZE 32
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 128
#endif
#define MQTT_ID_STR_SIZE 32
#define MQTT_TOPIC_STR_SIZE 64
#define MQTT_VAL_STR_SIZE 20

// пауза между попытками подключения MQTT в случае обрыва соединения
#define MQTT_RECONNECT_TIMEOUT 10000
// время на попытку соединения по MQTT
#define MQTT_CONNECT_TIMEOUT 10000

#define MQTT_ON_CHANGE_STATUS_CALLBACK_SIGNATURE void (*mqtt_on_change_status_callback)(bool connectStatus)

// WIFI String size
#define WIFI_SSID_STR_SIZE 32
#define WIFI_PASSWORD_STR_SIZE 64

// Time const
// пауза между попытками подключения WiFi в случае обрыва соединения
#define WIFI_RECONNECT_TIMEOUT 10000
// время на попытку соединения по WiFi
#define WIFI_CONNECT_TIMEOUT 10000

#define WIFI_ON_CHANGE_STATUS_CALLBACK_SIGNATURE void (*wifi_on_change_status_callback)(bool connectStatus)

#define BI_LED 2
 
class ESPSmart: public PubSubClient {
    

    // Pointer to WiFi object
    #ifdef ESP8266
    ESP8266WiFiClass *_wifi_obj = NULL;
    #else
    WiFiClass *_wifi = NULL;
    #endif

    

    // WiFi data
    const char *_wifi_ssid;
    const char *_wifi_password;
    uint32_t _wifi_channel = 0;
    // флаг задания параметров wifi подключения во FLASH
    bool _wifi_data_progmem = false;

    // метод проверки связи WiFi
    void check_wifi();

    WIFI_ON_CHANGE_STATUS_CALLBACK_SIGNATURE;

    // MQTT Data
    const char *_mqtt_login;
    const char *_mqtt_password;
    const char *_mqtt_id;
    const char *_mqtt_lwt_topic;
    const char *_mqtt_lwt_message;
    const char *_mqtt_server;
    uint16_t _mqtt_port = 0;

    // метод проверки связи <MQTT>
    void check_mqtt();

    MQTT_ON_CHANGE_STATUS_CALLBACK_SIGNATURE;

    // update web data
    // имя mDNS сервера для установки обновлений
    const char *_updater_host_name;

    bool _mqtt_willRetain = false;
    // флаг задания параметров mqtt подключения во FLASH
    bool _mqtt_data_progmem = false;
    

    // Status flags
    bool _wifi_is_connect = false;
    bool _mqtt_is_connect = false;
    // флаг активируется после выполнения WebUpdaterSetup()
    bool _web_updater_is_setup = false;

    // WebUpdateServer
    void webUpdaterSetup();
    void webUpdaterLoop();
  

    bool _autoConnect = false;

    public:
    // LED
    Led _led;
    // Constructors
    ESPSmart();
    // Methods
    // set initial data:
    //void setWiFiClient(Client &wifi_client);

    void setWiFiCallback(WIFI_ON_CHANGE_STATUS_CALLBACK_SIGNATURE);
    void setMQTTCallback(MQTT_ON_CHANGE_STATUS_CALLBACK_SIGNATURE);
   
    void setMQTT(const char *mqtt_id, const char *mqtt_server, const uint16_t mqtt_server_port, 
    const char *mqtt_lwt_topic, const char *mqtt_lwt_message, bool willRetain = false,
    const char *mqtt_login = NULL, const char *mqtt_password = NULL);

    void setMQTT(const __FlashStringHelper *mqtt_id, const __FlashStringHelper *mqtt_server, const uint16_t mqtt_server_port, 
    const __FlashStringHelper *mqtt_lwt_topic, const __FlashStringHelper *mqtt_lwt_message, bool willRetain = false,
    const __FlashStringHelper *mqtt_login = NULL, const __FlashStringHelper *mqtt_password = NULL)
    { setMQTT_P((PGM_P) mqtt_id, (PGM_P) mqtt_server, mqtt_server_port, 
    (PGM_P) mqtt_lwt_topic, (PGM_P) mqtt_lwt_message, willRetain,
    (PGM_P) mqtt_login, (PGM_P) mqtt_password); }

    void setMQTT_P(PGM_P mqtt_id, PGM_P mqtt_server, const uint16_t mqtt_server_port, 
    PGM_P mqtt_lwt_topic, PGM_P mqtt_lwt_message, bool willRetain = false,
    PGM_P mqtt_login = NULL, PGM_P mqtt_password = NULL);
    
    void setWiFi(const char *wifi_ssid, const char *wifi_password, int32_t wifi_channel = 0);

    void setWiFi(const __FlashStringHelper *wifi_ssid, const __FlashStringHelper *wifi_password, int32_t wifi_channel = 0)
        { setWiFi_P((PGM_P) wifi_ssid, (PGM_P) wifi_password, wifi_channel); }

    void setWiFi_P(PGM_P wifi_ssid, PGM_P wifi_password, int32_t wifi_channel = 0);

    // Check current wi-fi connection status
    bool wifiIsConnect();
     // Check current mqtt connection status
    bool mqttIsConnect();
    // WiFi control
    void wifiDisconnect();
    wl_status_t wifiConnect();
    // MQTT control
    void mqttDisconnect();
    bool mqttConnect();

    //All Control auto
    // устанавливаем флаг автоподключения по WiFi и MQTT
    void setAutoConnect(bool autoConnect);
    bool getAutoConnect();

    //WebUpdate
    void setWebUpdaterHostName(const char *host);
    
    // Передача одного параметра по MQTT
    // кастомные функции передачи, в т.ч. с параметрами из PROGMEM (PM) 

    bool publish_PM(const char *topic, const char *payload, bool retained = false);
    bool publish_M(const __FlashStringHelper *topic, const __FlashStringHelper *payload, bool retained = false)
    { return publish_PM((PGM_P)topic, (PGM_P)payload, retained); }

    bool publish_i8(const char *topic, int8_t payload, bool retained = false);
    bool publish_Pi8(const char *topic, int8_t payload, bool retained = false);
    bool publishi8(const __FlashStringHelper *topic, int8_t payload, bool retained = false)
    { return publish_Pi8((PGM_P) topic, payload, retained); } 

    bool publish_i32(const char *topic, int32_t payload, bool retained = false);
    bool publish_Pi32(const char *topic, int32_t payload, bool retained = false);
    bool publish_i32(const __FlashStringHelper *topic, int32_t payload, bool retained = false)
    { return publish_Pi32((PGM_P) topic, payload, retained); }

    bool publish_d(const char *topic, double payload, signed char width, unsigned char prec, bool retained = false);
    bool publish_Pd(const char *topic, double payload, signed char width, unsigned char prec, bool retained = false);
    bool publish_d(const __FlashStringHelper *topic, double payload, signed char width, unsigned char prec, bool retained = false)
    { return publish_Pd((PGM_P) topic, payload, width, prec, retained); }

    bool publish_b(const char *topic, bool payload, bool retained = false);
    bool publish_Pb(const char *topic, bool payload, bool retained = false);
    bool publish_b(const __FlashStringHelper *topic, bool payload, bool retained = false)
    { return publish_Pb((PGM_P) topic, payload, retained); }

    bool subscribe_P(const char *topic, uint8_t qos = 0);
    
    bool unsubscribe_P(const char *topic);

    // DEBUG
    void printWiFi();

    // return PubSubClient State
    int8_t getMQTTState();

    // return WiFi State
    wl_status_t getWiFiStatus();

    // loop methods
    //true - mqtt client connected
    bool loop();


};