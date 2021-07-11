#include <ESP.h>
#include <driver/rtc_io.h>
#include <NetworkClient.h>
#include "MQTTClient.h"
#include "UpdateService.h"
#include <CameraClient.h>
#include <ESPAsyncWebServer.h>
#include <Storage.h>
#include <Logger.h>
#include "AsyncJson.h"
#include <ArduinoJson.h>
#include <HTTPUpdate.h>

extern const char MQTT_CERTIFICATE_FILENAME[];

extern void setModuleName(const char *);
extern char *getModuleName();
extern char *setPlatform(const char *);
extern char *getPlatform();
extern char *getVersion();

#define HTTP_PORT 80
int httpPort = HTTP_PORT;
char httpURL[64] = {"Undefined"};

AsyncWebServer appServer(httpPort);

void options_handler(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(204);
    response->addHeader("Access-Control-Allow-Methods", "*");
    response->addHeader("Access-Control-Allow-Headers", "*");
    request->send(response);
}

void file_upload_handler(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

    try
    {
        String spiffs_filename = request->arg("type");
        spiffs_filename.replace('_', '.');

        if (!index)
        {
            Logger.log((String) "Upload Started: " + filename + "(" + spiffs_filename + ")");
            request->_tempFile = SPIFFS.open("/" + spiffs_filename, "w");
        }
        if (len)
        {
            request->_tempFile.write(data, len);
        }
        if (final)
        {
            Logger.log((String) "Upload Complete: " + filename + " (" + spiffs_filename + "), " + index + len);
            request->_tempFile.close();
            request->send(200, "text/plain", "File Uploaded !");
        }
    }
    catch (const std::exception &e)
    {
        Logger.log(e.what());
    }
}

void firmware_handler(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

    try
    {
        String type = request->arg("type");
        size_t contentLength = request->contentLength();

        if (!index)
        {

            int command = U_FLASH;
            if (strcmp("SPIFFS", type.c_str()) == 0)
            {
                command = U_SPIFFS;
            }

            Logger.log("Beginning update... (%d)", contentLength);
            if (Update.begin(contentLength, command))
            {
                Update.onProgress([](unsigned int progress, unsigned int total) {
                    Logger.log("Progress: %u%%\r", (progress / (total / 100)));
                });
            }
            else
            {
                Logger.log("Failed update...");
                request->send(500, "text/plain");
            }
        }
        if (len)
        {
            Update.write(data, len);
        }
        if (final)
        {

            if (Update.hasError())
            {
                Logger.log((String)Update.getError());
            }

            request->_tempFile.close();
            request->send(200, "text/plain", "Update Complete");
        }
    }
    catch (const std::exception &e)
    {
        Logger.log("Upload failed: %s", e.what());
        request->send(500, "text/plain");
        if (request->_tempFile)
        {
            request->_tempFile.close();
        }
    }
}

void module_info_handler(AsyncWebServerRequest *request)
{
    Logger.log("module info requested");

    char json_response[4096];

    char *p = json_response;
    *p++ = '{';

    //WiFi
    p += sprintf(p, "\"module_name\":\"%s\"", getPlatform());
    p += sprintf(p, ",\"version\":\"%s\"", getVersion());

    *p++ = '}';
    *p++ = 0;

    request->send(200, "application/json", json_response);
}

