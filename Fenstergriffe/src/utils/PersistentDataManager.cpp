#include "utils/PersistentDataManager.h"
#include "LittleFS.h"
#include "utils/LogHelper.h"

bool PersistentDataManager::isInitialized = false;

bool PersistentDataManager::readFile(const char *path, String *msg) {
    LogHelper::log("PersistentDataManager", "Read File: " + String(path), logging::LoggerLevel::LOGGER_LEVEL_INFO);

    if (!isInitialized) {
        if (!LittleFS.begin()) {
            LogHelper::log("PersistentDataManager", "Error mounting LittleFS",
                           logging::LoggerLevel::LOGGER_LEVEL_ERROR);
            return false;
        } else
            isInitialized = true;
    }

    File file = LittleFS.open(path, "r");

    if (!file || file.isDirectory()) {
        LogHelper::log("PersistentDataManager", "Failed to open file for reading",
                       logging::LoggerLevel::LOGGER_LEVEL_ERROR);
        return false;
    }

    *msg = file.readString();

    file.close();

    LogHelper::log("PersistentDataManager", "Data successfully read from LittleFS",
                   logging::LoggerLevel::LOGGER_LEVEL_INFO);
    return true;
}

bool PersistentDataManager::writeFile(const char *path, const char *msg) {
    LogHelper::log("PersistentDataManager", "Write File: " + String(path), logging::LoggerLevel::LOGGER_LEVEL_INFO);
    if (!isInitialized) {
        if (!LittleFS.begin()) {
            LogHelper::log("PersistentDataManager", "Error mounting LittleFS",
                           logging::LoggerLevel::LOGGER_LEVEL_ERROR);
            return false;
        } else
            isInitialized = true;
    }
    File file = LittleFS.open(path, "w");

    // msg to byte array
    byte *byteArray = new byte[strlen(msg)];
    for (int i = 0; i < strlen(msg); i++) {
        byteArray[i] = msg[i];
    }

    file.write(byteArray, strlen(msg));
    delay(2);
    file.close();
    LogHelper::log( "PersistentDataManager", "Data successfully written to LittleFS",
                    logging::LoggerLevel::LOGGER_LEVEL_INFO);
            return true;
}

bool PersistentDataManager::deleteFile(const char *path) {
    LogHelper::log("PersistentDataManager", "Delete File: " + String(path), logging::LoggerLevel::LOGGER_LEVEL_INFO);
    if (!isInitialized) {
        if (!LittleFS.begin()) {
            LogHelper::log("PersistentDataManager", "Error mounting LittleFS",
                           logging::LoggerLevel::LOGGER_LEVEL_ERROR);
                return false;
        } else
            isInitialized = true;
    }

    if (LittleFS.remove(path)) {
        LogHelper::log("PersistentDataManager", "File deleted", logging::LoggerLevel::LOGGER_LEVEL_INFO);
        return true;
    } else {
        LogHelper::log("PersistentDataManager", "File deletion failed", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
        return false;
    }
}