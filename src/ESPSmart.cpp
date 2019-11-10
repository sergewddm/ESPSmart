// ESPSmart is library for ESP8266 and ESP32 Arduino boards.
// This library is an extension for simple MQTT automation.
//
// Copyright (c) 2019 ALab.
//

#include "ESPSmart.h"


// Constructors

ESPSmart::ESPSmart(uint8_t led_pin, bool led_inverse)
{
  // set default values
  _wifi_obj = &WiFi;
  _wifi_is_connect = _mqtt_is_connect = false; 
  _bi_led_pin = led_pin;
  if (_bi_led_pin)
  {
    _pled = new Led(_bi_led_pin, led_inverse);
    _pled->setMode(Led::LED_OFF);
  }
   else _pled = nullptr; 
}


// Wifi setup
void ESPSmart::setWiFi(const char *wifi_ssid, const char *wifi_password, int32_t wifi_channel)
{
  DPRINTLN("[ESPSmart::setWiFi]");
  _wifi_ssid = wifi_ssid;
  _wifi_password = wifi_password;
  _wifi_channel = wifi_channel;
 
  _wifi_data_progmem = false;
}

#ifdef ESP8266
// WebUpdate
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

// WebUpdate setup
void ESPSmart::webUpdaterSetup()
{
  DPRINTLN("[ESPSmart::webUpdaterSetup]");
  if (!_updater_host_name)
  {
    DPRINTLN("Error! No set updaterHostName.");
    return;
  }
  MDNS.begin(_updater_host_name);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  _web_updater_is_setup = true;
}

void ESPSmart::webUpdaterLoop()
{
  // если не проводили setup то пробуем провести
  if (!_web_updater_is_setup)
  {
    if (!wifiIsConnect()) return;
    webUpdaterSetup();
  }

  if (wifiIsConnect())
  {
    httpServer.handleClient();
    MDNS.update();
  }
}
#else
// WebUpdate setup
void ESPSmart::webUpdaterSetup()
{
  
}
#endif



/* void ESPSmart::setWiFi_P(PGM_P wifi_ssid, PGM_P wifi_password, int32_t wifi_channel)
{
  DPRINTLN("[ESPSmart::setWiFi_P]");
  _wifi_ssid = wifi_ssid;
  _wifi_password = wifi_password;
  _wifi_channel = wifi_channel;

  _wifi_data_progmem = true;
} */

void ESPSmart::setMQTT(const char *mqtt_id, const char *mqtt_server, const uint16_t mqtt_server_port,
const char *mqtt_lwt_topic, const char *mqtt_lwt_message, 
bool willRetain, const char* mqtt_login, const char* mqtt_password)
{
   DPRINTLN("[ESPSmart::setMQTT]");
  _mqtt_login = mqtt_login;
  _mqtt_password = mqtt_password;
  _mqtt_id = mqtt_id;
  _mqtt_lwt_topic = mqtt_lwt_topic;
  _mqtt_lwt_message = mqtt_lwt_message;
  _mqtt_willRetain = willRetain;
  _mqtt_server = mqtt_server;
  _mqtt_port = mqtt_server_port;

  _mqtt_data_progmem = false;

  // set SMQTT Server call
  if ((!_mqtt_server) || (!_mqtt_port))
  {
    DPRINT("Not setServer(). Server or port == NULL");
    return;
  }
  DPRINT("setServer()   ");
  DPRINT("Server: ");
  DPRINT(_mqtt_server);
  DPRINT("     port: ");
  DPRINTLN(_mqtt_port);
  setServer(_mqtt_server, _mqtt_port);
}

void ESPSmart::setMQTT_P(PGM_P mqtt_id, PGM_P mqtt_server, const uint16_t mqtt_server_port, 
    PGM_P mqtt_lwt_topic, PGM_P mqtt_lwt_message, bool willRetain,
    PGM_P mqtt_login, PGM_P mqtt_password)
{
  DPRINTLN("[ESPSmart::setMQTT_P]");
  _mqtt_login = mqtt_login;
  _mqtt_password = mqtt_password;
  _mqtt_id = mqtt_id;
  _mqtt_lwt_topic = mqtt_lwt_topic;
  _mqtt_lwt_message = mqtt_lwt_message;
  _mqtt_willRetain = willRetain;
  _mqtt_server = mqtt_server;
  _mqtt_port = mqtt_server_port;

  _mqtt_data_progmem = true;
  
  // SetServer from PGM!!!
  if ((!_mqtt_server) || (!_mqtt_port))
  {
    DPRINT("Not setServer(). Server or port == NULL");
    return;
  }
  char mqtt_server_buf[MQTT_SERVER_STR_SIZE];
  strcpy_P(mqtt_server_buf, _mqtt_server);
  
  DPRINT("setServer()   ");
  DPRINT("Server: ");
  DPRINT(mqtt_server_buf);
  DPRINT("     port: ");
  DPRINTLN(_mqtt_port);
  setServer(mqtt_server_buf, _mqtt_port);
}

