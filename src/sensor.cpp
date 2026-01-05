#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 4 on the
#define pinTemperature 4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(pinTemperature);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void setupSensor()
{
  sensors.begin(); // Démare la lecture de capteur de température
}

float getTemperature()
{
  // Recupère la température
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    Serial.print("Température actuelle : ");
    Serial.println(tempC);
    return tempC;
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
    return -127; // Retourne -127 en cas d'erreur de lecture
  }
}