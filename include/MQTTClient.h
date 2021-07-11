#ifndef MQTT_Client_H
#define MQTT_Client_H

#include <ESPmDNS.h>
#include <WiFiClientSecure.h>
#include "PubSubClient.h"
#include <Preferences.h>
#include <cstring>
#include <string.h>
#include <regex>
#include <Logger.h>
#include "ArduinoJson.h"
#include "SPIFFS.h"
#include <time.h>

const char MQTT_CERTIFICATE_FILENAME[] = "/mqtt.ca";

const char KEY_CAM_NAME[] = "cam_name";
const char KEY_ACTIVE_CLIENTS[] = "active_clients";
const char KEY_CLIENT_ID[] = "client_id";

const char MQTT_PREF_KEY[] = "MQTT";
const char MQTT_PREF_KEY_MMQT_HOST[] = "mmqt_host";
const char MQTT_PREF_KEY_MMQT_PORT[] = "mmqt_port";
const char MQTT_PREF_KEY_MMQT_USER[] = "mmqt_user";
const char MQTT_PREF_KEY_MMQT_PASS[] = "mmqt_pass";
const char MQTT_PREF_KEY_MMQT_TOPIC[] = "mmqt_topic";
const char MQTT_PREF_KEY_MMQT_PUBLISH_PERIOD[] = "mqtt_pub_period";

static WiFiClientSecure wifiClient;
static PubSubClient mqttClient(wifiClient);

typedef struct
{
    int salt = 12664;
    uint16_t mqtt_port;
    char mqtt_host[64];
    char mqtt_user[64];
    char mqtt_pass[256];
    char mqtt_topic[256];
    unsigned long publish_period;

} MQTTSettings;

class MQTTClientClass
{
public:

    MQTTClientClass();
    ~MQTTClientClass();

    void begin(const char *json);
    void reconnect();
    void loop();
    void publish(StaticJsonDocument<200> &doc);

    char *getHost(const char *defaultVal = "");
    uint16_t getPort(uint16_t = 8883);
    unsigned long getPublishPeriod(const unsigned long defaultVal = 600);
    char *getUser(const char *defaultVal = "");
    char *getTopic(const char *defaultVal = "");
    void resetConfig();

    void setHost(const char *host);
    void setPort(uint16_t port);
    void setUser(const char *user);
    void setPass(const char *pass);
    void setPublishPeriod(unsigned long publish_period);
    void setTopic(const char *topic);
    void saveConfig();

private:
    
    MQTTSettings settings;

    char client_id[32];
    char json_format[2048];
    char mqtt_topic[64];

    void connect();
    void readConfig();
    char *getPass(const char *defaultVal = "");
    template<typename L> void loadFromFile(const char* , L&& );
    void loadCertificates(WiFiClientSecure* client);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
extern MQTTClientClass MQTTClient;
#endif

#endif