void ESPSmart::setWebUpdaterHostName(const char *host)
{
  _updater_host_name = host;
}

void ESPSmart::setAutoConnect(bool autoConnect)
{
  if (autoConnect == _autoConnect) return;
  // устанавливаем флаг автоподключения
  _autoConnect = autoConnect;
  // включаем всё что было выключено
  //if (!wifiIsConnect) wifiConnect()
  // DEBUG возможно, достаточно установить флаг.
  // в цикле автоконнект сам всё подключит 
}

void ESPSmart::setLed(uint8_t led_pin, bool led_logic)
{
  _bi_led_pin = led_pin;
  if (_bi_led_pin)
  {
    _pled = new Led(_bi_led_pin, led_logic);
    _pled->setMode(Led::LED_OFF);
  }
   else _pled = nullptr; 
}

void ESPSmart::printWiFi()
{

  if (_wifi_data_progmem)
  {
    char wifi_ssid_buf[WIFI_SSID_STR_SIZE];
    char wifi_pass_buf[WIFI_PASSWORD_STR_SIZE];

    strcpy_P(wifi_ssid_buf, _wifi_ssid);
    strcpy_P(wifi_pass_buf, _wifi_password);

  Serial.print("WiFi SSID: ");
  Serial.print(wifi_ssid_buf);
  Serial.print("   WiFi Password: ");
  Serial.print(wifi_pass_buf);
  Serial.print("   PROGMEM: YES");
  }
 else
 {
  Serial.print("WiFi SSID: ");
  Serial.print(_wifi_ssid);
  Serial.print("   WiFi Password: ");
  Serial.print(_wifi_password);
  Serial.print("   PROGMEM: NO");
 }
}

bool ESPSmart::getAutoConnect()
{
  return _autoConnect;
}

bool ESPSmart::mqttIsConnect()
{
   return connected();
}

bool ESPSmart::wifiIsConnect()
{
  if (!_wifi_obj)
  {
     return false;
  }

  if (_wifi_obj->status() == WL_CONNECTED) return true;
   else return false;  
}

void ESPSmart::wifiDisconnect()
{
  if (!_wifi_obj) return;
  _wifi_obj->disconnect();
}

wl_status_t ESPSmart::wifiConnect()
{
  // проверка существования сылки на объект WiFi и наличия установленных данных WiFi
  if (!_wifi_obj) return WL_CONNECT_FAILED;
  if ((!_wifi_ssid) || (!_wifi_password)) return WL_CONNECT_FAILED;

  if (_wifi_obj->status() == WL_CONNECTED) _wifi_obj->disconnect();

  //wl_status_t st;
  // ссылки на готовые С String
  const char *wifi_ssid;
  const char *wifi_pass;

  // Формируем реальные строки с учётом того, где хранятся данные WiFi подключения
  if (_wifi_data_progmem)
  {
    char ssid_buf[WIFI_SSID_STR_SIZE];
    char pass_buf[WIFI_PASSWORD_STR_SIZE];

    wifi_ssid = strcpy_P(ssid_buf, _wifi_ssid);
    wifi_pass = strcpy_P(pass_buf, _wifi_password);
  }
  else
  {
    wifi_ssid = _wifi_ssid;
    wifi_pass = _wifi_password;   
  }

  // запускаем подключение по WiFi    
  return _wifi_obj->begin(wifi_ssid, wifi_pass, _wifi_channel);
}

