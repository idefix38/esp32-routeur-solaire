#include "historyManager.h"

// Implémentation pour HistoryManager utilisant `float`.
HistoryManager::HistoryManager(size_t capacity) : capacity(capacity), head(0), tail(0), count(0)
{
    buffer = new DataPoint[capacity];
    mutex = xSemaphoreCreateMutex();
}

HistoryManager::~HistoryManager()
{
    delete[] buffer;
    vSemaphoreDelete(mutex);
}

void HistoryManager::add(float value)
{
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
    {
        DataPoint &dataPoint = buffer[tail];
        dataPoint.value = value;
        time(&dataPoint.timestamp);

        tail = (tail + 1) % capacity;

        if (count < capacity)
        {
            count++;
        }
        else
        {
            head = (head + 1) % capacity;
        }
        xSemaphoreGive(mutex);
    }
}

std::vector<DataPoint> HistoryManager::getData()
{
    std::vector<DataPoint> data;
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
    {
        data.reserve(count);
        if (count > 0)
        {
            size_t current = head;
            for (size_t i = 0; i < count; i++)
            {
                data.push_back(buffer[current]);
                current = (current + 1) % capacity;
            }
        }
        xSemaphoreGive(mutex);
    }
    return data;
}

void HistoryManager::serialize(JsonDocument &doc)
{
    JsonArray array = doc.to<JsonArray>();
    std::vector<DataPoint> data = getData();
    for (const auto &dp : data)
    {
        JsonObject obj = array.add<JsonObject>();
        obj["time"] = dp.timestamp;
        obj["value"] = dp.value;
    }
}
