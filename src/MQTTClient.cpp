#include "MQTTClient.h"

MQTTClientClass::MQTTClientClass(){};
MQTTClientClass::~MQTTClientClass(){};

void MQTTClientClass::begin(const char *jsonFormatStr)
{
    strcpy(json_format, jsonFormatStr);

    //Read the configuraiton from preferences
    readConfig();

    if (SPIFFS.exists("/mqtt.ca"))
    {
        File file = SPIFFS.open(MQTT_CERTIFICATE_FILENAME, FILE_READ);
        Logger.log("CA Cert: \n%s", file.readString().c_str());
        wifiClient.setCACert(file.readString().c_str());
    }
    if (SPIFFS.exists("/mqtt.crt"))
    {
        File file = SPIFFS.open("/mqtt.crt", FILE_READ);
        Logger.log("MQTT Cert: \n%s", file.readString().c_str());
        wifiClient.setCertificate(file.readString().c_str());
    }
    if (SPIFFS.exists("/mqtt.key"))
    {
        File file = SPIFFS.open("/mqtt.key", FILE_READ);
        Logger.log("MQTT Key: \n%s", file.readString().c_str());
        String cert = file.readString();

        wifiClient.setPrivateKey(file.readString().c_str());
    }

    if (!mqttClient.connected())
    {
        connect();
    }
}

void MQTTClientClass::resetConfig()
{
    try
    {
        setHost("");
        setUser("");
        setPass("");
        setPort(1883);
        setTopic("");
        setPublishPeriod(600);

        Logger.log("Saving empty config");
        saveConfig();

        if (mqttClient.connected())
        {
            Logger.log("MQTT Connected, disconnecting..");
            mqttClient.disconnect();
        }
    }
    catch (const std::exception &e)
    {
        Logger.log(e.what());
    }
}

void MQTTClientClass::loop()
{
    mqttClient.loop();
}

void MQTTClientClass::publish(StaticJsonDocument<200> &doc)
{
    //Append our client_id
    doc[KEY_CLIENT_ID] = client_id;

    char msg[300];
    serializeJson(doc, msg);

    Logger.log("Publishing message '%s' to '%s'", msg, getTopic());

    if (mqttClient.connected())
    {
        /* publish the message */
        mqttClient.publish(getTopic(), msg);
        return;
    }
}

void MQTTClientClass::connect()
{
    Logger.log("Starting MQTT Client");

    if (strcmp(getHost(), "") == 0)
    {
        Logger.log("Couldn't establish MQTT Connection, no host given");

        return;
    }

    /* Configure the MQTT server with IPaddress and port. */
    Logger.log("Querying '%s'...", getHost());

    IPAddress mqttHostIp;
    WiFi.hostByName(getHost(), mqttHostIp);

    if ((uint32_t)mqttHostIp == 0)
    {
        Logger.log("Unable to locate MQTT Host: %s", getHost());
        return;
    }

    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    sprintf(client_id, "%u", chipId);
    Logger.log("IP address of MQTT server: %s:%d", mqttHostIp.toString().c_str(), getPort());

    /* connect now */
    Logger.log("MQTT connecting as '%s' (%s:%s)...", client_id, getUser(), getPass());

    mqttClient.setServer(mqttHostIp, getPort());
    if (mqttClient.connect(client_id, getUser(), getPass()))
    {
        Logger.log("MQTT Connected to '%s'", getHost());
        delay(250);
    }
    else
    {
        mqttClient.disconnect();
        Logger.log("Failed, status code =%d.", mqttClient.state());
    }
}

void MQTTClientClass::reconnect()
{
    Logger.log("Restarting MQTT Connection");

    //disconnect the current session
    mqttClient.disconnect();

    //reconnect
    begin(json_format);
}

void MQTTClientClass::readConfig()
{
    Logger.log("Reading MQTT configuration.");

    Preferences preferences;
    preferences.begin(MQTT_PREF_KEY, true);

    setPort(preferences.getUInt(MQTT_PREF_KEY_MMQT_PORT, 1883));
    setHost(preferences.getString(MQTT_PREF_KEY_MMQT_HOST, "").c_str());
    setUser(preferences.getString(MQTT_PREF_KEY_MMQT_USER, "").c_str());
    setPass(preferences.getString(MQTT_PREF_KEY_MMQT_PASS, "").c_str());
    setTopic(preferences.getString(MQTT_PREF_KEY_MMQT_TOPIC, "default/topic").c_str());
    setPublishPeriod(preferences.getULong(MQTT_PREF_KEY_MMQT_PUBLISH_PERIOD, 600));

    preferences.end();
}

