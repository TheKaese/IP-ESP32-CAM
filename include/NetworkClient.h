#ifndef WiFiClient_h
#define WiFiClient_h

#include <WiFiManager.h>
#include <Logger.h>
#include <ESPmDNS.h>

class NetworkClientClass
{
public:
    NetworkClientClass();
    ~NetworkClientClass();

    void startWiFi(const char *);
    void updateConfig(const char*, const char*, const char*);
    void resetConfig();
    IPAddress getIPAddress();
    wl_status_t getWiFiStatus();
    String getWiFiSSID();

protected:
    
    WiFiManager wifiManager;
    IPAddress espIP;
    IPAddress espSubnet;
    IPAddress espGateway;

    static void configModeCallback(WiFiManager *myWiFiManager);
    static void saveConfigCallback();

};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
extern NetworkClientClass NetworkClient;
#endif

#endif