void ESPSmart::check_wifi()
{
  if (wifiIsConnect())
  {
    // если подключились первый раз
    if (!_wifi_is_connect)
    {
      DPRINTLN("[ESPSmart::check_wifi()]. Set _wifi_is_connect = true");
      _wifi_is_connect = true;
      if (_bi_led_pin) _pled->setMode(Led::LED_ON); 
    }
    return;
  }

  // wifi выкл.
  if (_wifi_is_connect)
    {
      DPRINTLN("[ESPSmart::check_wifi()]. Set _wifi_is_connect = false");
      _wifi_is_connect = false;
      if (_bi_led_pin) _pled->setMode(Led::LED_4HZ);      
    }

  // счёчик времени на установление соединения по WiFi
  static uint32_t _wifi_begin_counter = 0;
  // счётчик паузы между попытками соединения WiFi
  static uint32_t _wifi_reconnect_counter = 0;

  if ((!_wifi_begin_counter) && (!_wifi_reconnect_counter))
  {
    DPRINTLN("[ESPSmart::checkwifi()]. Set reconnect timer");
    //reconnect start
    _wifi_reconnect_counter = millis();
    if(!_wifi_reconnect_counter) _wifi_reconnect_counter++;
  }
  else if (_wifi_reconnect_counter)
  {
    // если таймер паузы между попытками соединения истёк, то подключаемся
    if ((millis() - _wifi_reconnect_counter) >= WIFI_RECONNECT_TIMEOUT)
    {
      DPRINTLN("[ESPSmart::checkwifi()]. Reconnect timer elapsed. wifiConnect(). Set wifi begin timer.");
      _wifi_reconnect_counter = 0;
      _wifi_begin_counter = millis();
      if (!_wifi_begin_counter) _wifi_begin_counter++;
      wifiConnect();
    }
  }
  else if (_wifi_begin_counter)
  {
    // проверяем таймаут на установку соединения
    if ((millis() - _wifi_begin_counter) >= WIFI_CONNECT_TIMEOUT)
    {
        DPRINTLN("[ESPSmart::checkwifi()]. WiFi begin timer elapsed. WiFi connect fail.");
        // не смогли соединиться, запускаем отсчёт паузы между соединениями
        _wifi_begin_counter = 0;
    }
  }
}

void ESPSmart::mqttDisconnect()
{
  disconnect();
}

void ESPSmart::mqttConnect()
{
  DPRINTLN("[ESPSmart::mqttConnect]");
  if (!wifiIsConnect()) 
  {
    DPRINTLN("!wifiIsConnect()");
    return;
  }
  if (!_mqtt_id)
  {
    DPRINTLN("!_mqtt_id");
    return;
  } 
  if (!_mqtt_server) 
  {
    DPRINTLN("!_mqtt_server");
    return;
  }
  if (!_mqtt_port)
  { 
    DPRINTLN("!_mqtt_port");
    return;
  }
  if (connected()) 
  {
    DPRINTLN("isConnected -> disconnect()");
    disconnect();
  }

  // ссылки на готовые строки для подключения
  const char *mqtt_id;
  const char *mqtt_server;
  const char *mqtt_lwt_topic;
  const char *mqtt_lwt_message;
  const char *mqtt_login;
  const char *mqtt_pass;

  // Формируем данные (строки) для подключения с учётом того, где храняться исходные данные подключения
  if (_mqtt_data_progmem)
  {
    char id_buf[MQTT_ID_STR_SIZE];
    mqtt_id = strcpy_P(id_buf, _mqtt_id);
    DPRINT("set id from progmem: ");
    DPRINTLN(mqtt_id);
    
    char server_buf[MQTT_SERVER_STR_SIZE];
    mqtt_server = strcpy_P(server_buf, _mqtt_server);
    DPRINT("set server from progmem: ");
    DPRINTLN(mqtt_server);

    if (_mqtt_lwt_topic)
    {
      char lwt_topic_buf[MQTT_TOPIC_STR_SIZE];
      mqtt_lwt_topic = strcpy_P(lwt_topic_buf, _mqtt_lwt_topic);
      DPRINT("set lwt_topic from progmem: ");
      DPRINTLN(mqtt_lwt_topic);
    }

     if (_mqtt_lwt_message)
    {
      char lwt_message_buf[MQTT_MAX_PACKET_SIZE];
      mqtt_lwt_message = strcpy_P(lwt_message_buf, _mqtt_lwt_message);
      DPRINT("set lwt_message from progmem: ");
      DPRINTLN(mqtt_lwt_message);
    }

    if (_mqtt_login)
    {
      char login_buf[WIFI_SSID_STR_SIZE];
      mqtt_login = strcpy_P(login_buf, _mqtt_login);
      DPRINT("set mqtt_login from progmem: ");
      DPRINTLN(mqtt_login);
    }

    if (_mqtt_password)
    {
      char pass_buf[WIFI_PASSWORD_STR_SIZE];
      mqtt_pass = strcpy_P(pass_buf, _mqtt_password);
      DPRINT("set mqtt_pass from progmem: ");
      DPRINTLN(mqtt_pass);
    }
  }
  else
  {
    DPRINTLN("copy dat no progmem");
    mqtt_id = _mqtt_id;
    mqtt_server = _mqtt_server;
    mqtt_lwt_topic = _mqtt_lwt_topic;
    mqtt_lwt_message = _mqtt_lwt_message;
    mqtt_login = _mqtt_login;
    mqtt_pass = _mqtt_password;
  }
  
  
  //setServer(mqtt_server, _mqtt_port);
  
  if ((mqtt_login) && (mqtt_pass) && (mqtt_lwt_topic) && (mqtt_lwt_message))
  {
    DPRINTLN("connect(id, login, pass, lwt_topic, QoS, willRetain, lwt_msg)");
    DPRINT("id: ");
    DPRINT(mqtt_id);
    DPRINT("  login: ");
    DPRINT(mqtt_login);
    DPRINT("  pass: ");
    DPRINT(mqtt_pass);
    DPRINT("  lwt_topic: ");
    DPRINT(mqtt_lwt_topic);
    DPRINT("  lwt_message: ");
    DPRINTLN(mqtt_lwt_message);
    
    setClientId(mqtt_id);
    setCredentials(mqtt_login, mqtt_pass);
    setWill(mqtt_lwt_topic, 1, _mqtt_willRetain, mqtt_lwt_message);
    connect();
  }
  else if ((mqtt_login) && (mqtt_pass))
  {
    DPRINTLN("connect(id, login, pass)");
    DPRINT("id: ");
    DPRINT(mqtt_id);
    DPRINT("  login: ");
    DPRINT(mqtt_login);
    DPRINT("  pass: ");
    DPRINTLN(mqtt_pass);

    setClientId(mqtt_id);
    setCredentials(mqtt_login, mqtt_pass);
    connect();
  }
  else
  {
     DPRINTLN("connect(id)");
     DPRINT("id: ");
     DPRINTLN(mqtt_id);
     
     setClientId(mqtt_id);
     connect();
  }
}

