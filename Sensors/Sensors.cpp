#include "Sensors.hpp"
#include <chrono>

Sensor::Sensor(const std::string& name, int minValue, int maxValue)
    : sensor_name(name), min_value(minValue), max_value(maxValue),
      rng(std::chrono::steady_clock::now().time_since_epoch().count()),
      dist(min_value, max_value) {}

int Sensor::generateReading() {
    return dist(rng);
}

std::string Sensor::getName() const {
    return sensor_name;
}
