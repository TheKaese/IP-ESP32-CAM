#include <CameraClient.h>

CameraClass::CameraClass(){};
CameraClass::~CameraClass(){};

void CameraClass::begin()
{
    // Create camera config structure; and populate with hardware and other defaults
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    //init with highest supported specs to pre-allocate large buffers
    if (psramFound())
    {
        char dbg[] = "psram found! Using UXGA";
        Logger.log(dbg);

        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_XGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err == ESP_OK)
    {
        Logger.log("Camera init succeeded");
    }
    else
    {
        delay(100); // need a delay here or the next serial o/p gets missed

        Logger.log("Halted: Camera sensor failed to initialise. Will reboot to try again in 10s");

        delay(10000);
        ESP.restart();
    }

    espCamSensor = esp_camera_sensor_get();

    // Dump camera module, warn for unsupported modules.
    switch (espCamSensor->id.PID)
    {
    case OV9650_PID:;
        Logger.log("WARNING: OV9650 camera module is not properly supported, will fallback to OV2640 operation");
        break;
    case OV7725_PID:
        Logger.log("WARNING: OV7725 camera module is not properly supported, will fallback to OV2640 operation");
        break;
    case OV2640_PID:
        Logger.log("OV2640 camera module detected");
        break;
    case OV3660_PID:
        Logger.log("OV3660 camera module detected");
        break;
    default:
        Logger.log("WARNING: Camera module is unknown and not properly supported, will fallback to OV2640 operation");
    }

    // OV3660 initial sensors are flipped vertically and colors are a bit saturated
    if (espCamSensor->id.PID == OV3660_PID)
    {
        setVFlip(1);       //flip it back
        setBrightness(1);  //up the blightness just a bit
        setSaturation(-2); //lower the saturation
    }
    espCamSensor->set_framesize(espCamSensor, FRAMESIZE_SVGA);

// M5 Stack Wide has special needs
#if defined(CAMERA_MODEL_M5STACK_WIDE)
    setVFlip(1);
    setHFlip(1);
#endif

#if defined(LED_PIN) // If we have a notification LED, set it to output
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LED_ON);
#endif

    ledcSetup(LAMP_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION); // configure LED PWM channel
    setLamp(lampVal);                                       // set default value
    ledcAttachPin(LAMP_PIN, LAMP_CHANNEL);                  // attach the GPIO pin to the channel

    loadCameraPreferences();
};

uint8_t CameraClass::getLamp()
{
    return lampVal;
};

// Lamp Control
int CameraClass::setLamp(uint8_t newVal)
{
    if (newVal == -1)
    {
        return -1;
    }

    // Apply a logarithmic function to the scale.
    uint8_t pwmMax = round(pow(2, PWM_RESOLUTION) - 1.00);
    lampVal = round((pow(2, (1 + (newVal * 0.02))) - 2.00) / 6.00 * pwmMax);
    ledcWrite(LAMP_CHANNEL, lampVal);
    return NULL;
};

String CameraClass::getResolution()
{
    framesize_t framesize = espCamSensor->status.framesize;

    switch (framesize)
    {
    case FRAMESIZE_UXGA:
        return "UXGA(1600x1200)";
    case FRAMESIZE_SXGA:
        return "SXGA(1280x1024)";
    case FRAMESIZE_XGA:
        return "XGA(1024x768)";
    case FRAMESIZE_SVGA:
        return "SVGA(800x600)";
    case FRAMESIZE_VGA:
        return "VGA(640x480)";
    case FRAMESIZE_CIF:
        return "CIF(400x296)";
    case FRAMESIZE_QVGA:
        return "QVGA(320x240)";
    case FRAMESIZE_HQVGA:
        return "HQVGA(240x176)";
    case FRAMESIZE_QQVGA:
        return "QQVGA(160x120)";
    default:
        return "Unknown";
    };
}

