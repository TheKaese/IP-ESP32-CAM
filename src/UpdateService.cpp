#include <UpdateService.h>

void UpdateClientClass::begin(const char *vers, const char *plat)
{
    strcpy(firmware_v, vers);
    strcpy(plat_v, plat);

    readConfig();
}

// Check update task
void UpdateClientClass::checkForupdate()
{
    Logger.log("Looking for new firmware...\r\n");

    if (strcmp(getHost(), "") == 0)
    {
        return;
    }

    WiFiClient client;

    IPAddress updateHostIP;
    WiFi.hostByName(getHost(), updateHostIP);

    if ((uint32_t)updateHostIP == 0)
    {
        Logger.log("Unable to locate Update Host: %s", getHost());
        return;
    }
    char baseUrl[64];
    sprintf(baseUrl, "%s://%d.%d.%d.%d:%d/", getProtocol(), updateHostIP[0], updateHostIP[1], updateHostIP[2], updateHostIP[3], getPort());

    //BIN
    String bin_url = String(baseUrl);
    bin_url.concat(getTopic());
    bin_url.concat("/bin");
    bin_url.concat("?version=" + String(firmware_v));
    bin_url.concat("&platform=" + String(plat_v));

    Logger.log("Checking URL: " + String(bin_url));

    t_httpUpdate_return ret = httpUpdate.update(client, bin_url);

    switch (ret)
    {
    default:
    case HTTP_UPDATE_FAILED:
        Logger.log("HTTP_UPDATE_FAILED Error (" + String(httpUpdate.getLastError()) + "): " + httpUpdate.getLastErrorString().c_str());
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Logger.log("HTTP_UPDATE_NO_UPDATES");
        break;
    case HTTP_UPDATE_OK:
        Logger.log("HTTP_UPDATE_OK");
        break;
    }

    //SPIFFS
    String www_url = String(baseUrl);
    bin_url.concat(getTopic());
    www_url.concat("/spiffs");
    www_url.concat("?version=" + String(firmware_v));
    www_url.concat("&platform=" + String(plat_v));

    Logger.log("Checking URL: " + String(www_url));

    ret = httpUpdate.updateSpiffs(client, www_url);

    switch (ret)
    {
    default:
    case HTTP_UPDATE_FAILED:
        Logger.log("HTTP_UPDATE_FAILED Error (" + String(httpUpdate.getLastError()) + "): " + httpUpdate.getLastErrorString().c_str());
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Logger.log("HTTP_UPDATE_NO_UPDATES");
        break;
    case HTTP_UPDATE_OK:
        Logger.log("HTTP_UPDATE_OK");
        break;
    }
}

void UpdateClientClass::resetConfig()
{

    try
    {
        setHost("");
        setUser("");
        setPass("");
        setPort(5000);
        setTopic("");
        setProtocol("http");

        Logger.log("Saving empty config");
        saveConfig();
    }
    catch (const std::exception &e)
    {
        Logger.log(e.what());
    }
}

void UpdateClientClass::readConfig()
{
    Logger.log("Reading Update configuration.");

    Preferences preferences;
    preferences.begin(UPDATE_PREF_KEY, true);

    setPort(preferences.getUInt(UPDATE_PREF_KEY_UPDATE_PORT, 5000));
    setHost(preferences.getString(UPDATE_PREF_KEY_UPDATE_HOST, "").c_str());
    setUser(preferences.getString(UPDATE_PREF_KEY_UPDATE_USER, "").c_str());
    setPass(preferences.getString(UPDATE_PREF_KEY_UPDATE_PASS, "").c_str());
    setTopic(preferences.getString(UPDATE_PREF_KEY_UPDATE_TOPIC, "").c_str());

    preferences.end();
}

void UpdateClientClass::saveConfig()
{
    Logger.log("Saving Update configuration.");

    Preferences preferences;
    preferences.begin(UPDATE_PREF_KEY, false);

    preferences.putUInt(UPDATE_PREF_KEY_UPDATE_PORT, getPort());
    preferences.putString(UPDATE_PREF_KEY_UPDATE_HOST, getHost());
    preferences.putString(UPDATE_PREF_KEY_UPDATE_TOPIC, getTopic());
    preferences.putString(UPDATE_PREF_KEY_UPDATE_USER, getUser());
    preferences.putString(UPDATE_PREF_KEY_UPDATE_PASS, getPass());

    preferences.end();
}

void UpdateClientClass::reconnect()
{
    Logger.log("Reconnecting to Update server.");
    checkForupdate();
}

char *UpdateClientClass::getHost(const char defaultVal[])
{
    if (!std::regex_match(settings.update_host, std::regex("[A-Z:a-z0-9_-]+")))
    {
        Logger.log("Update host '%s' did not succeed. Using default '%s'.", settings.update_host, defaultVal);
        strcpy(settings.update_host, defaultVal);
    }

    return settings.update_host;
}

uint16_t UpdateClientClass::getPort(uint16_t defaultVal)
{
    if (!settings.update_port)
    {
        Logger.log("Update port '%s' did not succeed. Using default '%s'.", settings.update_port, defaultVal);
        settings.update_port = defaultVal;
    }

    return settings.update_port;
}

char *UpdateClientClass::getUser(const char defaultVal[])
{
    if (!std::regex_match(settings.update_user, std::regex("[A-Za-z0-9_-]+")))
    {
        Logger.log("Update user '%s' did not succeed. Using default '%s'.", settings.update_user, defaultVal);
        strcpy(settings.update_user, defaultVal);
    }

    return settings.update_user;
}

char *UpdateClientClass::getPass(const char defaultVal[])
{
    if (!std::regex_match(settings.update_pass, std::regex(".+")))
    {
        Logger.log("Update user '%s' did not succeed. Using default '%s'.", settings.update_pass, defaultVal);
        strcpy(settings.update_pass, defaultVal);
    }

    return settings.update_pass;
}

char *UpdateClientClass::getTopic(const char defaultVal[])
{
    if (!std::regex_match(settings.update_topic, std::regex(".+")))
    {
        Logger.log("Update topic '%s' did not succeed. Using default '%s'", settings.update_topic, defaultVal);
        strcpy(settings.update_topic, defaultVal);
    }

    return settings.update_topic;
}

char *UpdateClientClass::getProtocol(const char defaultVal[])
{
    if (!std::regex_match(settings.update_protocol, std::regex(".+")))
    {
        Logger.log("Update protocol '%s' did not succeed. Using default '%s'", settings.update_protocol, defaultVal);
        strcpy(settings.update_protocol, defaultVal);
    }

    return settings.update_protocol;
}

void UpdateClientClass::setHost(const char host[])
{
    strcpy(settings.update_host, host);
}

void UpdateClientClass::setPort(uint16_t port)
{
    settings.update_port = port;
}

void UpdateClientClass::setUser(const char user[])
{
    strcpy(settings.update_user, user);
}

void UpdateClientClass::setPass(const char pass[])
{
    strcpy(settings.update_pass, pass);
}

void UpdateClientClass::setProtocol(const char protocol[])
{
    strcpy(settings.update_protocol, protocol);
}

void UpdateClientClass::setTopic(const char topic[])
{
    strcpy(settings.update_topic, topic);
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
UpdateClientClass UpdateClient;
#endif
