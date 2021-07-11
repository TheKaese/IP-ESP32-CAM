#ifndef CAMERA_CLIENT_H
#define CAMERA_CLIENT_H

#include <Arduino.h>
#include <Logger.h>
#include "esp_camera.h"
#include "camera_pins.h"
#include <Preferences.h>

// Illumination LAMP/LED
#define LAMP_CHANNEL 7
#define PWM_FREQUENCY 50000 // 50K pwm frequency
#define PWM_RESOLUTION 9    // duty cycle bit range

const char CAMERA_PREF_KEY[] = "CAMERA";
const char CAMERA_PREF_KEY_RESOLUTION[] = "resolution";
const char CAMERA_PREF_KEY_QUALITY[] = "quality";
const char CAMERA_PREF_KEY_BRIGHTNESS []= "brightness";
const char CAMERA_PREF_KEY_CONTRAST[] = "contrast";
const char CAMERA_PREF_KEY_SATURATION []= "saturation";
const char CAMERA_PREF_KEY_SPECIAL_EFFECT[] = "special_effect";
const char CAMERA_PREF_KEY_WB_MODE[] = "wb_mode";
const char CAMERA_PREF_KEY_AWB[] = "awb";
const char CAMERA_PREF_KEY_AWB_GAIN[] = "awb_gain";
const char CAMERA_PREF_KEY_AEC[] = "aec";
const char CAMERA_PREF_KEY_AEC2[] = "aec2";
const char CAMERA_PREF_KEY_AE_LEVEL[] = "ae_level";
const char CAMERA_PREF_KEY_AEC_VALUE[] = "aec_value";
const char CAMERA_PREF_KEY_AGC[] = "agc";
const char CAMERA_PREF_KEY_AGC_GAIN[] = "agc_gain";
const char CAMERA_PREF_KEY_GAINCEILING[] = "gainceiling";
const char CAMERA_PREF_KEY_BPC[] = "bpc";
const char CAMERA_PREF_KEY_WPC[] = "wpc";
const char CAMERA_PREF_KEY_RAW_GMA[] = "raw_gma";
const char CAMERA_PREF_KEY_LENS_CORRECTION[] = "lens_correction";
const char CAMERA_PREF_KEY_VFLIP[] = "vflip";
const char CAMERA_PREF_KEY_HFLIP[] = "hflip";
const char CAMERA_PREF_KEY_DCW[] = "dcw";
const char CAMERA_PREF_KEY_COLORBAR[] = "colorbar";
const char CAMERA_PREF_KEY_LAMP[] = "lamp";
const char CAMERA_PREF_KEY_ROTATION[] = "rotation";

class CameraClass
{
public:
    CameraClass();
    ~CameraClass();

    void begin();

    String getResolution();
    int setResolution(String newVal);

    uint8_t getLamp();
    int setLamp(uint8_t newVal);

    String getRotation();
    int setRotation(String newVal);
    
    uint8_t getQuality();
    int setQuality(uint8_t newVal);
    
    uint8_t getHFlip();
    int setHFlip(uint8_t newVal);

    uint8_t getVFlip();
    int setVFlip(uint8_t newVal);

    uint8_t getBrightness();
    int setBrightness(uint8_t newVal);

    uint8_t getContrast();
    int setContrast(uint8_t newVal);

    uint8_t getSharpness();
    int setSharpness(uint8_t newVal);

    uint8_t getSaturation();
    int setSaturation(uint8_t newVal);

    String getSpecialEffect();
    int setSpecialEffect(String newVal);
    
    String getWBMode();
    int setWBMode(String newVal);
    
    uint8_t getAWB();
    int setAWB(uint8_t newVal);
    
    uint8_t getAWBGain();
    int setAWBGain(uint8_t newVal);

    uint8_t getAEC();
    int setAEC(uint8_t newVal);

    uint8_t getAECValue();
    int setAECValue(uint8_t newVal);

    uint8_t getAEC2();
    int setAEC2(uint8_t newVal);

    uint8_t getAELevel();
    int setAELevel(uint8_t newVal);

    uint8_t getAGC();
    int setAGC(uint8_t newVal);

    uint8_t getAGCGain();
    int setAGCGain(uint8_t newVal);

    uint8_t getGainCieling();
    int setGainCieling(uint8_t newVal);

    uint8_t getBPC();
    int setBPC(uint8_t newVal);

    uint8_t getWPC();
    int setWPC(uint8_t newVal);

    uint8_t getRawGMA();
    int setRawGMA(uint8_t newVal);

    uint8_t getLensCorrection();
    int setLensCorrection(uint8_t newVal);

    uint8_t getDCW();
    int setDCW(uint8_t newVal);

    uint8_t getColorBar();
    int setColorBar(uint8_t newVal);

    void saveCameraPreferences();
    String getCameraPreferences();
    void removeCameraPreferences();

private:
    sensor_t *espCamSensor;

    void loadCameraPreferences();
    
    uint8_t lampVal = 0; 
    uint8_t rotation = 0;

};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
extern CameraClass CameraClient;
#endif

#endif