// Lamp Control
int CameraClass::setResolution(String resolution)
{
    framesize_t framesize;

    if (espCamSensor->pixformat == PIXFORMAT_JPEG)
    {

        if (String("UXGA(1600x1200)").compareTo(resolution) == 0)
        {
            framesize = FRAMESIZE_UXGA;
        }
        else if (String("SXGA(1280x1024)").compareTo(resolution) == 0)
        {
            framesize = FRAMESIZE_SXGA;
        }
        else if (String("XGA(1024x768)").compareTo(resolution) == 0)
        {
            framesize = FRAMESIZE_XGA;
        }
        else if (String("SVGA(800x600)").compareTo(resolution) == 0)
        {
            framesize = FRAMESIZE_SVGA;
        }
        else if (String("VGA(640x480)").compareTo(resolution) == 0)
        {
            framesize = FRAMESIZE_VGA;
        }
        else if (String("CIF(400x296)").compareTo(resolution) == 0)
        {
            framesize = FRAMESIZE_CIF;
        }
        else if (String("QVGA(320x240)").compareTo(resolution) == 0)
        {
            framesize = FRAMESIZE_QVGA;
        }
        else if (String("HQVGA(240x176)").compareTo(resolution) == 0)
        {
            framesize = FRAMESIZE_HQVGA;
        }
        else if (String("QQVGA(160x120)").compareTo(resolution) == 0)
        {
            framesize = FRAMESIZE_QQVGA;
        }
        else
        {
            return -1;
        }

        Logger.log("framesize %d", framesize);

        return espCamSensor->set_framesize(espCamSensor, framesize);
    }

    return -1;
};

String CameraClass::getRotation()
{
    return "0"; //TODO
};

// Lamp Control
int CameraClass::setRotation(String newVal)
{
    return 0; //TODO
};

uint8_t CameraClass::getQuality()
{
    return espCamSensor->status.quality;
}

int CameraClass::setQuality(uint8_t i)
{
    return espCamSensor->set_quality(espCamSensor, i);
}

uint8_t CameraClass::getHFlip()
{
    return espCamSensor->status.hmirror;
}

int CameraClass::setHFlip(uint8_t i)
{
    return espCamSensor->set_hmirror(espCamSensor, i);
}

uint8_t CameraClass::getVFlip()
{
    return espCamSensor->status.vflip;
}

int CameraClass::setVFlip(uint8_t i)
{
    return espCamSensor->set_vflip(espCamSensor, i);
}

uint8_t CameraClass::getBrightness()
{
    return espCamSensor->status.brightness;
}

int CameraClass::setBrightness(uint8_t i)
{
    return espCamSensor->set_brightness(espCamSensor, i);
}

uint8_t CameraClass::getContrast()
{
    return espCamSensor->status.contrast;
}

int CameraClass::setContrast(uint8_t i)
{
    return espCamSensor->set_contrast(espCamSensor, i);
}

uint8_t CameraClass::getSharpness()
{
    return espCamSensor->status.sharpness;
}

int CameraClass::setSharpness(uint8_t i)
{
    return espCamSensor->set_sharpness(espCamSensor, i);
}

uint8_t CameraClass::getSaturation()
{
    return espCamSensor->status.sharpness;
}

int CameraClass::setSaturation(uint8_t i)
{
    return espCamSensor->set_sharpness(espCamSensor, i);
}

String CameraClass::getSpecialEffect()
{
    switch (espCamSensor->status.special_effect)
    {
    case (1):
        return "Negative";
    case (2):
        return "Grayscale";
    case (3):
        return "Red Tint";
    case (4):
        return "Green Tint";
    case (5):
        return "Blue Tint";
    case (6):
        return "Sepia";
    case (0):
    default:
        return "None";
    }
}

int CameraClass::setSpecialEffect(String i)
{
    uint8_t effect = 0;
    if (String("Negative").compareTo(i) == 0)
    {
        effect = 1;
    }
    else if (String("Grayscale").compareTo(i) == 0)
    {
        effect = 2;
    }
    else if (String("Red Tint").compareTo(i) == 0)
    {
        effect = 3;
    }
    else if (String("Green Tint").compareTo(i) == 0)
    {
        effect = 4;
    }
    else if (String("Blue Tint").compareTo(i) == 0)
    {
        effect = 5;
    }
    else if (String("Sepia").compareTo(i) == 0)
    {
        effect = 6;
    }
    return espCamSensor->set_special_effect(espCamSensor, effect);
}

