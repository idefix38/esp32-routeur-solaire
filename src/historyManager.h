#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <Arduino.h>
#include <time.h>
#include <vector>
#include <ArduinoJson.h>

// Structure pour un point de donnée horodaté (utilise `float` pour la valeur).
struct DataPoint
{
    time_t timestamp;
    float value;
};

// Classe de tampon circulaire pour stocker un historique de données de type `float`.
// Déclarations seulement — les implémentations sont dans `historyManager.cpp`.
class HistoryManager
{
public:
    HistoryManager(size_t capacity);
    ~HistoryManager();
    void add(float value);
    std::vector<DataPoint> getData();
    void serialize(JsonDocument &doc);

private:
    DataPoint *buffer;       // Le tampon de données alloué dynamiquement
    size_t capacity;         // Capacité maximale du tampon
    size_t head;             // Index du plus ancien élément
    size_t tail;             // Index où sera inséré le prochain élément
    size_t count;            // Nombre actuel d'éléments dans le tampon
    SemaphoreHandle_t mutex; // Mutex pour la synchronisation entre les tâches
};

#endif // HISTORY_MANAGER_H