void camera_update_handler(AsyncWebServerRequest *request)
{
    Logger.log("Camera command received!");

    String variable = request->arg("var").c_str();
    String valueStr = request->arg("val").c_str();

    Logger.log("'%s' requested. Value: %s", variable.c_str(), valueStr.c_str());

    int res = 0;
    if (variable.compareTo("resolution") == 0)
    {
        res = CameraClient.setResolution(valueStr);
    }
    else if (variable.compareTo("quality") == 0)
    {
        res = CameraClient.setQuality(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("contrast") == 0)
    {
        res = CameraClient.setContrast(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("brightness") == 0)
    {
        res = CameraClient.setBrightness(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("saturation") == 0)
    {
        res = CameraClient.setSaturation(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("sharpness") == 0)
    {
        res = CameraClient.setSharpness(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("gainceiling") == 0)
    {
        res = CameraClient.setGainCieling(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("colorbar") == 0)
    {
        res = CameraClient.setColorBar(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("awb") == 0)
    {
        res = CameraClient.setAWB(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("agc") == 0)
    {
        res = CameraClient.setAGC(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("aec") == 0)
    {
        res = CameraClient.setAEC(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("aec_sensor") == 0)
    {
        res = CameraClient.setAEC(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("hflip") == 0)
    {
        res = CameraClient.setHFlip(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("vflip") == 0)
    {
        res = CameraClient.setVFlip(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("awb_gain") == 0)
    {
        res = CameraClient.setAWBGain(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("agc_gain") == 0)
    {
        res = CameraClient.setAGCGain(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("aec_value") == 0)
    {
        res = CameraClient.setAECValue(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("aec_dsp") == 0)
    {
        res = CameraClient.setAEC2(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("dcw") == 0)
    {
        res = CameraClient.setDCW(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("bpc") == 0)
    {
        res = CameraClient.setBPC(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("wpc") == 0)
    {
        res = CameraClient.setWPC(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("raw_gma") == 0)
    {
        res = CameraClient.setRawGMA(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("lens_correction") == 0)
    {
        res = CameraClient.setLensCorrection(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("special_effect") == 0)
    {
        res = CameraClient.setSpecialEffect(valueStr);
    }
    else if (variable.compareTo("wb_mode") == 0)
    {
        res = CameraClient.setWBMode(valueStr);
    }
    else if (variable.compareTo("ae_level") == 0)
    {
        res = CameraClient.setAELevel(atoi(valueStr.c_str()));
    }
    else if (variable.compareTo("rotation") == 0)
    {
        res = CameraClient.setRotation(valueStr);
    }
    else if (variable.compareTo("lamp") == 0)
    {
        int lampVal = constrain(atoi(valueStr.c_str()), 0, 100);
        res = CameraClient.setLamp(lampVal);
    }
    else if (variable.compareTo("save_prefs") == 0)
    {
        CameraClient.saveCameraPreferences();
    }
    else if (variable.compareTo("clear_prefs") == 0)
    {
        CameraClient.removeCameraPreferences();
    }
    else
    {
        res = -1;
    }
    Logger.log("Result %d", res);
    if (res)
    {
        request->send(404, "text/plain", "Invalid parameters.");
        return;
    }
    request->send(200);
    return;
}

void camera_info_handler(AsyncWebServerRequest *request)
{
    if (request->hasArg("var") && request->hasArg("val"))
    {
        camera_update_handler(request);
        return;
    }

    char json_response[1024];

    char *p = json_response;
    *p++ = '{';
    p += sprintf(p, "\"resolution\":\"%s\"", CameraClient.getResolution().c_str());
    p += sprintf(p, ",\"quality\":%u", CameraClient.getQuality());
    p += sprintf(p, ",\"brightness\":%d", CameraClient.getBrightness());
    p += sprintf(p, ",\"contrast\":%d", CameraClient.getContrast());
    p += sprintf(p, ",\"saturation\":%d", CameraClient.getSaturation());
    p += sprintf(p, ",\"sharpness\":%d", CameraClient.getSharpness());
    p += sprintf(p, ",\"special_effect\":\"%s\"", CameraClient.getSpecialEffect().c_str());
    p += sprintf(p, ",\"wb_mode\":\"%s\"", CameraClient.getWBMode().c_str());
    p += sprintf(p, ",\"awb\":%u", CameraClient.getAWB());
    p += sprintf(p, ",\"awb_gain\":%u", CameraClient.getAWBGain());
    p += sprintf(p, ",\"aec\":%u", CameraClient.getAEC());
    p += sprintf(p, ",\"aec2\":%u", CameraClient.getAEC2());
    p += sprintf(p, ",\"ae_level\":%d", CameraClient.getAELevel());
    p += sprintf(p, ",\"aec_value\":%u", CameraClient.getAECValue());
    p += sprintf(p, ",\"agc\":%u", CameraClient.getAGC());
    p += sprintf(p, ",\"agc_gain\":%u", CameraClient.getAGCGain());
    p += sprintf(p, ",\"gain_ceiling\":%u", CameraClient.getGainCieling());
    p += sprintf(p, ",\"bpc\":%u", CameraClient.getBPC());
    p += sprintf(p, ",\"wpc\":%u", CameraClient.getWPC());
    p += sprintf(p, ",\"raw_gma\":%u", CameraClient.getRawGMA());
    p += sprintf(p, ",\"lens_correction\":%u", CameraClient.getLensCorrection());
    p += sprintf(p, ",\"vflip\":%u", CameraClient.getVFlip());
    p += sprintf(p, ",\"hflip\":%u", CameraClient.getHFlip());
    p += sprintf(p, ",\"dcw\":%u", CameraClient.getDCW());
    p += sprintf(p, ",\"colorbar\":%u", CameraClient.getColorBar());
    p += sprintf(p, ",\"lamp\":%d", CameraClient.getLamp());
    p += sprintf(p, ",\"rotation\":\"%s\"", CameraClient.getRotation().c_str());
    *p++ = '}';
    *p++ = 0;

    request->send(200, "application/json", json_response);
}

void settings_update_handler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    Logger.log("Settings command received!");

    DynamicJsonDocument jsonResult(4096);
    if (DeserializationError::Ok == deserializeJson(jsonResult, (const char *)data))
    {
        char buffer[512];
        serializeJsonPretty(jsonResult, buffer);
        Logger.log(buffer);
    }

    if (jsonResult.isNull())
    {
        request->send(400, "text/plain", "Invalid json body");
        return;
    }

    String savetype = jsonResult["type"];

    if (savetype.compareTo("MQTT") == 0)
    {
        Logger.log("Doing MQTT Save");

        MQTTClient.setHost(jsonResult["mqtt_host"]);
        MQTTClient.setPort(jsonResult["mqtt_port"]);
        MQTTClient.setTopic(jsonResult["mqtt_topic"]);
        MQTTClient.setPublishPeriod(jsonResult["mqtt_frequency"]);

        if (strcmp("", jsonResult["mqtt_pass"]) != 0)
        {
            MQTTClient.setUser(jsonResult["mqtt_user"]);
            MQTTClient.setPass(jsonResult["mqtt_pass"]);
        }

        MQTTClient.saveConfig();
        MQTTClient.reconnect();
    }
    else if (savetype.compareTo("Update") == 0)
    {
        Logger.log("Doing Update Save");

        UpdateClient.setProtocol(jsonResult["update_proto"]);
        UpdateClient.setHost(jsonResult["update_host"]);
        UpdateClient.setPort(jsonResult["update_port"]);
        UpdateClient.setTopic(jsonResult["update_topic"]);

        if (strcmp("", jsonResult["update_pass"]) != 0)
        {
            UpdateClient.setUser(jsonResult["update_user"]);
            UpdateClient.setPass(jsonResult["update_pass"]);
        }

        UpdateClient.saveConfig();
        UpdateClient.reconnect();
    }
    else if (savetype.compareTo("Platform") == 0)
    {
        Logger.log("Doing Platform Save");

        setModuleName(jsonResult["module_name"]);
        setPlatform(jsonResult["plat_platform"]);
    }
    else if (savetype.compareTo("WiFi") == 0)
    {
        Logger.log("Doing WiFi Save");

        if (strcmp("", jsonResult["pass"]) != 0)
        {
            NetworkClient.updateConfig(getModuleName(), jsonResult["ssid"], jsonResult["pass"]);
        }
    }
    else if (savetype.compareTo("Clear_WiFi") == 0)
    {
        Logger.log("Clearing WiFi Settings");
        NetworkClient.resetConfig();
    }
    else if (savetype.compareTo("Clear_MQTT") == 0)
    {
        Logger.log("Clearing MQTT Settings");
        // MQTTClient.resetConfig();
    }
    else if (savetype.compareTo("Clear_Update") == 0)
    {
        Logger.log("Clearing Update Settings");
        UpdateClient.resetConfig();
    }
    else
    {
        request->send(404, "text/plain", "Invalid type.");
        return;
    }

    request->send(200);
    return;
}

void settings_info_handler(AsyncWebServerRequest *request)
{
    Logger.log("settings requested");

    char json_response[4096];

    char *p = json_response;
    *p++ = '{';

    //WiFi
    p += sprintf(p, "\"wifi_ssid\":\"%s\"", NetworkClient.getWiFiSSID().c_str());

    //MQTT
    p += sprintf(p, ",\"mqtt_host\":\"%s\"", MQTTClient.getHost());
    p += sprintf(p, ",\"mqtt_port\":%u", MQTTClient.getPort());
    p += sprintf(p, ",\"mqtt_user\":\"%s\"", MQTTClient.getUser());
    p += sprintf(p, ",\"mqtt_topic\":\"%s\"", MQTTClient.getTopic());
    p += sprintf(p, ",\"mqtt_frequency\":%lu", MQTTClient.getPublishPeriod());

    //Update
    p += sprintf(p, ",\"update_proto\":\"%s\"", UpdateClient.getProtocol());
    p += sprintf(p, ",\"update_host\":\"%s\"", UpdateClient.getHost());
    p += sprintf(p, ",\"update_port\":%u", UpdateClient.getPort());
    p += sprintf(p, ",\"update_user\":\"%s\"", UpdateClient.getUser());
    p += sprintf(p, ",\"update_topic\":\"%s\"", UpdateClient.getTopic());

    //Platform
    p += sprintf(p, ",\"module_name\":\"%s\"", getModuleName());
    p += sprintf(p, ",\"plat_platform\":\"%s\"", getPlatform());
    p += sprintf(p, ",\"plat_version\":\"%s\"", getVersion());
    p += sprintf(p, ",\"update_port\":%u", UpdateClient.getPort());
    p += sprintf(p, ",\"update_user\":\"%s\"", UpdateClient.getUser());
    p += sprintf(p, ",\"update_topic\":\"%s\"", UpdateClient.getTopic());

    *p++ = '}';
    *p++ = 0;

    request->send(200, "application/json", json_response);
}

void capture_handler(AsyncWebServerRequest *request)
{
    // camera_fb_t *fb = NULL;

    // char dbg[256];
    // sprintf(dbg, "Capture requested!");
    // Logger.log(dbg);

    // int64_t fr_start = esp_timer_get_time();

    // fb = esp_camera_fb_get();
    // if (!fb)
    // {
    //     sprintf(dbg, "Camera capture failed!");
    //     Logger.log(dbg);

    //     request->send(500, "text/plain", "Camera Capture Failed");
    //     return;
    // }

    // size_t fb_len = 0;
    // if (fb->format == PIXFORMAT_JPEG)
    // {
    //     fb_len = fb->len;
    //     request->send(200, "image/jpeg", (const char *)fb->buf);
    // }
    // else
    // {
    //     request->send(500, "text/plain", "Camera Capture Failed");
    // }
    // esp_camera_fb_return(fb);
    // int64_t fr_end = esp_timer_get_time();

    // sprintf(dbg, "JPG: %uB %ums\r\n", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start) / 1000));
    // Logger.log(dbg);
}

void favicon_16x16_handler(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/favicon-16x16.png", "image/png", false);
}

void favicon_32x32_handler(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/favicon-32x32.png", "image/png", false);
}

void favicon_ico_handler(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/favicon.ico", "image/png", false);
}

void logo_svg_handler(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/logo.svg", "image/svg+xml", false);
}

void index_handler(AsyncWebServerRequest *request)
{
    Logger.log("Index page requested");
    request->send(SPIFFS, "/index.html", String(), false);
    Logger.log("Index page sent");
}

void index_style_handler(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/index.css", String(), false);
}

void settings_handler(AsyncWebServerRequest *request)
{
    Logger.log("Settings page requested");
    request->send(SPIFFS, "/settings.html", String(), false);
    Logger.log("Settings page sent");
}

void settings_style_handler(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/settings.css", String(), false);
}

void startCameraServer()
{
    Logger.log("Starting web server on port: '%d'", httpPort);

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    appServer.on("/", HTTP_ANY, index_handler);
    appServer.on("/index", HTTP_ANY, index_handler);
    appServer.on("/index.css", HTTP_ANY, index_style_handler);
    appServer.on("/settings", HTTP_ANY, settings_handler);
    appServer.on("/settings.css", HTTP_ANY, settings_style_handler);

    appServer.on("/logo.svg", HTTP_GET, logo_svg_handler);
    appServer.on("/favicon.ico", HTTP_GET, favicon_ico_handler);
    appServer.on("/favicon-16x16.png", HTTP_GET, favicon_16x16_handler);
    appServer.on("/favicon-32x32.png", HTTP_GET, favicon_32x32_handler);

    appServer.on("/api/module", HTTP_GET, module_info_handler);

    appServer.on("/api/camera", HTTP_OPTIONS, options_handler);
    appServer.on("/api/camera", HTTP_GET, camera_info_handler);

    appServer.on("/api/settings", HTTP_OPTIONS, options_handler);
    appServer.on("/api/settings", HTTP_GET, settings_info_handler);
    appServer.on(
        "/api/settings", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, settings_update_handler);

    appServer.on("/api/capture", HTTP_GET, capture_handler);

    appServer.on(
        "/file", HTTP_POST, [](AsyncWebServerRequest *request) {}, file_upload_handler);
    appServer.on(
        "/firmware", HTTP_POST, [](AsyncWebServerRequest *request) {}, firmware_handler);

    try
    {
        appServer.begin();
    }
    catch (const std::exception &e)
    {
        Logger.log(e.what());
    }

    IPAddress espIP = NetworkClient.getIPAddress();
    sprintf(httpURL, "http://%d.%d.%d.%d:%d/", espIP[0], espIP[1], espIP[2], espIP[3], httpPort);

    Logger.log("Camera App Ready! Use '%s' to connect", httpURL);
}