void MQTTClientClass::saveConfig()
{
    Logger.log("Saving MQTT configuration.");

    Preferences preferences;
    preferences.begin(MQTT_PREF_KEY, false);

    preferences.putUInt(MQTT_PREF_KEY_MMQT_PORT, getPort());
    preferences.putString(MQTT_PREF_KEY_MMQT_HOST, getHost());
    preferences.putString(MQTT_PREF_KEY_MMQT_USER, getUser());
    preferences.putString(MQTT_PREF_KEY_MMQT_PASS, getPass());
    preferences.putString(MQTT_PREF_KEY_MMQT_TOPIC, getTopic());
    preferences.putULong(MQTT_PREF_KEY_MMQT_PUBLISH_PERIOD, getPublishPeriod());

    preferences.end();
}

char *MQTTClientClass::getHost(const char defaultVal[])
{

    if (!std::regex_match(settings.mqtt_host, std::regex("[A-Za-z0-9_-]+")))
    {
        Logger.log("MQTT host '%s' did not succeed. Using default '%s'.", settings.mqtt_host, defaultVal);
        strcpy(settings.mqtt_host, defaultVal);
    }

    return settings.mqtt_host;
}

uint16_t MQTTClientClass::getPort(uint16_t defaultVal)
{
    if (!settings.mqtt_port)
    {
        Logger.log("MQTT port '%s' did not succeed. Using default '%s'.", settings.mqtt_port, defaultVal);
        settings.mqtt_port = defaultVal;
    }

    return settings.mqtt_port;
}

char *MQTTClientClass::getUser(const char defaultVal[])
{
    if (!std::regex_match(settings.mqtt_user, std::regex("[A-Za-z0-9_-]+")))
    {
        Logger.log("MQTT user '%s' did not succeed. Using default '%s'.", settings.mqtt_user, defaultVal);
        strcpy(settings.mqtt_user, defaultVal);
    }

    return settings.mqtt_user;
}

char *MQTTClientClass::getPass(const char defaultVal[])
{
    if (!std::regex_match(settings.mqtt_pass, std::regex(".+")))
    {
        Logger.log("MQTT user '%s' did not succeed. Using default '%s'.", settings.mqtt_pass, defaultVal);
        strcpy(settings.mqtt_pass, defaultVal);
    }

    return settings.mqtt_pass;
}

char *MQTTClientClass::getTopic(const char defaultVal[])
{
    if (!std::regex_match(settings.mqtt_topic, std::regex(".+")))
    {
        Logger.log("MQTT topic '%s' did not succeed. Using default '%s'", settings.mqtt_topic, defaultVal);
        strcpy(settings.mqtt_topic, defaultVal);
    }

    return settings.mqtt_topic;
}

unsigned long MQTTClientClass::getPublishPeriod(unsigned long defaultVal)
{
    if (!settings.publish_period)
    {
        Logger.log("MQTT publish period '%lu' did not succeed. Using default '%s'", settings.publish_period, defaultVal);
        settings.publish_period = defaultVal;
    }

    return settings.publish_period;
}

void MQTTClientClass::setHost(const char host[])
{
    strcpy(settings.mqtt_host, host);
}

void MQTTClientClass::setPort(uint16_t port)
{
    settings.mqtt_port = port;
}

void MQTTClientClass::setUser(const char user[])
{
    strcpy(settings.mqtt_user, user);
}

void MQTTClientClass::setPass(const char pass[])
{
    strcpy(settings.mqtt_pass, pass);
}

void MQTTClientClass::setTopic(const char topic[])
{
    strcpy(settings.mqtt_topic, topic);
}

void MQTTClientClass::setPublishPeriod(unsigned long period)
{
    settings.publish_period = period;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
MQTTClientClass MQTTClient;
#endif