void ESPSmart::check_mqtt()
{
  if (mqttIsConnect())
  {
    // если подключились первый раз
    if (!_mqtt_is_connect)
    {
       DPRINTLN("[ESPSmart::check_mqtt()]. Set _mqtt_is_connect = true");
      _mqtt_is_connect = true;
      if (_bi_led_pin) _pled->setMode(Led::LED_FADEINOUT); 
    }
    return;
  }

  // mqtt disconnect
  if (_mqtt_is_connect)
    {
       DPRINTLN("[ESPSmart::check_mqtt()]. Set _mqtt_is_connect = false");
      _mqtt_is_connect = false;

      if (_bi_led_pin)
      {
        // проверяем, доступен ли WiFi
        if (wifiIsConnect()) _pled->setMode(Led::LED_ON);
         else _pled->setMode(Led::LED_4HZ);
      }
      
    }   

  // счёчик времени на установление соединения по MQTT
  static uint32_t _mqtt_begin_counter = 0;
  // счётчик паузы между попытками соединения MQTT
  static uint32_t _mqtt_reconnect_counter = 0;

  if ((!_mqtt_begin_counter) && (!_mqtt_reconnect_counter))
  {
    DPRINTLN("[ESPSmart::checkmqtt()]. Set reconnect timer");
    //reconnect start
    _mqtt_reconnect_counter = millis();
    if(!_mqtt_reconnect_counter) _mqtt_reconnect_counter++;
  }
  else if (_mqtt_reconnect_counter)
  {
    // если таймер паузы между попытками соединения истёк, то подключаемся
    if ((millis() - _mqtt_reconnect_counter) >= MQTT_RECONNECT_TIMEOUT)
    {
      DPRINTLN("[ESPSmart::checkmqtt()]. Reconnect timer elapsed. mqttConnect(). Set mqtt begin timer.");
      _mqtt_reconnect_counter = 0;
      _mqtt_begin_counter = millis();
      if (!_mqtt_begin_counter) _mqtt_begin_counter++;
      mqttConnect();
    }
  }
  else if (_mqtt_begin_counter)
  {
    // проверяем таймаут на установку соединения
    if ((millis() - _mqtt_begin_counter) >= MQTT_CONNECT_TIMEOUT)
    {
        DPRINTLN("[ESPSmart::checkmqtt()]. MQTT begin timer elapsed. MQTT connect fail.");
        // не смогли соединиться, запускаем отсчёт паузы между соединениями
        _mqtt_begin_counter = 0;
    }
  }
}

