#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <string>
#include <random>

class Sensor {
public:
    Sensor(const std::string& name, int minValue = 10, int maxValue = 1000);

    int generateReading();               // Generates one random moisture value
    std::string getName() const;         // Sensor identifier

private:
    std::string sensor_name;
    int min_value;
    int max_value;

    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;
};

#endif // SENSOR_HPP
