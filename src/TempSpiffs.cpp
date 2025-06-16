
#include <SPIFFS.h>  // Gestion des fichiers du repertoire data


void setupSpiffs() {
 if(!SPIFFS.begin()) {
      Serial.println("Erreur SPIFFS ...");
      return;
    } 

  File root = SPIFFS.open("/");
  Serial.println("[-] Lecture des fichiers SPIFFS" );
  File file = root.openNextFile();  
  while(file) {
    String filename = "/";
    filename.concat(file.name());                          
    Serial.print("  File: ");
    Serial.println(filename);
    file.close();
    file = root.openNextFile();
  }   
}