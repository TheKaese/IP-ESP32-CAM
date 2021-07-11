#include <NetworkClient.h>

NetworkClientClass::NetworkClientClass() {}
NetworkClientClass::~NetworkClientClass() {}

void NetworkClientClass::startWiFi(const char *module_name)
{
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setPreSaveConfigCallback(saveConfigCallback);
    wifiManager.setConfigPortalTimeout(120);
    wifiManager.setConnectTimeout(30);
    wifiManager.setCountry("US");

    wifiManager.setCleanConnect(true);
    wifiManager.setWiFiAutoReconnect(true);

    if (!wifiManager.autoConnect(module_name))
    {
        ESP.restart();
    }

    wifiManager.setHostname(module_name);

    espIP = WiFi.localIP();
    espSubnet = WiFi.subnetMask();
    espGateway = WiFi.gatewayIP();

    if (!MDNS.begin(module_name))
    {
        Logger.log("Error setting up MDNS responder!");
    }

    Logger.log("Wifi Started");
}

void NetworkClientClass::updateConfig(const char *module_name, const char *ssid, const char *pass)
{
    Logger.log("Updating WiFi config");

    WiFi.disconnect(true);

    yield();
    delay(100);

    WiFi.begin(ssid, pass, false);
    startWiFi(module_name);
}

void NetworkClientClass::resetConfig()
{
    wifiManager.resetSettings();

    Logger.log("WiFi Connected, disconnecting..");
    WiFi.disconnect(true);

}

void NetworkClientClass::configModeCallback(WiFiManager *myWiFiManager)
{
    Logger.log("Entered config mode. SSID '%s' AP: '%s'\r\n",
               myWiFiManager->getConfigPortalSSID().c_str(),
               WiFi.softAPIP().toString().c_str());
}

//callback notifying us of the need to save config
void NetworkClientClass::saveConfigCallback()
{
    Logger.log("Updated config to be saved");
}

wl_status_t NetworkClientClass::getWiFiStatus()
{
    return WiFi.status();
}

IPAddress NetworkClientClass::getIPAddress()
{
    return espIP;
}

String NetworkClientClass::getWiFiSSID()
{
    return wifiManager.getWiFiSSID();
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
NetworkClientClass NetworkClient;
#endif