String CameraClass::getWBMode()
{
    switch (espCamSensor->status.wb_mode)
    {
    case 1:
        return "Sunny";
    case 2:
        return "Cloudy";
    case 3:
        return "Office";
    case 4:
        return "Home";
    case 0:
    default:
        return "Auto";
    }
}

int CameraClass::setWBMode(String i)
{
    int mode = 0;

    if (String("Sunny").compareTo(i) == 0)
    {
        mode = 1;
    }
    else if (String("Cloudy").compareTo(i) == 0)
    {
        mode = 2;
    }
    else if (String("Office").compareTo(i) == 0)
    {
        mode = 3;
    }
    else if (String("Home").compareTo(i) == 0)
    {
        mode = 4;
    }

    return espCamSensor->set_wb_mode(espCamSensor, mode);
}

uint8_t CameraClass::getAWB()
{
    return espCamSensor->status.awb;
}

int CameraClass::setAWB(uint8_t i)
{
    return espCamSensor->set_whitebal(espCamSensor, i);
}

uint8_t CameraClass::getAWBGain()
{
    return espCamSensor->status.awb_gain;
}

int CameraClass::setAWBGain(uint8_t i)
{
    return espCamSensor->set_awb_gain(espCamSensor, i);
}

uint8_t CameraClass::getAEC()
{
    return espCamSensor->status.aec;
}

int CameraClass::setAEC(uint8_t i)
{
    return espCamSensor->set_exposure_ctrl(espCamSensor, i);
}

uint8_t CameraClass::getAECValue()
{
    return espCamSensor->status.aec_value;
}

int CameraClass::setAECValue(uint8_t i)
{
    return espCamSensor->set_aec_value(espCamSensor, i);
}

uint8_t CameraClass::getAEC2()
{
    return espCamSensor->status.aec2;
}

int CameraClass::setAEC2(uint8_t i)
{
    return espCamSensor->set_aec2(espCamSensor, i);
}

uint8_t CameraClass::getAELevel()
{
    return espCamSensor->status.ae_level;
}

int CameraClass::setAELevel(uint8_t i)
{
    return espCamSensor->set_ae_level(espCamSensor, i);
}

uint8_t CameraClass::getAGC()
{
    return espCamSensor->status.agc;
}

int CameraClass::setAGC(uint8_t i)
{
    return espCamSensor->set_gain_ctrl(espCamSensor, i);
}

uint8_t CameraClass::getAGCGain()
{
    return espCamSensor->status.agc_gain;
}

int CameraClass::setAGCGain(uint8_t i)
{
    return espCamSensor->set_agc_gain(espCamSensor, i);
}

uint8_t CameraClass::getGainCieling()
{
    return espCamSensor->status.gainceiling;
}

int CameraClass::setGainCieling(uint8_t i)
{
    return espCamSensor->set_gainceiling(espCamSensor, (gainceiling_t)i);
}

uint8_t CameraClass::getBPC()
{
    return espCamSensor->status.bpc;
}

int CameraClass::setBPC(uint8_t i)
{
    return espCamSensor->set_bpc(espCamSensor, i);
}

uint8_t CameraClass::getWPC()
{
    return espCamSensor->status.wpc;
}

int CameraClass::setWPC(uint8_t i)
{
    return espCamSensor->set_wpc(espCamSensor, i);
}

uint8_t CameraClass::getRawGMA()
{
    return espCamSensor->status.raw_gma;
}

int CameraClass::setRawGMA(uint8_t i)
{
    return espCamSensor->set_raw_gma(espCamSensor, i);
}

uint8_t CameraClass::getLensCorrection()
{
    return espCamSensor->status.lenc;
}

