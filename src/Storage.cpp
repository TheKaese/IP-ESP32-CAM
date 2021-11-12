#include "Storage.h"


/*
 * Useful utility when debugging... 
 */

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing SPIFFS directory: \"%s\"\r\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void FileSystemInit()
{
  while (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
  {
    // if we sit in this loop something is wrong;
    // if no existing spiffs partition exists one should be automagically created.
    Serial.println("SPIFFS Mount failed, this can happen on first-run initialisation.");
    Serial.println("If it happens repeatedly check if a SPIFFS partition is present for your board?");

    delay(1000);
    Serial.println("Retrying..");
  }
  Serial.println("Internal filesystem contents");
  listDir(SPIFFS, "/", 0);
}
