#ifndef UPDATE_CLIENT_H
#define UPDATE_CLIENT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <Logger.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <regex>
#include <cstring>
#include <string.h>
#include <Preferences.h>

const char UPDATE_PREF_KEY[] = "MQTT";
const char UPDATE_PREF_KEY_UPDATE_HOST[] = "update_host";
const char UPDATE_PREF_KEY_UPDATE_PORT[] = "update_port";
const char UPDATE_PREF_KEY_UPDATE_USER[] = "update_user";
const char UPDATE_PREF_KEY_UPDATE_PASS[] = "update_pass";
const char UPDATE_PREF_KEY_UPDATE_TOPIC[] = "update_topic";

typedef struct
{
    int salt = 34123;
    uint16_t update_port;
    char update_host[64];
    char update_user[64];
    char update_pass[256];
    char update_topic[256];
    char update_protocol[8];

} UpdateSettings;

class UpdateClientClass
{
public:
    void begin(const char* vers, const char* plat);
    void checkForupdate();

    char *getHost(const char *defaultVal = "");
    uint16_t getPort(uint16_t defaultVal = 5000);
    char *getUser(const char *defaultVal = "");
    char *getProtocol(const char *defaultVal = "http");
    char *getTopic(const char *defaultVal = "");

    void resetConfig();
    void saveConfig();
    void reconnect();

    void setHost(const char *host);
    void setPort(uint16_t port);
    void setUser(const char *user);
    void setPass(const char *pass);
    void setProtocol(const char *pass);
    void setTopic(const char *topic);

private:
    UpdateSettings settings;

    char firmware_v[64];
    char plat_v[256];

    void readConfig();
    char *getPass(const char *defaultVal = "");

};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
extern UpdateClientClass UpdateClient;
#endif

#endif