int CameraClass::setLensCorrection(uint8_t i)
{
    return espCamSensor->set_lenc(espCamSensor, i);
}

uint8_t CameraClass::getDCW()
{
    return espCamSensor->status.lenc;
}

int CameraClass::setDCW(uint8_t i)
{
    return espCamSensor->set_lenc(espCamSensor, i);
}

uint8_t CameraClass::getColorBar()
{
    return espCamSensor->status.colorbar;
}

int CameraClass::setColorBar(uint8_t i)
{
    return espCamSensor->set_colorbar(espCamSensor, i);
}

void CameraClass::loadCameraPreferences()
{

    Preferences prefs;
    
    prefs.begin(CAMERA_PREF_KEY, true);

    setResolution(prefs.getString(CAMERA_PREF_KEY_RESOLUTION));
    setQuality(prefs.getUInt(CAMERA_PREF_KEY_QUALITY));
    setBrightness(prefs.getUInt(CAMERA_PREF_KEY_BRIGHTNESS));
    setContrast(prefs.getUInt(CAMERA_PREF_KEY_CONTRAST));
    setSaturation(prefs.getUInt(CAMERA_PREF_KEY_SATURATION));
    setSpecialEffect(prefs.getString(CAMERA_PREF_KEY_SPECIAL_EFFECT));
    setWBMode(prefs.getString(CAMERA_PREF_KEY_WB_MODE));
    setAWB(prefs.getUInt(CAMERA_PREF_KEY_AWB));
    setAWBGain(prefs.getUInt(CAMERA_PREF_KEY_AWB_GAIN));
    setAEC(prefs.getUInt(CAMERA_PREF_KEY_AEC));
    setAEC2(prefs.getUInt(CAMERA_PREF_KEY_AEC2));
    setAELevel(prefs.getUInt(CAMERA_PREF_KEY_AE_LEVEL));
    setAECValue(prefs.getUInt(CAMERA_PREF_KEY_AEC_VALUE));
    setAGC(prefs.getUInt(CAMERA_PREF_KEY_AGC));
    setAGCGain(prefs.getUInt(CAMERA_PREF_KEY_AGC_GAIN));
    setGainCieling(prefs.getUInt(CAMERA_PREF_KEY_GAINCEILING));
    setBPC(prefs.getUInt(CAMERA_PREF_KEY_BPC));
    setWPC(prefs.getUInt(CAMERA_PREF_KEY_WPC));
    setRawGMA(prefs.getUInt(CAMERA_PREF_KEY_RAW_GMA));
    setLensCorrection(prefs.getUInt(CAMERA_PREF_KEY_LENS_CORRECTION));
    setVFlip(prefs.getUInt(CAMERA_PREF_KEY_VFLIP));
    setHFlip(prefs.getUInt(CAMERA_PREF_KEY_HFLIP));
    setDCW(prefs.getUInt(CAMERA_PREF_KEY_DCW));
    setColorBar(prefs.getUInt(CAMERA_PREF_KEY_COLORBAR));

    setLamp(prefs.getUInt(CAMERA_PREF_KEY_LAMP));
    setRotation(prefs.getString(CAMERA_PREF_KEY_ROTATION));

    prefs.end();
}

