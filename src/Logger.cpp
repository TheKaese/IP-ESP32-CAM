#include <Logger.h>

LoggerClass::LoggerClass(){};
LoggerClass::~LoggerClass(){};

void LoggerClass::telnetCB(void *pvParameters)
{
    while (true)
    {
        //Auto start the TelNetstream as soon as WiFi is connected
        if (WL_CONNECTED == WiFi.status())
        {
            Serial.println("Starting telnet stream");
            TelnetStream.begin();
            vTaskDelete(NULL);
        }

        delay(5000);
    }
}

void LoggerClass::begin()
{

    //Serial should always be available, so just kick it off
    Serial.begin(115200);
    Serial.println("Starting logger");

    xTaskCreate(this->telnetCB, "Telnet", 2048, this, 2, &tTelnet);
}

void LoggerClass::log(std::string msg)
{
    Serial.println(msg.c_str());
    if (TelnetStream.peek() > -1)
    {
        TelnetStream.println(msg.c_str());
    }
}

void LoggerClass::log(String msg)
{
    Serial.println(msg);
    if (TelnetStream.peek() > -1)
    {
        TelnetStream.println(msg);
    }
}

void LoggerClass::log(char msg[])
{
    Serial.println(msg);
    if (TelnetStream.peek() > -1)
    {
        TelnetStream.println(msg);
    }
}

void LoggerClass::log(const char *format, ...)
{
    static char loc_buf[64];
    char *temp = loc_buf;
    int len;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf))
    {
        temp = (char *)malloc(len + 1);
        if (temp == NULL)
        {
            return;
        }
    }
    vsnprintf(temp, len + 1, format, arg);

    Serial.println(temp);
    if (TelnetStream.peek() > -1)
    {
        TelnetStream.println(temp);
    }
}

void LoggerClass::end()
{
    TelnetStream.stop();
    vTaskDelete(tTelnet);
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
LoggerClass Logger;
#endif
