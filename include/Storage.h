#include "FS.h"
#include "SPIFFS.h"
#include "esp_camera.h"
#include "jsonlib/jsonlib.h"
#include <CameraClient.h>

#define FORMAT_SPIFFS_IF_FAILED true

extern char MODULE_NAME[];

extern void dumpPrefs(fs::FS &fs);

extern void FileSystemInit();