void CameraClass::saveCameraPreferences()
{
    Preferences prefs;
    prefs.begin(CAMERA_PREF_KEY, false);

    prefs.putString(CAMERA_PREF_KEY_RESOLUTION, getResolution().c_str());
    prefs.putUInt(CAMERA_PREF_KEY_QUALITY, getQuality());
    prefs.putUInt(CAMERA_PREF_KEY_BRIGHTNESS, getBrightness());
    prefs.putUInt(CAMERA_PREF_KEY_CONTRAST, getContrast());
    prefs.putUInt(CAMERA_PREF_KEY_SATURATION, getSaturation());
    prefs.putString(CAMERA_PREF_KEY_SPECIAL_EFFECT, getSpecialEffect().c_str());
    prefs.putString(CAMERA_PREF_KEY_WB_MODE, getWBMode().c_str());
    prefs.putUInt(CAMERA_PREF_KEY_AWB, getAWB());
    prefs.putUInt(CAMERA_PREF_KEY_AWB_GAIN, getAWBGain());
    prefs.putUInt(CAMERA_PREF_KEY_AEC, getAEC());
    prefs.putUInt(CAMERA_PREF_KEY_AEC2, getAEC2());
    prefs.putUInt(CAMERA_PREF_KEY_AE_LEVEL, getAELevel());
    prefs.putUInt(CAMERA_PREF_KEY_AEC_VALUE, getAECValue());
    prefs.putUInt(CAMERA_PREF_KEY_AGC, getAGC());
    prefs.putUInt(CAMERA_PREF_KEY_AGC_GAIN, getAGCGain());
    prefs.putUInt(CAMERA_PREF_KEY_GAINCEILING, getGainCieling());
    prefs.putUInt(CAMERA_PREF_KEY_BPC, getBPC());
    prefs.putUInt(CAMERA_PREF_KEY_WPC, getWPC());
    prefs.putUInt(CAMERA_PREF_KEY_RAW_GMA, getRawGMA());
    prefs.putUInt(CAMERA_PREF_KEY_LENS_CORRECTION, getLensCorrection());
    prefs.putUInt(CAMERA_PREF_KEY_VFLIP, getVFlip());
    prefs.putUInt(CAMERA_PREF_KEY_HFLIP, getHFlip());
    prefs.putUInt(CAMERA_PREF_KEY_DCW, getDCW());
    prefs.putUInt(CAMERA_PREF_KEY_COLORBAR, getColorBar());
    prefs.putUInt(CAMERA_PREF_KEY_LAMP, getLamp());
    prefs.putString(CAMERA_PREF_KEY_ROTATION, getRotation().c_str());

    prefs.end();
}

void CameraClass::removeCameraPreferences()
{
    Preferences prefs;
    prefs.begin(CAMERA_PREF_KEY, false);

    prefs.remove(CAMERA_PREF_KEY_RESOLUTION);
    prefs.remove(CAMERA_PREF_KEY_QUALITY);
    prefs.remove(CAMERA_PREF_KEY_BRIGHTNESS);
    prefs.remove(CAMERA_PREF_KEY_CONTRAST);
    prefs.remove(CAMERA_PREF_KEY_SATURATION);
    prefs.remove(CAMERA_PREF_KEY_SPECIAL_EFFECT);
    prefs.remove(CAMERA_PREF_KEY_WB_MODE);
    prefs.remove(CAMERA_PREF_KEY_AWB);
    prefs.remove(CAMERA_PREF_KEY_AWB_GAIN);
    prefs.remove(CAMERA_PREF_KEY_AEC);
    prefs.remove(CAMERA_PREF_KEY_AEC2);
    prefs.remove(CAMERA_PREF_KEY_AE_LEVEL);
    prefs.remove(CAMERA_PREF_KEY_AEC_VALUE);
    prefs.remove(CAMERA_PREF_KEY_AGC);
    prefs.remove(CAMERA_PREF_KEY_AGC_GAIN);
    prefs.remove(CAMERA_PREF_KEY_GAINCEILING);
    prefs.remove(CAMERA_PREF_KEY_BPC);
    prefs.remove(CAMERA_PREF_KEY_WPC);
    prefs.remove(CAMERA_PREF_KEY_RAW_GMA);
    prefs.remove(CAMERA_PREF_KEY_LENS_CORRECTION);
    prefs.remove(CAMERA_PREF_KEY_VFLIP);
    prefs.remove(CAMERA_PREF_KEY_HFLIP);
    prefs.remove(CAMERA_PREF_KEY_DCW);
    prefs.remove(CAMERA_PREF_KEY_COLORBAR);
    prefs.remove(CAMERA_PREF_KEY_LAMP);
    prefs.remove(CAMERA_PREF_KEY_ROTATION);

    prefs.end();
}
#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
CameraClass CameraClient;
#endif