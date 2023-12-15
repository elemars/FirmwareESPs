#ifndef SSDS_GATEWAY_LOGHELPER_H
#define SSDS_GATEWAY_LOGHELPER_H


#include <Arduino.h>
#include "logger.h"

class LogHelper {

public:
    static void log(const String& module, const String& text, logging::LoggerLevel level);
};


#endif //SSDS_GATEWAY_LOGHELPER_H