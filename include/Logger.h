#ifndef Logger_h
#define Logger_h

#include <TelnetStream.h>
#include <Arduino.h>

class LoggerClass
{
public:
    LoggerClass();
    ~LoggerClass();

    void begin();
    void end();

    void log(std::string msg);
    void log(String msg);
    void log(char msg[]);
    // void log(const char *msg);

    void log(const char* format, ...);

protected:
    TaskHandle_t tTelnet;
    static void telnetCB(void *pvParameters);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
extern LoggerClass Logger;
#endif

#endif