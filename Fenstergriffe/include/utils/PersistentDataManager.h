#ifndef PERSISTENTDATAMANAGER_H
#define PERSISTENTDATAMANAGER_H
#include <Arduino.h>

class PersistentDataManager {
private:
    static bool isInitialized;
public:
    static bool readFile(const char* path, String *result);

    static bool writeFile(const char* path, const char* msg);

    static bool deleteFile(const char* path);
};

#endif //PERSISTENTDATAMANAGER_H