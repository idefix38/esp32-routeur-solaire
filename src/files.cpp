#include "files.h"
#include <LittleFS.h> // Gestion des fichiers du repertoire data

void listDir(File dir)
{
  while (File file = dir.openNextFile())
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.path());
      listDir(file);
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.path());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
  }
}

void setupSpiffs()
{
  if (!LittleFS.begin())
  {
    Serial.println("Erreur LittleFS  ...");
    return;
  }

  Serial.println("[-] Lecture des fichiers LittleFS ");
  File root = LittleFS.open("/");
  listDir(root);
}
