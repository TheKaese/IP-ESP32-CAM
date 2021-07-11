#include <rom/rtc.h>
#include <Arduino.h>
#include <Storage.h>
#include <Logger.h>
#include <CameraClient.h>
#include <MQTTClient.h>
#include <UpdateService.h>
#include <NetworkClient.h>

extern void startCameraServer();
extern void startStreamServer();

const char *VERSION = "V2.0.2";
char MODULE_NAME[256];
char PLATFORM[256];
const char *MQTT_JSON = "{ \
\"ClientId\":\"%s\", \
\"CameraName\":\"%s\", \
\"Millis\":\"%lu\", \
\"ActiveClients\":%d \
}";

// Notification LED
void flashLED(int flashtime)
{
#ifdef LED_PIN                      // If we have it; flash it.
    digitalWrite(LED_PIN, LED_ON);  // On at full power.
    delay(flashtime);               // delay
    digitalWrite(LED_PIN, LED_OFF); // turn Off
#else
    return; // No notifcation LED, do nothing, no delay
#endif
}

void setModuleName(const char *new_name)
{
    strcpy(MODULE_NAME, new_name);

    Preferences prefs;
    if (prefs.begin("module", false))
    {
        prefs.putString("module_name", MODULE_NAME);
        Logger.log("Succefully changed MODULE_NAME to %s", MODULE_NAME);
    }
    prefs.end();
}

char *getModuleName()
{
    Preferences prefs;
    if (prefs.begin("module", true))
    {
        strcpy(MODULE_NAME, prefs.getString("module_name").c_str());
    }
    prefs.end();

    //Default if empty
    if (strcmp(MODULE_NAME, "") == 0)
    {
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8)
        {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        sprintf(MODULE_NAME, "ESP32IPCam-%u", chipId);
    }

    return MODULE_NAME;
}

void setPlatform(const char *new_plat)
{
    strcpy(PLATFORM, new_plat);

    Preferences prefs;
    if (prefs.begin("module", false))
    {
        prefs.putString("platform", PLATFORM);
        Logger.log("Succefully changed PLATFORM to %s", PLATFORM);
    }
    prefs.end();
}

char *getPlatform()
{
    Preferences prefs;
    if (prefs.begin("module", true))
    {
        strcpy(PLATFORM, prefs.getString("platform").c_str());
    }
    prefs.end();

    return PLATFORM;
}

const char *getVersion()
{
    return VERSION;
}

void noWiFiCB(void *pvParameters)
{

    while (true)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            vTaskDelete(NULL);
        }

        flashLED(500);
        vTaskDelay(1000);
    }
}
void print_reset_reason(RESET_REASON reason)
{
    switch (reason)
    {
    case 1:
        Serial.println("POWERON_RESET");
        break; /**<1, Vbat power on reset*/
    case 3:
        Serial.println("SW_RESET");
        break; /**<3, Software reset digital core*/
    case 4:
        Serial.println("OWDT_RESET");
        break; /**<4, Legacy watch dog reset digital core*/
    case 5:
        Serial.println("DEEPSLEEP_RESET");
        break; /**<5, Deep Sleep reset digital core*/
    case 6:
        Serial.println("SDIO_RESET");
        break; /**<6, Reset by SLC module, reset digital core*/
    case 7:
        Serial.println("TG0WDT_SYS_RESET");
        break; /**<7, Timer Group0 Watch dog reset digital core*/
    case 8:
        Serial.println("TG1WDT_SYS_RESET");
        break; /**<8, Timer Group1 Watch dog reset digital core*/
    case 9:
        Serial.println("RTCWDT_SYS_RESET");
        break; /**<9, RTC Watch dog Reset digital core*/
    case 10:
        Serial.println("INTRUSION_RESET");
        break; /**<10, Instrusion tested to reset CPU*/
    case 11:
        Serial.println("TGWDT_CPU_RESET");
        break; /**<11, Time Group reset CPU*/
    case 12:
        Serial.println("SW_CPU_RESET");
        break; /**<12, Software reset CPU*/
    case 13:
        Serial.println("RTCWDT_CPU_RESET");
        break; /**<13, RTC Watch dog Reset CPU*/
    case 14:
        Serial.println("EXT_CPU_RESET");
        break; /**<14, for APP CPU, reseted by PRO CPU*/
    case 15:
        Serial.println("RTCWDT_BROWN_OUT_RESET");
        break; /**<15, Reset when the vdd voltage is not stable*/
    case 16:
        Serial.println("RTCWDT_RTC_RESET");
        break; /**<16, RTC Watch dog reset digital core and rtc module*/
    default:
        Serial.println("NO_MEAN");
    }
}
void setup()
{
    Logger.begin();

    FileSystemInit();

    delay(500);

    getModuleName();
    getPlatform();

    delay(500);

    CameraClient.begin();

    delay(500);

    xTaskCreate(noWiFiCB, "nowifi", 1024, NULL, 10, NULL);
    while (WiFi.status() != WL_CONNECTED)
    {
        NetworkClient.startWiFi(MODULE_NAME);
        for (int i = 0; (WiFi.status() != WL_CONNECTED) && (i < 10); i++)
        {
            delay(250);
        }
        delay(2000);
    }

    // Now we have a network we can start the two handlers for the UI and Stream.
    startCameraServer();

    delay(500);

    startStreamServer();

    delay(500);

    UpdateClient.begin(VERSION, PLATFORM);

    delay(500);

    MQTTClient.begin(MQTT_JSON);
}

void loop()
{
    delay(250);

    // Handle WiFi
    static bool wifiDisconnectWarning = false;
    if (NetworkClient.getWiFiStatus() != WL_CONNECTED)
    {
        // disconnected; attempt to reconnect
        if (!wifiDisconnectWarning)
        {
            // Tell the user if we just disconnected
            WiFi.disconnect(); // ensures disconnect is complete, wifi scan cleared
            Serial.println("WiFi disconnected, retrying");
            wifiDisconnectWarning = true;
        }

        NetworkClient.startWiFi(MODULE_NAME);

        //Restart the loop
        return;
    }

    // We are connected, wait a bit and re-check
    if (wifiDisconnectWarning)
    {
        // Tell the user if we have just reconnected
        Serial.println("WiFi reconnected");
        wifiDisconnectWarning = false;
    }

    //Handle MQTT
    MQTTClient.loop();

    static unsigned long lastMQTTPublish = 0;
    static unsigned long lastUpdateCheck = 0;

    unsigned long now = millis();
    unsigned long timeSinceLastPub = now - lastMQTTPublish;

    if (timeSinceLastPub > (MQTTClient.getPublishPeriod() * 1000) || lastMQTTPublish == 0)
    {
        int activeClients = 0; //Stream.getActiveClients();
        char activeClientsStr[10];
        sprintf(activeClientsStr, "%d", activeClients);

        StaticJsonDocument<200> doc;
        doc[KEY_CAM_NAME] = getModuleName();
        doc[KEY_ACTIVE_CLIENTS] = activeClientsStr;

        MQTTClient.publish(doc);

        lastMQTTPublish = millis();
    }

    if (timeSinceLastPub > (600 * 1000) || lastUpdateCheck == 0)
    {
        lastUpdateCheck = millis();
        UpdateClient.checkForupdate();
    }
}