/* bool ESPSmart::publish_PM(const char *topic, const char *payload, bool retained)
{
  if (!topic) return false;

  char topic_buf[MQTT_TOPIC_STR_SIZE];
  strcpy_P(topic_buf, topic);

  return publish_P(topic_buf, payload, retained);
} */

bool ESPSmart::publish_i8(const char *topic, int8_t payload, bool retained)
{
  if (!topic) return false;

  char buf[MQTT_VAL_STR_SIZE];
  itoa(payload, buf, 10); 
  return publish(topic, 1, retained, buf);
}

bool ESPSmart::publish_Pi8(const char *topic, int8_t payload, bool retained)
{
  if (!topic) return false;

  char buf[MQTT_VAL_STR_SIZE];
  itoa(payload, buf, 10); 

  char topic_buf[MQTT_TOPIC_STR_SIZE];
  strcpy_P(topic_buf, topic);

  return publish(topic_buf, retained, buf);
} 


bool ESPSmart::publish_i32(const char *topic, int32_t payload, bool retained)
{
  DPRINTLN("[ESPSmart::pblish_i32]");
  if (!topic) 
  {
    DPRINTLN("topic == NULL");
    return false;
  }

  char buf[MQTT_VAL_STR_SIZE];
  ultoa(payload, buf, 10); 
  DPRINT("publish   ");
  DPRINT("topic: ");
  DPRINT(topic);
  DPRINT("   buf: ");
  DPRINT(buf);
  DPRINT("   retained: ");
  DPRINTLN(retained);
  return publish(topic, retained, buf);
}

bool ESPSmart::publish_Pi32(const char *topic, int32_t payload, bool retained)
{
  if (!topic) return false;
  
  char buf[MQTT_VAL_STR_SIZE];
  ultoa(payload, buf, 10); 

  char topic_buf[MQTT_TOPIC_STR_SIZE];
  strcpy_P(topic_buf, topic);

  return publish(topic_buf, retained, buf);
}

bool ESPSmart::publish_d(const char *topic, double payload, signed char width, unsigned char prec, bool retained)
{
  if (!topic) return false;

  char buf[MQTT_VAL_STR_SIZE];
  dtostrf(payload, width, prec, buf);
  return publish(topic, retained, buf);
}

bool ESPSmart::publish_Pd(const char *topic, double payload, signed char width, unsigned char prec, bool retained)
{
  if (!topic) return false;
  
  char buf[MQTT_VAL_STR_SIZE];
  dtostrf(payload, width, prec, buf);

  char topic_buf[MQTT_TOPIC_STR_SIZE];
  strcpy_P(topic_buf, topic);

  return publish(topic_buf, retained, buf);
}

bool ESPSmart::publish_b(const char *topic, bool payload, bool retained)
{
  if (!topic) return false;

  char buf[MQTT_VAL_STR_SIZE];
  itoa((uint8_t)payload, buf, 10);
  return publish(topic, retained, buf);
}

bool ESPSmart::publish_Pb(const char *topic, bool payload, bool retained)
{
  if (!topic) return false;
  
  char buf[MQTT_VAL_STR_SIZE];
  itoa((uint8_t)payload, buf, 10);

  char topic_buf[MQTT_TOPIC_STR_SIZE];
  strcpy_P(topic_buf, topic);

  return publish(topic_buf, retained, buf);
}

bool ESPSmart::subscribe_P(const char *topic, uint8_t qos)
{
  if (!topic) return false;

  char topic_buf[MQTT_TOPIC_STR_SIZE];
  strcpy_P(topic_buf, topic);

  return subscribe(topic_buf, qos);
}

bool ESPSmart::unsubscribe_P(const char *topic)
{
  if (!topic) return false;

  char topic_buf[MQTT_TOPIC_STR_SIZE];
  strcpy_P(topic_buf, topic);

  return unsubscribe(topic_buf);
}

/* int8_t ESPSmart::getMQTTState()
{
  return state();
} */

wl_status_t ESPSmart::getWiFiStatus()
{
  if (! _wifi_obj) return WL_CONNECT_FAILED;
   else return _wifi_obj->status();
}

void ESPSmart::loop()
{
  if (_bi_led_pin) _pled->update();
  webUpdaterLoop();

  if (_autoConnect) 
  {
    check_wifi();
    check_mqtt();
  }
}