#include "utils/LogHelper.h"
#include "logger.h"

logging::Logger logger;

void LogHelper::log(const String& module, const String& text, logging::LoggerLevel level) {
    logger.log(level, module, text.c_str());
}